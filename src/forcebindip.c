/**
 * forcebindip.c
 *
 * Compile with: gcc -D _GNU_SOURCE -shared -fPIC -o forcebindip.so forcebindip.c
 *
 * Usage: LD_PRELOAD=./forcebindip.so BIND_ADDRESS=1.2.3.4 wget ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <err.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	char *ip;
	struct in_addr inaddr;
	struct sockaddr_in sin;

	ip = getenv("BIND_ADDRESS");

	if (ip != NULL)
	{
		if(inet_aton(ip, &inaddr) == 0)
		{
			warnx("forcebindip: Invalid address: %s", ip);
		}
		else
		{
			sin.sin_family = AF_INET;
			sin.sin_port = 0;
			sin.sin_addr = inaddr;

			if(bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
			{
				warn("forcebindip: bind: %s", ip);
			}
		}
	}

	int (*orig_connect)(int, const struct sockaddr *, socklen_t) = dlsym(RTLD_NEXT, "connect");
	return orig_connect(sockfd, addr, addrlen);
}

