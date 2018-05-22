// gcc tcpmaxrt.c -fPIC -shared -ldl -o tcpmaxrt.so

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/tcp.h>


int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{

	int (*real_setsockopt)(int sockfd, int level, int optname, const void *optval, socklen_t optlen) = NULL;
	int real_setsockopt_return;
	int i1 = 1;

	if (real_setsockopt == NULL) {
		void *handle = dlopen("/lib/libc.so.6", RTLD_LAZY);
		if (handle == NULL) {
			fprintf(stderr, "dlopen: %s\n", dlerror());
			exit(1);
		}
		real_setsockopt = dlsym(handle, "setsockopt");
		if (real_setsockopt == NULL) {
			fprintf(stderr, "dlsym: %s\n", dlerror());
			exit(1);
		}
	}

	fprintf(stderr, "setsockopt: optname=%d optval=%d\n", optname, optval);
	fflush(stderr);

	real_setsockopt_return = real_setsockopt(sockfd, level, optname, optval, optlen);
	fprintf(stderr, "setsockopt %d\n", 
		real_setsockopt(sockfd, level, TCP_, (const void *)&i1, optlen)
	);
	fflush(stderr);
	
	return real_setsockopt_return;
}
