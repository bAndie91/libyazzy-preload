// gcc ip_dscp.c -fPIC -shared -ldl -D_GNU_SOURCE -o ip_dscp.so

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/in.h>


int socket(int domain, int type, int protocol)
{
	int (*real_socket)(int, int, int) = dlsym(RTLD_NEXT, __func__);
	int socket_result = real_socket(domain, type, protocol);
	
	if (socket_result != -1)
	{
		char* DSCP = getenv("SOCKET_IP_DSCP");
		if (DSCP != NULL)
		{
			int dscp = strtod(DSCP, NULL);
			setsockopt(socket_result, IPPROTO_IP, IP_TOS, &dscp, sizeof(int));
		}
	}
	
	return socket_result;
}

