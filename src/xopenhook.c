
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <X11/Xlib.h>

Display* XOpenDisplay(const char * display_name)
{
	Display* (*real_XOpenDisplay)(const char *) = dlsym(RTLD_NEXT, "XOpenDisplay");
	Display* display_handle = real_XOpenDisplay(display_name);
	if(display_handle != NULL) system(getenv("XOPENHOOK_COMMAND"));
	return display_handle;
}
