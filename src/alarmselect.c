#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

// select(17, [15 16], NULL, NULL, {32, 557492}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
        int (*real_select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) = NULL;
        int real_select_return = 0;
        struct timeval *timeo2 = { 3, 0 };


        if (real_select == NULL) {
                void *handle = dlopen(/*"/lib/libc.so.6"*/ "/lib/i386-linux-gnu/libc.so.6", RTLD_LAZY);
                if (handle == NULL) {
                        fprintf(stderr, "dlopen: %s\n", dlerror());
                        exit(1);
                }
                real_select = dlsym(handle, "select");
                if (real_select == NULL) {
                        fprintf(stderr, "dlsym: %s\n", dlerror());
                        exit(1);
                }
        }

        if(nfds == 17) timeout = timeo2;
        fprintf(stderr, "select timeout = %d.%d\n", timeout->tv_sec, timeout->tv_usec);
        real_select_return = real_select(nfds, readfds, writefds, exceptfds, timeout);
        return real_select_return;
}

// Fordítás: gcc -s -x c alarmselect.c -fPIC -shared -o alarmselect.so
// Futtatás: LD_PRELOAD=./alarmselect.so claws-mail
