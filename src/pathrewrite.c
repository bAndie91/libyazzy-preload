/**
 * pathrewrite.c
 *
 * Compile with: gcc -D _GNU_SOURCE -shared -fPIC -o pathrewrite.so pathrewrite.c
 *
 * Usage: LD_PRELOAD=./pathrewrite.so PATHREWRITE_FROM=/usr/bin PATHREWRITE_TO=/usr/local/bin someprogramm ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <bits/posix1_lim.h>


void pathrewrite_rewrite(const char *orig, char *rw, const char *caller_name)
{
	char temp[_POSIX_PATH_MAX];
	char *pattern;
	char *newstring;
	char *index;
	
	strcpy(rw, orig);
	pattern = getenv("PATHREWRITE_FROM");
	newstring = getenv("PATHREWRITE_TO");
	
	if(pattern != NULL && newstring != NULL)
	{
		index = strstr(rw, pattern);
		if(index != NULL)
		{
			//fprintf(stderr, "index = %s\n", index);
			strcpy(temp, &index[strlen(pattern)]);
			strcpy(index, newstring);
			strcpy(&index[strlen(newstring)], temp);
			fprintf(stderr, "pathrewrite.so: %s(%s -> %s)\n", caller_name, orig, rw);
		}
	}
}


int __lxstat(int ver, const char *path, struct stat *buf)
{
	char pathrw[_POSIX_PATH_MAX];
	
	pathrewrite_rewrite(path, pathrw, __func__);
	
	int (*orig___lxstat)(int, const char *, struct stat *) = dlsym(RTLD_NEXT, "__lxstat");
	return orig___lxstat(ver, pathrw, buf);
}

int open(const char *pathname, int flags, mode_t mode)
{
	char pathrw[_POSIX_PATH_MAX];

	pathrewrite_rewrite(pathname, pathrw, __func__);
	
	int (*orig_open)(const char *, int, mode_t) = dlsym(RTLD_NEXT, "open");
	return orig_open(pathrw, flags, mode);
}

int open64(const char *pathname, int flags, mode_t mode)
{
	char pathrw[_POSIX_PATH_MAX];

	pathrewrite_rewrite(pathname, pathrw, __func__);

	int (*orig_open64)(const char *, int, mode_t) = dlsym(RTLD_NEXT, "open64");
	return orig_open64(pathrw, flags, mode);
}

int creat()
ssize_t readlink()
int __open64_2()
int __lxstat64()
// perhaps not needed // int execve()


int __open_2(const char* pathname, int flags)
{
	char pathrw[_POSIX_PATH_MAX];

	pathrewrite_rewrite(pathname, pathrw, __func__);

	int (*orig___open_2)(const char *, int) = dlsym(RTLD_NEXT, "__open_2");
	return orig___open_2(pathrw, flags);
}

FILE *fopen(__const char *__restrict pathname, __const char *__restrict mode)
{
	char pathrw[_POSIX_PATH_MAX];

	pathrewrite_rewrite(pathname, pathrw, __func__);
	
	FILE *(*orig_fopen)(__const char *__restrict, __const char *__restrict) = dlsym(RTLD_NEXT, "fopen");
	return orig_fopen(pathrw, mode);
}
