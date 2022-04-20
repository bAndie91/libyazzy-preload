/*
 * gcc -D _GNU_SOURCE -shared -fPIC -ldl -o nosleep.so nosleep.c
 *
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <time.h>

int nanosleep (const struct timespec *req, struct timespec *rem)
{
	int (*orig_nanosleep)(const struct timespec *, struct timespec *) = dlsym(RTLD_NEXT, __func__);
	return orig_nanosleep(&(struct timespec){0, 0}, rem);
}
unsigned int sleep (unsigned int sec)
{
	int (*orig_sleep)(unsigned int) = dlsym(RTLD_NEXT, __func__);
	return orig_sleep((unsigned int)0);
}
int usleep (useconds_t usec)
{
	int (*orig_usleep)(useconds_t usec) = dlsym(RTLD_NEXT, __func__);
	return orig_usleep((useconds_t)0);
}
