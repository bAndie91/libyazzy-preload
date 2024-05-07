#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>

//Credits go to https://catonmat.net/simple-ld-preload-tutorial and https://catonmat.net/simple-ld-preload-tutorial-part-two
//And of course to https://unix.stackexchange.com/a/648721/334883

//compile with gcc -nostartfiles -fpic -shared bindInterface.c -o bindInterface.so -ldl -D_GNU_SOURCE
//Use with BIND_INTERFACE=<network interface> LD_PRELOAD=./bindInterface.so <your program> like curl ifconfig.me

int socket(int family, int type, int protocol)
{
    //printf("MySocket\n"); //"LD_PRELOAD=./bind.so wget -O- ifconfig.me 2> /dev/null" prints two times "MySocket". First is for DNS-Lookup. 
                            //If your first nameserver is not reachable via bound interface, 
                            //then it will try the next nameserver until it succeeds or stops with name resolution error. 
                            //This is why it could take significantly longer than curl --interface wlan0 ifconfig.me
    char *bind_addr_env;
    struct ifreq interface;
    int *(*original_socket)(int, int, int);
    original_socket = dlsym(RTLD_NEXT,"socket");
    int fd = (int)(*original_socket)(family,type,protocol);
    bind_addr_env = getenv("BIND_INTERFACE");
    int errorCode;
    if ( bind_addr_env!= NULL && strlen(bind_addr_env) > 0)
    {
        //printf(bind_addr_env);
        strcpy(interface.ifr_name,bind_addr_env);
        errorCode = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &interface, sizeof(interface));
        if ( errorCode < 0)
        {
            perror("setsockopt");
            errno = EINVAL;
            return -1;
        };
    }
    else
    {
        //printf("Warning: Programm with LD_PRELOAD started, but BIND_INTERFACE environment variable not set\n");
        fprintf(stderr,"Warning: Programm started with LD_PRELOAD, but BIND_INTERFACE environment variable not set\n");
    }

    return fd;
}
