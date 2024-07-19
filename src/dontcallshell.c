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

#ifdef DEBUG
#define DEBUG_fprintf fprintf
#else
#define DEBUG_fprintf noop
void noop(void*, ...) { }
#endif

extern char **environ;

// glibc seems to use INT_MAX for this but i get SIGSEGV if try to allocate an array that big
#define ARGV_MAX_LEN 65536

/* the following macros mimic glibc converting variadic arguments into an array */

#define _BUILD_ARGV() \
	const char *argv[ARGV_MAX_LEN]; \
	int argv_idx; \
	argv[0] = arg; \
	for(argv_idx = 1; ; argv_idx++) { \
		if(argv_idx >= ARGV_MAX_LEN) { va_end(argv_ap); errno = E2BIG; return -1; } \
		argv[argv_idx] = va_arg(argv_ap, char*); \
		DEBUG_fprintf(stderr, "argv[%u] = %s\n", argv_idx, argv[argv_idx]); \
		if(argv[argv_idx] == NULL) break; \
	}

#define _BUILD_ENVP() \
	const char **envp = environ; \
	const char *next_arg = va_arg(argv_ap, char*); \
	if(next_arg != NULL) envp = next_arg;

// last fixed argument's name must be "arg"
#define BUILD_ARGV() \
	va_list argv_ap; \
	va_start(argv_ap, arg); \
	_BUILD_ARGV(); \
	va_end(argv_ap)

// last fixed argument's name must be "arg"
#define BUILD_ARGV_AND_ENVP() \
	va_list argv_ap; \
	va_start(argv_ap, arg); \
	_BUILD_ARGV(); \
	_BUILD_ENVP(); \
	va_end(argv_ap)



const char * dontcallshell_replace_exec_path(const char *cmd, char *const argv[])
{
	DEBUG_fprintf(stderr, "exec*: %s [%s %s %s ...]\n", cmd, argv[0], argv[1], argv[2]);
	if(argv[0] != NULL && argv[1] != NULL && EQ(argv[1], "-c")
	   && ( EQ(cmd, "/bin/sh") || EQ(cmd, "/bin/bash") || EQ(cmd, "sh") || EQ(cmd, "bash") )
	  )
	{
		return "/usr/tool/notashell";
	}
	return cmd;
}


int execv(const char *cmd, char *const argv[])
{
	DEBUG_fprintf(stderr, "execve %s [%s %s %s ...]\n", cmd, argv[0], argv[1], argv[2]);
	int (*real_execv)(const char *cmd, char *const argv[]) = dlsym(RTLD_NEXT, "execv");
	return real_execv(dontcallshell_replace_exec_path(cmd, argv), argv);
}

int execvp(const char *cmd, char *const argv[])
{
	DEBUG_fprintf(stderr, "execvp %s [%s %s %s ...]\n", cmd, argv[0], argv[1], argv[2]);
	int (*real_execvp)(const char *cmd, char *const argv[]) = dlsym(RTLD_NEXT, "execvp");
	return real_execvp(dontcallshell_replace_exec_path(cmd, argv), argv);
}

int execve(const char *cmd, char *const argv[], char *const envp[])
{
	DEBUG_fprintf(stderr, "execve %s [%s %s %s ...] [%s ...]\n", cmd, argv[0], argv[1], argv[2], envp[0]);
	int (*real_execve)(const char *cmd, char *const argv[], char *const envp[]) = dlsym(RTLD_NEXT, "execve");
	return real_execve(dontcallshell_replace_exec_path(cmd, argv), argv, envp);
}

int execvpe(const char *cmd, char *const argv[], char *const envp[])
{
	DEBUG_fprintf(stderr, "execvpe %s [%s %s %s ...] [%s ...]\n", cmd, argv[0], argv[1], argv[2], envp[0]);
	int (*real_execvpe)(const char *cmd, char *const argv[], char *const envp[]) = dlsym(RTLD_NEXT, "execvpe");
	return real_execvpe(dontcallshell_replace_exec_path(cmd, argv), argv, envp);
}

/* the following variadic functions calls to the above functions which are not variadic and do the replacements themself */

int execl(const char *cmd, const char *arg, ...)
{
	DEBUG_fprintf(stderr, "execl %s %s ...\n", cmd, arg);
	BUILD_ARGV();
	return execve(cmd, argv, environ);
}

int execlp(const char *cmd, const char *arg, ...)
{
	DEBUG_fprintf(stderr, "execlp %s %s ...\n", cmd, arg);
	BUILD_ARGV();
	return execvpe(cmd, argv, environ);
}

int execle(const char *cmd, const char *arg, ...)
{
	DEBUG_fprintf(stderr, "execle %s %s ...\n", cmd, arg);
	BUILD_ARGV_AND_ENVP();
	return execve(cmd, argv, envp);
}


// execveat(...

// fexecve(...

#include <spawn.h>

// FIXME override private functions?!
int __posix_spawn (pid_t *pid, const char *path,
	const posix_spawn_file_actions_t *file_actions,
	const posix_spawnattr_t *attrp, char *const argv[],
	char *const envp[])
{
	DEBUG_fprintf(stderr, "__posix_spawn %s [%s %s %s ...]\n", path, argv[0], argv[1], argv[2]);
	int (*real_func)(pid_t *, const char *,	const posix_spawn_file_actions_t *, const posix_spawnattr_t *, char *const [], char *const []) = dlsym(RTLD_NEXT, "__posix_spawn");
	return real_func(pid, dontcallshell_replace_exec_path(path, argv), file_actions, attrp, argv, envp);
}
