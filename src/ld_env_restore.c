
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> //sleep
#include <string.h>
#include <dlfcn.h>
#include <bsd/stdlib.h>
#include <bsd/unistd.h>

#define TRUE 1
#define FALSE (!TRUE)

int restore_env(const char* envname)
{
	char* env;
	char* envnameold;
	
	envnameold = malloc(strlen(envname)+4+1);
	if(envnameold == NULL) abort();
	
	sprintf(envnameold, "%s_OLD", envname);
	
	env = getenv(envnameold);
	if(env == NULL)
	{
		unsetenv(envname);
	}
	else
	{
		setenv(envname, env, TRUE);
		unsetenv(envnameold);
	}
	free(envnameold);
}

int __libc_start_main(
	int (*main)(int, char **, char **),
	int argc, char **ubp_av,
	void (*init)(void),
	void (*fini)(void),
	void (*rtld_fini)(void),
	void (*stack_end)
)
{
	int (*original__libc_start_main)(
		int (*main)(int, char **, char **),
		int argc, char **ubp_av,
		void (*init)(void),
		void (*fini)(void),
		void (*rtld_fini)(void),
		void (*stack_end)
	);
	char* env;
	
	restore_env("LD_PRELOAD");
	restore_env("LD_LIBRARY_PATH");
	
	original__libc_start_main = dlsym(RTLD_NEXT, "__libc_start_main");
	return original__libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}
