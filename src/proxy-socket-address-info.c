
/*
What is this library?

This is an LD_PRELOAD-able lib, of which purpose is to fake the source IP addresses and port numbers
of the internet traffic received by the target programm.

IP and port are changed in the msghdr structure returned by recvmsg(2).
That to which IP and port should be changed is determined by symlinks found in the directory
pointed by PSAI_SHM_PATH environment, /run/shm/psai by default.
Original IP address IP1 and port number PORT1 pair is searched on the path "PSAI_SHM_PATH/IP1,PORT1",
then if found, take the symlink target in format IP2,PORT2 and get the fake IP and port from it.

This is useful in situations where you put a reverse proxy in front of your network application (server),
which proxy terminates the remote client's network connection, thus hiding the real client's IP from the server.
Use this lib on the proxy to manage a PSAI (proxy socket address info) table (ie. symlinks in SHM)
for the traffic sent to the server;
and use it on the server program to make it know the real client IPs from the PSAI table.

It's a good idea to point PSAI_SHM_PATH to a memory-backed filesystem to prevent high I/O,
but I don't know hard reason against putting it on a network-shared filesystem - this way the
proxy and the server probably don't even need to be on the same machine.

IP address and port numbers are separated not by the well-established colon ":" char, but by comma ","
in order not to confuse IPv6 addresses.
*/

#include <dlfcn.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>


#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#else
#define DEBUG(...) 0
#endif

typedef int bool_t;
#define TRUE 1
#define FALSE 0


static ssize_t (*sys_recvmsg)(int sockfd, struct msghdr *msg, int flags);
static ssize_t (*sys_sendmsg)(int sockfd, const struct msghdr *msg, int flags);
static int (*sys_close)(int fd);
static int (*sys_getpeername)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

ssize_t __attribute__((visibility("default")))
	recvmsg(int sockfd, struct msghdr *msg, int flags);
ssize_t __attribute__((visibility("default")))
	sendmsg(int sockfd, const struct msghdr *msg, int flags);
int __attribute__((visibility("default")))
	close(int fd);
int __attribute__((visibility("default")))
	getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);


void _init(void)
{
  sys_recvmsg = dlsym(RTLD_NEXT, "recvmsg");
  sys_sendmsg = dlsym(RTLD_NEXT, "sendmsg");
  sys_close = dlsym(RTLD_NEXT, "close");
  sys_getpeername = dlsym(RTLD_NEXT, "getpeername");
}

#define DFLT_SHM_PATH "/run/shm/psai"
#define SHM_PATH_MAX_LEN 64
#define IPADDR_MAX_LEN INET6_ADDRSTRLEN
#define PORTNUM_MAX_LEN 5
#define SOCKET_ADDR_MAX_LEN (IPADDR_MAX_LEN + 1 + PORTNUM_MAX_LEN)
#define PATH_MAX_LEN (SHM_PATH_MAX_LEN + 1 + SOCKET_ADDR_MAX_LEN)

static char * shm_path()
{
	char * path = getenv("PSAI_SHM_PATH");
	if(path == NULL) return DFLT_SHM_PATH;
	if(strlen(path) > SHM_PATH_MAX_LEN)
	{
		warnx("PSAI_SHM_PATH length > %u", SHM_PATH_MAX_LEN);
		abort();
	}
	return path;
}

bool_t
extract_ip_port(struct sockaddr * socket_addr, char * out_ipaddr, uint16_t * out_port)
{
	in_port_t * port_p;
	void * addr_p;
	
	if(socket_addr->sa_family == AF_INET)
	{
		port_p = &((struct sockaddr_in*)socket_addr)->sin_port;
		addr_p = &((struct sockaddr_in*)socket_addr)->sin_addr;
	}
	else if(socket_addr->sa_family == AF_INET6)
	{
		port_p = &((struct sockaddr_in6*)socket_addr)->sin6_port;
		addr_p = &((struct sockaddr_in6*)socket_addr)->sin6_addr;
	}
	else return FALSE;
	
	if(out_port) *out_port = ntohs(*port_p);
	if(!out_ipaddr) return TRUE;
	if(inet_ntop(socket_addr->sa_family, addr_p, out_ipaddr, IPADDR_MAX_LEN) == NULL) return FALSE;
	return TRUE;
}

bool_t
update_socket_addr(struct sockaddr * socket_addr, char * ipaddr_str, char * portnum_str)
{
	int ret;
	in_port_t * port_p;
	void * addr_p;
	
	if(socket_addr->sa_family == AF_INET)
	{
		port_p = &(((struct sockaddr_in*)socket_addr)->sin_port);
		addr_p = &(((struct sockaddr_in*)socket_addr)->sin_addr);
	}
	else if(socket_addr->sa_family == AF_INET6)
	{
		port_p = &(((struct sockaddr_in6*)socket_addr)->sin6_port);
		addr_p = &(((struct sockaddr_in6*)socket_addr)->sin6_addr);
	}
	else return FALSE;
	
	if(portnum_str) *port_p = htons(atoi(portnum_str));
	ret = inet_pton(socket_addr->sa_family, ipaddr_str, addr_p);
	return ret == 1 ? TRUE : FALSE;
}

bool_t
change_socket_address_in_msg(struct msghdr * msg)
{
	uint16_t orig_port;
	char orig_ipaddr[IPADDR_MAX_LEN+1];
	char linkname_buf[PATH_MAX_LEN+1];
	char linktarg_buf[SOCKET_ADDR_MAX_LEN+1];
	ssize_t nbytes;
	char * portnum_str;
	
	if(! msg) return FALSE;
	if(! extract_ip_port(msg->msg_name, &orig_ipaddr, &orig_port)) return FALSE;
	
	snprintf(linkname_buf, PATH_MAX_LEN, "%s/%s,%u", shm_path(), orig_ipaddr, orig_port);
	DEBUG("socket address lookup: %s", linkname_buf);
	
	nbytes = readlink(linkname_buf, linktarg_buf, SOCKET_ADDR_MAX_LEN+1);
	if(nbytes <= 0) return FALSE;
	if(nbytes > SOCKET_ADDR_MAX_LEN)
	{
		linktarg_buf[SOCKET_ADDR_MAX_LEN] = '\0';
		DEBUG("symlink target too long: %s -> %s...", linkname_buf, linktarg_buf);
		return FALSE;
	}
	linktarg_buf[nbytes] = '\0';
	DEBUG("socket address mapping: %s -> %s", linkname_buf, linktarg_buf);
	
	portnum_str = strrchr(linktarg_buf, ',');
	if(portnum_str) { portnum_str[0] = '\0'; portnum_str++; }
	
	return update_socket_addr((struct sockaddr*)msg->msg_name, linktarg_buf, portnum_str);
}

ssize_t
recvmsg(int sockfd, struct msghdr *msg, int flags)
{
	ssize_t recvmsg_ret;
	recvmsg_ret = sys_recvmsg(sockfd, msg, flags);
	change_socket_address_in_msg(msg);
	return recvmsg_ret;
}
