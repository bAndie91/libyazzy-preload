/*
 * gcc -ldl -D _GNU_SOURCE -shared -fPIC -o ignore_read_lock.so ignore_read_lock.c
 *
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <fcntl.h>
#include <stdarg.h>

#define SUCCESS 0
#define MIN(a,b) (((a)<(b))?(a):(b))


int fcntl(int fd, int cmd, ...)
{
	va_list vargs;
	va_start(vargs, cmd);
	void* arg = va_arg(vargs, void*);
	void* arg2 = va_arg(vargs, void*);
	va_end(vargs);
	struct flock *lk = arg;
	
	int (*orig_fcntl)(int, int, ...) = dlsym(RTLD_NEXT, __func__);
	int result = orig_fcntl(fd, cmd, arg, arg2);
	
	//warnx("cmd = %d l_type = %d result = %d errno = %d", cmd, lk->l_type, result, errno);
	
	if(cmd == F_SETLK64 && lk->l_type == F_RDLCK && result != SUCCESS && errno == EAGAIN)
	{
		char linkpath[PATH_MAX];
		char path[PATH_MAX];
		sprintf(linkpath, "/proc/self/fd/%d", fd);
		ssize_t sz = readlink(linkpath, path, PATH_MAX);
		path[MIN(sz, PATH_MAX-1)] = '\0';
		warnx("%s: failed to acquire read lock, passing by", path);
		result = SUCCESS;
	}
	return result;
}

