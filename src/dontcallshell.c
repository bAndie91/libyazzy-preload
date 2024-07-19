#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>

#define EQ(a, b) (strcmp((a), (b))==0)

extern char **environ;

/* the following macro mimics glibc converting variadic arguments into an array */

#define _BUILD_ARGV() \
	const char *argv[INT_MAX]; \
	int argv_idx; \
	for(argv_idx = 0;; argv_idx++) { \
		if(argv_idx >= INT_MAX) { va_end(argv_ap); errno = E2BIG; return -1; } \
		argv[argv_idx] = va_arg(argv_ap, char*); \
		if(argv[argv_idx] == NULL) break; \
	}

#define _BUILD_ENVP() \
	const char **envp = environ; \
	const char *next_arg = va_arg(argv_ap, char*); \
	if(next_arg != NULL) envp = next_arg;

#define BUILD_ARGV(start_arg) \
	va_list argv_ap; \
	va_start(argv_ap, start_arg); \
	_BUILD_ARGV(); \
	va_end(argv_ap)

#define BUILD_ARGV_AND_ENVP(start_arg) \
	va_list argv_ap; \
	va_start(argv_ap, start_arg); \
	_BUILD_ARGV(); \
	_BUILD_ENVP(); \
	va_end(argv_ap)



const char * dontcallshell_replace_exec_path(const char *name, char *const argv[])
{
	fprintf(stderr, "exec*: %s\n", name);
	fprintf(stderr, "arg 0 %s\n", argv[0]);
	fprintf(stderr, "arg 1 %s\n", argv[1]);
	fprintf(stderr, "arg 2 %s\n", argv[2]);
	if(argv[0] != NULL && argv[1] != NULL && strcmp(argv[1], "-c")
	   && ( EQ(name, "/bin/sh") || EQ(name, "/bin/bash") || EQ(name, "sh") || EQ(name, "bash") )
	  )
	{
		return "/usr/tool/notashell";
	}
	return name;
}


int execv(const char *path, char *const argv[])
{
	int (*real_execv)(const char *path, char *const argv[]) = dlsym(RTLD_NEXT, "execv");
	return real_execv(dontcallshell_replace_exec_path(path, argv), argv);
}

int execvp(const char *file, char *const argv[])
{
	int (*real_execvp)(const char *file, char *const argv[]) = dlsym(RTLD_NEXT, "execvp");
	return real_execvp(dontcallshell_replace_exec_path(file, argv), argv);
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	int (*real_execve)(const char *path, char *const argv[], char *const envp[]) = dlsym(RTLD_NEXT, "execve");
	return real_execve(dontcallshell_replace_exec_path(path, argv), argv, envp);
}

int execvpe(const char *file, char *const argv[], char *const envp[])
{
	int (*real_execvpe)(const char *file, char *const argv[], char *const envp[]) = dlsym(RTLD_NEXT, "execvpe");
	return real_execvpe(dontcallshell_replace_exec_path(file, argv), argv, envp);
}

int execl(const char *path, const char *arg, ...)
{
	BUILD_ARGV(arg);
	return execve(dontcallshell_replace_exec_path(path, argv), argv, environ);
}

int execlp(const char *file, const char *arg, ...)
{
	BUILD_ARGV(arg);
	return execvpe(dontcallshell_replace_exec_path(file, argv), argv, environ);
}

int execle(const char *path, const char *arg, ...)
{
	BUILD_ARGV_AND_ENVP(arg);
	return execve(dontcallshell_replace_exec_path(path, argv), argv, envp);
}


// execveat(...

// fexecve(...


int __posix_spawn (&pid, SHELL_PATH, 0, &spawn_attr,
		       (char *const[]){ (char *) SHELL_NAME,
					(char *) "-c",
					(char *) line, NULL },
		       __environ);
