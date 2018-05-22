#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

//typedef ssize_t (*_read_f_t_)(int fd, void *buf, size_t count);
//static _read_f_t_ _read_f_ = NULL;

ssize_t read(int fd, void *buf, size_t count)
{
	int (*real_read)(int fd, void *buf, size_t count) = NULL;
	char fdpath[21];
	char *pwd;
	struct stat st;

	warnx("intercepted fd=%d", fd);

/*
	_read_f_ = (_read_f_t_)dlsym(RTLD_NEXT, "read");
	warnx("-> %p", _read_f_);
	if (_read_f_ == NULL)
			errx(1, "dlsym: %s", dlerror());
*/
	if(real_read == NULL)
	{
		void *handle = dlopen("/lib/i386-linux-gnu/libc.so.6", RTLD_LAZY);
		if (handle == NULL)
			errx(1, "dlopen: %s", dlerror());
		real_read = dlsym(handle, "read");
		if (real_read == NULL)
			errx(1, "dlopen: %s", dlerror());
	}

	snprintf(fdpath, 21, "/proc/self/fd/%d", fd);
	if(lstat(fdpath, &st) == 0)
	{
		if(major(st.st_rdev) == 5 && minor(st.st_rdev) == 0)
		{
			pwd = getenv("CUPS_PASSWORD");
			if(pwd != NULL)
			{
				if(count >= strlen(pwd))
				{
					sprintf(buf, "%s\n", pwd);
					return strlen(pwd)+1;
				}
				else
				{
					warnx("Buffer is too small, %d, %d.", count, stren(pwd));
				}
			}
			else
			{
				warnx("CUPS_PASSWORD is not set.");
			}
		}
	}
	else
	{
		warn("lstat: %s", fdpath);
	}
	return real_read(fd, buf, count);
//	return _read_f_(fd, buf, count);
}

// Fordítás: gcc -s -x c cupspwd.c -fPIC -shared -o cupspwd.so
// Futtatás: CUPS_PASSWORD=xxx LD_PRELOAD=./cupspwd.so lpstat -l -t
