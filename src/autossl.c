/*

autossl.so

USAGE
	
	LD_PRELOAD=$PWD/autossl.so AUTOSSL_UPGRADE_PORTS="80 8080" AUTOSSL_TLS_CMD=stunnel.sh wget ...

	LD_PRELOAD=$PWD/autossl.so AUTOSSL_UPGRADE_PORT=80 AUTOSSL_UPGRADE_IPS="192.0.2.1 192.0.2.2" AUTOSSL_TLS_CMD=s_client.sh wget ...

DESCRIPTION

	This shared library extends connect(2) standard library function
	adding SSL/TLS layer on the socket transparently. Doing it by
	invoking a wrapper command and passing the caller process
	traffic through its STDIO.
	
	On each connect() calls, it checks that the destination port is one
	of those from AUTOSSL_UPGRADE_PORTS environment variable and the
	destination IP is one of the IPs in AUTOSSL_UPGRADE_IPS (if it's
	set). AUTOSSL_UPGRADE_PORTS and AUTOSSL_UPGRADE_IPS are
	space-delimited list of port numbers and IPs respectively. If you
	don't know the IP(s) prior, leave AUTOSSL_UPGRADE_IPS unset, then
	any connection on AUTOSSL_UPGRADE_PORTS ports to any host will be
	upgraded.
	
	If the criteria above are satisfied, autossl.so starts
	AUTOSSL_TLS_CMD expecting it to connect to the right TLS endpoint.
	AUTOSSL_TLS_CMD invoked with 2 arguments: the original IP and port
	the caller process wanted to connect, so it can find out where to
	open the TLS channel if the caller possibly connects to more than 1
	endpoints during runtime.
	
	In the wrapper command, run stunnel(8) or openssl s_client(1SSL) or
	other command to open a TLS channel connected to the STDIO, eg.
	
	  stunnel -f -c -r mail.example.net:993
	  
	  openssl s_client -connect $1:443 -servername example.net -quiet
	
	If you dont want to upgrade a particular connection to TLS, simply
	run something like 'unset AUTOSSL_UPGRADE_PORTS; netcat $1 $2' 
	
	Currently the domain name which the caller process wants to connect
	to is not known, due to the disconnected nature of hostname
	resolution and socket networking on the level of standard library
	calls. Therefore the wrapper command has to be creative what to send
	in SNI to the TLS endpoint. Autossl is not particular recommended to
	use in processes which connects to many various hosts (eg. web
	browsers) with little domain name – IP range corelation, because
	it makes hard for the wrapper command to find out the right SNI for
	a given IP. A more plausible scenario to have autossl.so upgrade
	socket connections to a few, 1, 2, or 3 hosts, or to hosts whiches
	server name is unambiguous by thier IP in order to make correct SNI.
	
	By default, autossl does not cause exception in the caller process
	in error cases (eg. ip address parse error, invalid port number),
	rather falls back to system's connect(2) call. However if
	AUTOSSL_ERRNO is set, it sets errno to that value and returns -1 in
	the above error cases. You may set AUTOSSL_ERRNO to 5 to report
	IOError in such cases.

COMPATIBILITY

	inet sockets (ipv4)
	SOCK_STREAM (tcp)

ENVIRONMENT VARIABLES

	AUTOSSL_UPGRADE_PORTS
	AUTOSSL_UPGRADE_IPS
	AUTOSSL_TLS_CMD
	AUTOSSL_ERRNO

COMPILE

	gcc -D_GNU_SOURCE -ldl -shared -fPIC -o autossl.so autossl.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>

void _autossl_ip_parse_error(const char* s, const size_t len)
{
	fprintf(stderr, "autossl: failed to parse ip address '%.*s'\n", len, s);
}

int connect(int sockfd, const struct sockaddr_in *orig_sockaddr, socklen_t addrlen)
{
	char *upgrade_ip_str;
	struct in_addr upgrade_ip;
	char *upgrade_port_str;
	in_port_t upgrade_port = 0;
	
	char *tls_cmd;
	#define IP_STR_LEN 39+1
	#define PORT_STR_LEN 5+1
	char connect_ip[IP_STR_LEN];
	char connect_port[PORT_STR_LEN];
	
	char *next_separator;
	char *autossl_errno_str;
	
	struct sockaddr_in to_sockaddr;
	int (*orig_connect)(int, const struct sockaddr_in *, socklen_t) = dlsym(RTLD_NEXT, "connect");
	
	

	if(orig_sockaddr->sin_family != AF_INET) goto stdlib;
	
	upgrade_port_str = getenv("AUTOSSL_UPGRADE_PORTS");
	if(upgrade_port_str == NULL) goto stdlib;
	
	
	int upgrade_port_matched = 0;
	next_separator = NULL;
	
	do{
		next_separator = strchrnul(upgrade_port_str, ' ');
		upgrade_port = atoi(upgrade_port_str);
		if(upgrade_port == 0) { fprintf(stderr, "autossl: failed to parse port number(s): %s\n", upgrade_port_str); goto error_case; }
		if(upgrade_port == ntohs(orig_sockaddr->sin_port)) upgrade_port_matched = 1;
		upgrade_port_str = (char*)(next_separator + 1);
	}
	while(!upgrade_port_matched && *next_separator != '\0');
	
	if(!upgrade_port_matched) goto stdlib;
	
	upgrade_ip_str = getenv("AUTOSSL_UPGRADE_IPS");
	if(upgrade_ip_str != NULL)
	{
		int upgrade_ip_matched = 0;
		char *next_separator = NULL;
		
		do{
			next_separator = strchrnul(upgrade_ip_str, ' ');
			if(inet_aton(upgrade_ip_str, &upgrade_ip) == 0)
			{
				_autossl_ip_parse_error(upgrade_ip_str, next_separator - upgrade_ip_str);
				goto error_case;
			}
			if(ntohl(orig_sockaddr->sin_addr.s_addr) == ntohl(upgrade_ip.s_addr)) {
				upgrade_ip_matched = 1;
			}
			upgrade_ip_str = (char*)(next_separator + 1);
		}
		while(!upgrade_ip_matched && *next_separator != '\0');
		
		if(!upgrade_ip_matched) goto stdlib;
	}
	
	tls_cmd = getenv("AUTOSSL_TLS_CMD");
	if(tls_cmd == NULL) goto stdlib;
	
	
	int sockpair[2];
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair) == -1)
	{
		perror("autossl: sockpair");
		goto error_case;
	}
	
	pid_t childpid = fork();
	if(childpid < 0)
	{
		perror("autossl: fork");
		goto error_case;
	}
	if(childpid == 0)
	{
		/* save the ip and port we wanted to connect to as strings */
		snprintf(connect_ip, IP_STR_LEN, "%s", inet_ntoa(orig_sockaddr->sin_addr));
		snprintf(connect_port, PORT_STR_LEN, "%d", ntohs(orig_sockaddr->sin_port));
		/* wire STDIO to the newly created socket */
		dup2(sockpair[1], 0);
		dup2(sockpair[1], 1);
		/* leave stderr open */
		/* close dangling files */
		closefrom(3);
		execlp(tls_cmd, tls_cmd, connect_ip, connect_port, NULL);
		_exit(127);
	}
	
	close(sockpair[1]);
	if(dup2(sockpair[0], sockfd) == -1)
	{
		perror("autossl: dup2");
		close(sockpair[0]);
		goto error_case;
	}
	fprintf(stderr, "autossl: redirecting %s:%d -> fd#%d\n", inet_ntoa(orig_sockaddr->sin_addr), ntohs(orig_sockaddr->sin_port), sockpair[0]);
	
	/* sockpair[0] will be closed eventually by the caller. */
	
	/* childpid process won't be reaped, so don't be scared on the
	zombie processes, they will be disappear as the main program exits. */
	
	return 0;
	
	error_case:
	autossl_errno_str = getenv("AUTOSSL_ERRNO");
	if(autossl_errno_str)
	{
		errno = atoi(autossl_errno_str);
		return -1;
	}
	
	stdlib:
	return orig_connect(sockfd, orig_sockaddr, addrlen);
}
