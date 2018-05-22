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
#include <limits.h>

#ifndef PASS_MAX
#define PASS_MAX 255
#endif

char *getpass(const char *prompt)
{
	char * (*real_getpass)(const char *prompt) = NULL;
	//warnx("intercepted \"%s\"", prompt);
	char *env;
	char *getpass_pwd = NULL;

	if(real_getpass == NULL)
	{
		void *handle = dlopen("/lib/i386-linux-gnu/libc.so.6", RTLD_LAZY);
		if (handle == NULL)
			errx(1, "dlopen: %s", dlerror());
		real_getpass = dlsym(handle, "getpass");
		if (real_getpass == NULL)
			errx(1, "dlopen: %s", dlerror());
	}

	env = getenv("CUPS_PASSWORD");
	if(env != NULL)
	{
		getpass_pwd = malloc(PASS_MAX);
		strncpy(getpass_pwd, env, PASS_MAX);
	}
	else
	{
		env = getenv("CUPS_ASKPASS");
		if(env != NULL)
		{
			int pipe_in[2];
			int pipe_ex[2];
			FILE *pipe_in_1;
			FILE *pipe_ex_0;
			pid_t pid;
			int status;
	
			pipe(pipe_in);
			pipe(pipe_ex);
			pid = fork();
			if(pid == 0)
			{
				char *args[3] = {NULL, "stdin", NULL};
				args[0] = env;
				close(pipe_in[1]);
				dup2(pipe_in[0], STDIN_FILENO);
				close(pipe_ex[0]);
				dup2(pipe_ex[1], STDOUT_FILENO);
				exit(execvp(env, args));
			}
			else if(pid > 0)
			{
				close(pipe_in[0]);
				close(pipe_ex[1]);
				pipe_in_1 = fdopen(pipe_in[1], "w");
				pipe_ex_0 = fdopen(pipe_ex[0], "r");
	
				fprintf(pipe_in_1, "prompt=%s\nanswer=password\n", prompt);
				fflush(pipe_in_1);
				close(pipe_in[1]);
				
				getpass_pwd = malloc(PASS_MAX);
				if(fgets(getpass_pwd, PASS_MAX, pipe_ex_0) == NULL)
				{
					free(getpass_pwd);
					getpass_pwd = NULL;
				}
				else if(getpass_pwd[strlen(getpass_pwd)-1] == '\n')
				{
					getpass_pwd[strlen(getpass_pwd)-1] = '\0';
				}
				close(pipe_ex[0]);
				
				if(waitpid(pid, &status, 0) != -1)
				{
					if(WEXITSTATUS(status) == 0)
					{
					}
					else
					{
						free(getpass_pwd);
						getpass_pwd = NULL;
						warnx("%s: %s: exited %d", __FILE__, env, WEXITSTATUS(status));
					}
				}
				else
				{
					warn("%s: waitpid", __FILE__);
				}
			}
			else
			{
				close(pipe_in[0]);
				close(pipe_in[1]);
				close(pipe_ex[0]);
				close(pipe_ex[1]);
				warn("%s: fork", __FILE__);
			}
		}
		else
		{
			warnx("Neither CUPS_PASSWORD or CUPS_ASKPASS are set.");
			return real_getpass(prompt);
		}
	}

	return getpass_pwd;
}

