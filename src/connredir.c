/*

connredir.c

USAGE
	
	LD_PRELOAD=$PWD/connredir.so CONNREDIR_ORIG_IP=169.254.169.254 CONNREDIR_ORIG_PORT=80 CONNREDIR_TO_IP=127.0.0.1 CONNREDIR_TO_PORT=8080 wget ...

	LD_PRELOAD=$PWD/connredir.so CONNREDIR_ORIG_IP=127.0.0.1 CONNREDIR_ORIG_PORT=25 CONNREDIR_TO_FD=1 sendmail ...

DESCRIPTION

	This shared library is intended to extend connect(2) standard library function by ip/port override capability, so you control
	programs, where to connect, without modifying or configuring themself.
	
	On each connect() calls, it checks that the destination IP and port match to the ones in CONNREDIR_ORIG_IP and CONNREDIR_ORIG_PORT
	environment variables. Then changes them to the IP and port numbers in CONNREDIR_TO_IP and CONNREDIR_TO_PORT respectively.
	
	Leave CONNREDIR_ORIG_PORT unset to override all ports.
	Leave CONNREDIR_TO_PORT unset to not override the destination port number.
	
	If CONNREDIR_TO_FD is set, then it's evaluated before CONNREDIR_TO_IP. CONNREDIR_TO_FD is a file descriptor number which
	replaces the socket's FD which is about to be connected; so you can make your program communicate on an arbitrary pre-opened
	file descriptor instead of INET. Useful when combined with socat(1).
	
	By default, connredir does not cause exception in the caller process in error cases (eg. ip address parse error, invalid port number,
	closed file descriptor), rather falls back to system's connect(2) call.
	However if CONNREDIR_ERRNO is set, the it sets errno to that value and returns -1 in the above error cases.
	You may set CONNREDIR_ERRNO to 5 to report IO Error in such cases.

COMPATIBILITY

	inet sockets (ipv4)
	SOCK_STREAM (tcp)

ENVIRONMENT VARIABLES

	CONNREDIR_ORIG_IP
	CONNREDIR_ORIG_PORT
	CONNREDIR_TO_FD
	CONNREDIR_TO_IP
	CONNREDIR_TO_PORT
	CONNREDIR_ERRNO

COMPILE

	gcc -D_GNU_SOURCE -ldl -shared -fPIC -o connredir.so connredir.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>

void _connredir_ip_parse_error(const char* s)
{
	fprintf(stderr, "connredir: failed to parse ip address: %s\n", s);
}

int connect(int sockfd, const struct sockaddr_in *orig_sockaddr, socklen_t addrlen)
{
	struct in_addr orig_ip;
	char *orig_ip_str;
	char *orig_port_str;
	in_port_t orig_port = 0;
	
	char *to_fd_str;
	int to_fd;
	
	char *to_ip_str;
	struct in_addr to_ip;
	char *to_port_str;
	in_port_t to_port = 0;
	
	char *connredir_errno_str;
	
	struct sockaddr_in to_sockaddr;
	int (*orig_connect)(int, const struct sockaddr_in *, socklen_t) = dlsym(RTLD_NEXT, "connect");
	
	
	
	if(orig_sockaddr->sin_family != AF_INET) goto stdlib;
	
	orig_ip_str = getenv("CONNREDIR_ORIG_IP");
	if(orig_ip_str == NULL) goto stdlib;
	
	if(inet_aton(orig_ip_str, &orig_ip) == 0)
	{
		_connredir_ip_parse_error(orig_ip_str);
		goto error_case;
	}
	if(ntohl(orig_sockaddr->sin_addr.s_addr) != ntohl(orig_ip.s_addr)) goto stdlib;
	
	orig_port_str = getenv("CONNREDIR_ORIG_PORT");
	if(orig_port_str != NULL) orig_port = atoi(orig_port_str);
	if(orig_port != 0 && orig_port != ntohs(orig_sockaddr->sin_port)) goto stdlib;
	
	
	to_fd_str = getenv("CONNREDIR_TO_FD");
	if(to_fd_str != NULL)
	{
		to_fd = atoi(to_fd_str);
		if(dup2(to_fd, sockfd) == -1) { perror("connredir: dup2"); goto error_case; }
		fprintf(stderr, "connredir: redirecting %s:%d -> fd#%d\n", inet_ntoa(orig_sockaddr->sin_addr), ntohs(orig_sockaddr->sin_port), to_fd);
		return 0;
	}
	
	to_ip_str = getenv("CONNREDIR_TO_IP");
	if(to_ip_str == NULL) goto stdlib;
	
	if(inet_aton(to_ip_str, &to_ip) == 0)
	{
		_connredir_ip_parse_error(to_ip_str);
		goto error_case;
	}
	
	to_port_str = getenv("CONNREDIR_TO_PORT");
	if(to_port_str != NULL) to_port = atoi(to_port_str);
	
	to_sockaddr.sin_family = AF_INET;
	to_sockaddr.sin_port = to_port == 0 ? orig_sockaddr->sin_port : htons(to_port);
	to_sockaddr.sin_addr = to_ip;
	
	fprintf(stderr, "connredir: redirecting %s:%d -> ", inet_ntoa(orig_sockaddr->sin_addr), ntohs(orig_sockaddr->sin_port));
	fprintf(stderr, "%s:%d\n", inet_ntoa(to_sockaddr.sin_addr), ntohs(to_sockaddr.sin_port));
	orig_sockaddr = &to_sockaddr;
	goto stdlib;
	
	error_case:
	connredir_errno_str = getenv("CONNREDIR_ERRNO");
	if(connredir_errno_str)
	{
		errno = atoi(connredir_errno_str);
		return -1;
	}
	
	stdlib:
	return orig_connect(sockfd, orig_sockaddr, addrlen);
}
