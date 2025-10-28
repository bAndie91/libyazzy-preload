/*
  Attention! This code was originally generated and refined with assistance from OpenAI's ChatGPT (GPT-5 model).
  Without any warranty!
  License: AGPLv3 or later.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdlib.h>

static const char *inject_class = NULL;
static Window (*real_XCreateWindow)(Display *, Window, int, int, unsigned int,
                                    unsigned int, unsigned int, int, unsigned int,
                                    Visual *, unsigned long, XSetWindowAttributes *) = NULL;

Window XCreateWindow(Display *dpy, Window parent, int x, int y,
                     unsigned int width, unsigned int height, unsigned int border_width,
                     int depth, unsigned int class, Visual *visual,
                     unsigned long valuemask, XSetWindowAttributes *attr)
{
    if (!real_XCreateWindow)
        real_XCreateWindow = dlsym(RTLD_NEXT, "XCreateWindow");

    Window w = real_XCreateWindow(dpy, parent, x, y, width, height,
                                  border_width, depth, class, visual, valuemask, attr);

    if (!inject_class)
        inject_class = getenv("WM_CLASS");

    if (inject_class) {
        char buf[256];
        const char *inst = inject_class; // instance same as class
        size_t len1 = strlen(inst);
        size_t len2 = strlen(inject_class);
        if (len1 + len2 + 2 < sizeof(buf)) {
            strcpy(buf, inst);
            buf[len1 + 1] = '\0'; // NUL separator between instance and class
            strcpy(buf + len1 + 1, inject_class);
            XChangeProperty(dpy, w, XInternAtom(dpy, "WM_CLASS", False),
                            XA_STRING, 8, PropModeReplace,
                            (unsigned char *)buf, len1 + len2 + 2);
        }
    }

    return w;
}
