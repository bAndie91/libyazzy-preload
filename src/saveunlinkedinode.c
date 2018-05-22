
#include <sys/stat.h>
#include <bits/posix1_lim.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

int unlinkat(int dirfd, const char *pathname, int flags)
{
        int (*real_unlinkat)(int dirfd, const char *pathname, int flags) = dlsym(RTLD_NEXT, "unlinkat");
        int retval;
        
        char fdlinkname[_POSIX_PATH_MAX];
        char atdir[_POSIX_PATH_MAX];
        char bkfilename[_POSIX_PATH_MAX];
        char timebuf[20];
	time_t timer;
	struct tm* tm_info;
        struct stat st;
        FILE *fd;

	atdir[0] = 0;


	if(fstatat(dirfd, pathname, &st, flags) == -1) {
		perror("fstatat");
		goto do_unlink;
	}
	
	if(dirfd != AT_FDCWD) {
		sprintf(fdlinkname, "/proc/self/fd/%d", dirfd);
		if(readlink(fdlinkname, atdir, _POSIX_PATH_MAX) == -1) {
			perror("readlink");
			goto do_unlink;
		}
	}
	else {
		if(getcwd(atdir, _POSIX_PATH_MAX) == NULL) {
			perror("getcwd");
			goto do_unlink;
		}
	}
	
	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuf, 20, "%Y-%m-%d %H:%M:%S", tm_info);
	
	
	do_unlink:
	retval = real_unlinkat(dirfd, pathname, flags);
	
	if(retval == 0 && atdir[0] != 0) {
		sprintf(bkfilename, "%s/%s", getenv("HOME"), ".unlinked_inodes");
		fd = fopen(bkfilename, "a");
		if(fd != NULL) {
			fprintf(fd, "%s %ld %s/%s\n", timebuf, (long)st.st_ino, atdir, pathname);
			fclose(fd);
		}
		else {
			perror("fopen");
		}
	}
	
	return retval;
}

// Fordítás: gcc -s -x c saveunlinkedinode.c -fPIC -shared -ldl -o saveunlinkedinode.so
// Futtatás: LD_PRELOAD=./saveunlinkedinode.so rm somefile
