
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>

#define TRUE 1
#define FALSE 0
#define MBUF_LENGTH 1024

int xdev_allow(const char *pathname)
{
	char realpathname[PATH_MAX];
	FILE* proc_mounts = NULL;
	char mbuf[MBUF_LENGTH+1];
	char* space;
	char* mntdev;
	char* mntpnt;
	char* mntype;
	char* mntopt;
	
	warnx("xdev? %s", pathname);
	
	proc_mounts = fopen("/proc/mounts", "r");
	if(proc_mounts == NULL)
	{
		err(1, "/proc/mounts: open");
	}
	
	if(realpath(pathname, realpathname) != NULL)
	{
		while(fgets(mbuf, MBUF_LENGTH, proc_mounts) != NULL)
		{
			mntdev = mbuf;
			space = strchr(mbuf, ' ');
			if(space != NULL)
			{
				*space = 0;
				mntpnt = space + 1;
				space = strchr(mntpnt, ' ');
				if(space != NULL)
				{
					*space = 0;
					mntype = space + 1;
					space = strchr(mntype, ' ');
					if(space != NULL)
					{
						*space = 0;
						mntopt = space + 1;
						space = strchr(mntopt, ' ');
						if(space != NULL)
						{
							*space = 0;
							if(strncmp(realpathname, mntpnt, strlen(mntpnt)) == 0)
							{
								/* It's a mountpoint, or under a mountpoint */
								
								/* Deny if it is likely a network device */
								if(strncmp(mntdev, "//", 2) == 0 || strchr(mntdev, ':') != NULL)
									goto deny;
								
								//TODO XDEV_FSTYPE
								//TODO XDEV_OPTION
							}
						}
					}
				}
			}
		}
	}
	goto allow;
	
	deny:
	fclose(proc_mounts);
	errno = ECONNREFUSED;
	return FALSE;
	
	allow:
	fclose(proc_mounts);
	return TRUE;
}


int __lxstat64(int ver, const char *path, struct stat64 *buf)
{
	int (*real___lxstat64)(int, const char *, struct stat64 *) = dlsym(RTLD_NEXT, "__lxstat64");
	if(xdev_allow(path)) return real___lxstat64(ver, path, buf);
	return -1;
}

int __openat_2(int fd, const char *path, int oflag, mode_t mode)
{
	int (*real___openat_2)(int, const char*, int, mode_t) = dlsym(RTLD_NEXT, "__openat_2");
	if(xdev_allow(path)) return real___openat_2(fd, path, oflag, mode);
	return -1;
}

DIR* opendir(const char *pathname)
{
	DIR* (*real_opendir)(const char *) = dlsym(RTLD_NEXT, "opendir");
	if(xdev_allow(pathname)) return real_opendir(pathname);
	return NULL;
}

int xdev_ftw_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf
//  ,int (*real_cb)(const char*, const struct stat*, int, struct FTW*)
)
{
//	if(xdev_allow(fpath)) return real_cb(fpath, sb, typeflag, ftwbuf);
//	return FTW_STOP;
	return 0;
}

int nftw64(const char *dirpath,
//  int (*fgv)(const char*, const struct stat*, int, struct FTW*),
  __nftw64_func_t fgv,
  int nopenfd, int flags)
{
	int (*real_nftw64)(const char *, int (*)(const char*, const struct stat*, int, struct FTW*), int, int) = dlsym(RTLD_NEXT, "nftw64");
	return real_nftw64(dirpath, xdev_ftw_cb, nopenfd, flags);
}
