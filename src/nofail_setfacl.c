/*
 * gcc -D _GNU_SOURCE -shared -fPIC -o nofail_setfacl.so nofail_setfacl.c
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

#define XATTRNAME "system.posix_acl_access"
#define NOTICEMSG "%s: Could not set acl"
#define NOTICEMSGFD (NOTICEMSG + 4)
#define SUCCESS 0
#define MIN(a,b) (((a)<(b))?(a):(b))

int setxattr (const char *path, const char *name, const void *value, size_t size, int flags)
{
	int (*orig_setxattr)(const char*, const char*, const void*, size_t, int) = dlsym(RTLD_NEXT, __func__);
	int result = orig_setxattr(path, name, value, size, flags);
	
	if(result < 0 && errno == EOPNOTSUPP)
	{
		if(strcmp(name, XATTRNAME)==0)
		{
			warnx(NOTICEMSG, path);
			result = SUCCESS;
		}
	}
	return result;
}

int lsetxattr (const char *path, const char *name, const void *value, size_t size, int flags)
{
	int (*orig_lsetxattr)(const char*, const char*, const void*, size_t, int) = dlsym(RTLD_NEXT, __func__);
	int result = orig_lsetxattr(path, name, value, size, flags);
	
	if(result < 0 && errno == EOPNOTSUPP)
	{
		if(strcmp(name, XATTRNAME)==0)
		{
			warnx(NOTICEMSG, path);
			result = SUCCESS;
		}
	}
	return result;
}

int fsetxattr (int fd, const char *name, const void *value, size_t size, int flags)
{
	int (*orig_fsetxattr)(int, const char*, const void*, size_t, int) = dlsym(RTLD_NEXT, __func__);
	int result = orig_fsetxattr(fd, name, value, size, flags);
	
	if(result < 0 && errno == EOPNOTSUPP)
	{
		if(strcmp(name, XATTRNAME)==0)
		{
			char linkpath[PATH_MAX];
			char path[PATH_MAX];
			sprintf(linkpath, "/proc/self/fd/%d", fd);
			ssize_t sz = readlink(linkpath, path, PATH_MAX);
			path[MIN(sz, PATH_MAX-1)] = '\0';
			warnx(NOTICEMSG, path);
			result = SUCCESS;
		}
	}
	return result;
}
