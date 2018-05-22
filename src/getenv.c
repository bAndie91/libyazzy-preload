#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dlfcn.h>

char *getenv (const char *name)
{
	char *(*real_getenv)(const char *name) = dlsym(RTLD_NEXT, "getenv");
	char *retptr;

	retptr = real_getenv(name);
	if (retptr == NULL)
		fprintf(stderr, "getenv(\"%s\") = NULL\n", name);
	else
		fprintf(stderr, "getenv(\"%s\") = \"%s\"\n", name, retptr);

	return retptr;
}
