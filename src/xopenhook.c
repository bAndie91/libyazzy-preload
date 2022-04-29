
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <X11/Xlib.h>

void _xopenhook_preload_run(const char * phase)
{
	char * xopenhook_enable = getenv("XOPENHOOK_ENABLE");
	if(xopenhook_enable != NULL && strcmp(xopenhook_enable, "1")==0)
	{
		unsetenv("XOPENHOOK_ENABLE");
		setenv("XOPENHOOK_PHASE", phase, 1);
		system(getenv("XOPENHOOK_COMMAND"));
		unsetenv("XOPENHOOK_PHASE");
		setenv("XOPENHOOK_ENABLE", xopenhook_enable, 1);
	}
}

Display* XOpenDisplay(const char * display_name)
{
	Display* (*real_XOpenDisplay)(const char *) = dlsym(RTLD_NEXT, "XOpenDisplay");
	
	Display* display_handle = real_XOpenDisplay(display_name);
	if(display_handle != NULL) _xopenhook_preload_run("post-XOpenDisplay");
	
	return display_handle;
}


#include <Xvlibint.h>

XvImageFormatValues * XvListImageFormats (
   Display 	*dpy,
   XvPortID 	port,
   int 		*num
){
	XvImageFormatValues* (*real_XvListImageFormats)(Display*, XvPortID, int*) = dlsym(RTLD_NEXT, "XvListImageFormats");

	_xopenhook_preload_run("pre-XvListImageFormats");
	XvImageFormatValues *ret = real_XvListImageFormats(dpy, port, num);
	_xopenhook_preload_run("post-XvListImageFormats");

	return ret;
}
