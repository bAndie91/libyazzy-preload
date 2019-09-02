/**
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <fnmatch.h>
#include <errno.h>
#include <unistd.h>

#define MYNAME "openhelper.so"


FILE* real_fopen(__const char *__restrict pathname, __const char *__restrict mode)
{
	FILE *(*the_real_fopen)(__const char *__restrict, __const char *__restrict) = dlsym(RTLD_NEXT, "fopen");
	return the_real_fopen(pathname, mode);
}

int openhelper(const char* requested_path, FILE** resulted_fh)
{
	/**
	 * return 0 or -1
	 * sets (explicitely or inheritedly) errno
	 * resulted_fd is a valid fd (pointing to a temp file which you don't need to delete) or -1
	 */
	
	char *only_pattern;
	char *helper_command;
	FILE *tmpf;
	pid_t child;
	
	*resulted_fh = NULL;
	only_pattern = getenv("OPENHELPER_FNMATCH");
	helper_command = getenv("OPENHELPER_COMMAND");
	
	if(helper_command == NULL)
	{
		return 0;
	}
	if(only_pattern == NULL)
	{
		fprintf(stderr, "%s: not set: OPENHELPER_FNMATCH\n", MYNAME);
		return 0;
	}
	if(fnmatch(only_pattern, requested_path, 0) != 0) return 0;
	
	if((tmpf = tmpfile()) == NULL)
	{
		fprintf(stderr, "%s: tmpfile: %s\n", MYNAME, strerror(errno));
		return -1;
	}
	
	child = fork();
	if(child == -1)
	{
		fprintf(stderr, "%s: fork: %s\n", MYNAME, strerror(errno));
		fclose(tmpf);
		return -1;
	}

	if(child == 0)
	{
		char* args[] = {"sh", "-c", helper_command, NULL};
		char tmpfn[16];
		FILE* tmpf2;
		
		close(fileno(stdin));
		close(fileno(stdout));
		sprintf(tmpfn, "/dev/fd/%d", fileno(tmpf));
		tmpf2 = real_fopen(tmpfn, "w+");
		if(tmpf2 == NULL)
		{
			fprintf(stderr, "%s: reopen temp file: %s\n", MYNAME, strerror(errno));
			_exit(errno);
		}
		dup2(fileno(tmpf2), fileno(stdout));
		unsetenv("OPENHELPER_COMMAND");
		setenv("OPENHELPER_FILE", requested_path, 1);
		execvp("sh", args);
		_exit(127);
	}
	
	int status;
	pid_t w;
	
	do {
		w = waitpid(child, &status, 0);
		if (w == -1)
		{
			fprintf(stderr, "%s: waitpid for helper command: %s\n", MYNAME, strerror(errno));
			return -1;
		}
		if (WIFEXITED(status)) {
			if(WEXITSTATUS(status) == 0)
			{
				*resulted_fh = tmpf;
				return 0;
			}
			fprintf(stderr, "%s: helper command exited %d: %s\n", MYNAME, WEXITSTATUS(status), strerror(WEXITSTATUS(status)));
			errno = WEXITSTATUS(status);
			return -1;
		} else if (WIFSIGNALED(status)) {
			fprintf(stderr, "%s: helper command process is killed by signal %d\n", MYNAME, WTERMSIG(status));
			errno = EIO;
			return -1;
		} else if (WIFSTOPPED(status)) {
			// pass
		} else if (WIFCONTINUED(status)) {
			// pass
		}
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

int open(const char *pathname, int flags, mode_t mode)
{
	FILE* replacement_fh;
	
	if(openhelper(pathname, &replacement_fh) != 0) return -1;
	if(replacement_fh != NULL)
	{
		return fileno(replacement_fh);
	}

	int (*real_open)(const char *, int, mode_t) = dlsym(RTLD_NEXT, __func__);
	return real_open(pathname, flags, mode);
}

int open64(const char *pathname, int flags, mode_t mode)
{
	FILE* replacement_fh;
	
	if(openhelper(pathname, &replacement_fh) != 0) return -1;
	if(replacement_fh != NULL)
	{
		return fileno(replacement_fh);
	}

	int (*real_open64)(const char *, int, mode_t) = dlsym(RTLD_NEXT, __func__);
	return real_open64(pathname, flags, mode);
}

FILE* fopen(__const char *__restrict pathname, __const char *__restrict mode)
{
	FILE* replacement_fh;
	
	if(openhelper(pathname, &replacement_fh) != 0) return NULL;
	if(replacement_fh != NULL)
	{
		return replacement_fh;
	}

	return real_fopen(pathname, mode);
}

FILE* fopen64(__const char *__restrict pathname, __const char *__restrict mode)
{
	FILE* replacement_fh;
	
	if(openhelper(pathname, &replacement_fh) != 0) return NULL;
	if(replacement_fh != NULL)
	{
		return replacement_fh;
	}

	FILE *(*real_fopen64)(__const char *__restrict, __const char *__restrict) = dlsym(RTLD_NEXT, __func__);
	return real_fopen64(pathname, mode);
}
