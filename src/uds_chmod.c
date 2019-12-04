
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <string.h>
#include <sys/stat.h>


int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int (*real_bind)(int, const struct sockaddr *, socklen_t) = dlsym(RTLD_NEXT, __func__);
	int bind_result = real_bind(sockfd, addr, addrlen);
	
	if (bind_result != -1)
	{
		char* path = getenv("UDS_CHMOD_PATH");
		char* mode_oct = getenv("UDS_CHMOD_MODE");
		
		if (path != NULL && mode_oct != NULL)
		{
			struct sockaddr_un *uds_addr = (struct sockaddr_un *) addr;
			if(uds_addr->sun_path[0] != '\0')
			{
				//fprintf(stderr, "uds_chmod: uds path '%s'\n", uds_addr->sun_path);
				
				if(strcmp(uds_addr->sun_path, path)==0)
				{
					unsigned int mode;
					if(sscanf(mode_oct, "%o", &mode) == 1)
					{
						chmod(path, (mode_t)mode);
						// ignore errors here
					}
					else
					{
						fprintf(stderr, "uds_chmod: wrong octal mode '%s'\n", mode_oct);
					}
				}
			}
		}
	}
	
	return bind_result;
}
