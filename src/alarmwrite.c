#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

ssize_t write(int fd, const void *buf, size_t count)
{
        int (*real_write)(int fd, const void *buf, size_t count) = NULL;
        int real_write_return;
        int alarm_sec = 2;

        if (real_write == NULL) {
                void *handle = dlopen(/* "/lib/libc.so.6" */ "/lib/i386-linux-gnu/libc.so.6", RTLD_LAZY);
                if (handle == NULL) {
                        fprintf(stderr, "dlopen: %s\n", dlerror());
                        exit(1);
                }
                real_write = dlsym(handle, "write");
                if (real_write == NULL) {
                        fprintf(stderr, "dlsym: %s\n", dlerror());
                        exit(1);
                }
        }

        alarm(alarm_sec);
        real_write_return = real_write(fd, buf, count);
        alarm(0);
        return real_write_return;
}

// Fordítás: gcc -s -x c alarmwrite.c -fPIC -shared -o alarmwrite.so
// Futtatás: LD_PRELOAD=./alarmwrite.so x2vnc host:display
