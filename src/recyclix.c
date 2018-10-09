
// see flow chart: https://github.com/bAndie91/libyazzy-preload/blob/master/recyclix-flow.svg

#include <sys/stat.h>
#include <bits/posix1_lim.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <regex.h>

#define REGEXP_SZLIMIT "^([0-9]+)([BkMG]?)(-([0-9]+)([BkMG]?))?$"
#define NMATCH_SZLIMIT 6
#define REGEXP_EXCLUDE "^!(.*)$"
#define NMATCH_EXCLUDE 2

off_t recyclix_tosize(const char * str, regoff_t num_start, int digits, regoff_t unit);
int recyclix_clonepath_recursive(char * p_path, char * p_donor);


int unlinkat(int dirfd, const char *pathname, int flags)
{
	int (*real_unlinkat)(int dirfd, const char *pathname, int flags) = dlsym(RTLD_NEXT, "unlinkat");
	int retval;

	char *recyclers;
	char *ptr, *tmp, *comma;
	char recycler[_POSIX_PATH_MAX];
	char recycled[_POSIX_PATH_MAX];
	char atdir[_POSIX_PATH_MAX];
	char fqpath[_POSIX_PATH_MAX];
	char mp_path[_POSIX_PATH_MAX];
	struct stat st1;
	struct stat st2;
	ino_t recycler_inode;
	char *txt_unset = "unset RECYCLER environment if you do not care about Recycle bin";
	off_t sz_min, sz_max;
	regex_t regex;
	regmatch_t pmatch[NMATCH_SZLIMIT];
	int regex_err;
	bool excluded;
	

	recyclers = getenv("RECYCLER");
	if(recyclers != NULL)
	{
		retval = fstatat(dirfd, pathname, &st1, (flags & ~AT_REMOVEDIR /* fstatat hates AT_REMOVEDIR */) | AT_SYMLINK_NOFOLLOW /* we are about to move/unlink the symlink itself */);
		if(retval != 0)
		{
			warn("%s", pathname);
		}
		else
		{
			while(1)
			{
				ptr = strchrnul(recyclers, ':');
				strncpy(recycler, recyclers, ptr - recyclers);
				recycler[ptr - recyclers] = '\0';

				if(strncmp(recycler, "~/", 2) == 0)
				{
					tmp = getenv("HOME");
					if(tmp == NULL)
					{
						warnx("Could not resolve: %s", recycler);
						recycler[0] = '\0';
					}
					else
					{
						snprintf(recycler, strlen(tmp) + 1 /* slash */ + (ptr - recyclers - 2 /* tilde+slash*/) + 1 /* 0x00 */, "%s/%s", tmp, recyclers + 2);
					}
				}
				
				if(strlen(recycler) > 0)
				{
					/* Found a Recycle Bin */
					#ifdef DEBUG
					warnx("RECYCLER %s", recycler);
					#endif
					
					sz_min = 1;  // ignore empty files by default
					sz_max = 0;  // no upper size limit by default
					excluded = false;
					
					/* Reading options for this Recycle Bin */
					comma = strchr(recycler, ',');
					if(comma != NULL)
					{
						comma[0] = '\0';
						comma++;
					}
					while(comma != NULL && strlen(comma) > 0)
					{
						/* comma points to the rest of options */
						tmp = strchr(comma, ',');
						if(tmp != NULL)
						{
							tmp[0] = '\0';
							/* comma now points to a single option */
							tmp++;
							/* tmp now points to the next option(s) */
						}
						
						
						regex_err = regcomp(&regex, REGEXP_SZLIMIT, REG_EXTENDED | REG_ICASE);
						if(regex_err != 0)
						{
							goto regexp_error;
						}
						else if(regexec(&regex, comma, NMATCH_SZLIMIT, pmatch, 0) == 0)
						{
							#ifdef DEBUG
							int i;
							char *s;
							char *e;
							for(i=0; i<NMATCH_SZLIMIT; i++)
							{
								if(pmatch[i].rm_so == -1) break;
								s = comma + pmatch[i].rm_so;
								e = comma + pmatch[i].rm_eo;
								if(i == 0) fprintf(stderr, "$& eq ");
								else fprintf(stderr, "$%d eq ", i);
								fprintf(stderr, "'%.*s' [%p:%p]\n", (e - s), s, s, e);
							}
							#endif
							sz_min = recyclix_tosize(comma, pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so, pmatch[2].rm_so);
							sz_max = recyclix_tosize(comma, pmatch[4].rm_so, pmatch[4].rm_eo - pmatch[4].rm_so, pmatch[5].rm_so);
							#ifdef DEBUG
							warnx("size min %lu, size max %lu.", sz_min, sz_max);
							#endif
							if(sz_max != 0 && sz_min > sz_max)
							{
								warnx("recyclix: Insane size limits: %lu-%lu", sz_min, sz_max);
								errno = ECANCELED;
								return -1;
							}
							goto next_option;
						}

						regex_err = regcomp(&regex, REGEXP_EXCLUDE, REG_EXTENDED | REG_ICASE);
						if(regex_err != 0)
						{
							goto regexp_error;
						}
						else if(regexec(&regex, comma, NMATCH_EXCLUDE, pmatch, 0) == 0)
						{
							strncpy(mp_path, comma + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so + 1);  /* mp_path is carrying user defined regex pattern */
							regex_err = regcomp(&regex, mp_path, REG_EXTENDED);
							if(regex_err != 0)
							{
								goto regexp_error;
							}
							else
							{
								regex_err = regexec(&regex, pathname, 0, pmatch, 0);
								#ifdef DEBUG
								warnx("exclude: '%s' %c~ /%s/", pathname, regex_err == 0 ? '=' : '!', comma+1);
								#endif
								excluded = regex_err == 0 ? true : false;
							}
							goto next_option;
						}

						regexp_error:
						regerror(regex_err, &regex, mp_path, sizeof(mp_path));  /* mp_path is carrying the regex error message */
						warnx("recyclix: Unrecognized option: '%s' (%s)", comma, mp_path);
						errno = ECANCELED;
						return -1;
						
						next_option:
						/* Read the next option (or end) */
						comma = tmp;
					}
					
					retval = stat(recycler, &st2);
					recycler_inode = st2.st_ino;
					if(retval != 0)
					{
						warn("stat: %s", recycler);
					}
					else
					{
						if(st2.st_dev != st1.st_dev)
						{
							/* Cross-device condition */
							#ifdef DEBUG
							warnx("xdev: %03u,%03u (looking for %03u,%03u)", major(st2.st_dev), minor(st2.st_dev), major(st1.st_dev), minor(st1.st_dev));
							#endif
						}
						else if(st1.st_size < sz_min || (sz_max != 0 && st1.st_size > sz_max))
						{
							/* File size exceeds limit */
							#ifdef DEBUG
							warnx("filesize %lu, min %lu, max %lu (0=infinite)", st1.st_size, sz_min, sz_max);
							#endif
						}
						else if(excluded)
						{
							/* pathname matched to exclude criteria */
							#ifdef DEBUG
							warnx("filename matched to exclude criteria");
							#endif
						}
						else
						{
							/* Recycle Bin and the subject file are on the same fs */
							/* ptr no longer points to the next RECYCLER */
							
							if(pathname[0] == '/')
							{
								/* Absolute pathname given */
								strcpy(fqpath, pathname);
							}
							else
							{
								/* Relative pathname given */
								if(dirfd == AT_FDCWD)
								{
									if(getcwd(atdir, _POSIX_PATH_MAX) == NULL)
									{
										warn("getcwd");
										retval = -1;
										break;
									}
								}
								else
								{
									sprintf(fqpath, "/proc/self/fd/%d", dirfd);
									retval = readlink(fqpath, atdir, _POSIX_PATH_MAX);
									if(retval == -1)
									{
										warn("readlink: %s", fqpath);
										break;
									}
									atdir[retval] = '\0';
								}
								
								sprintf(fqpath, "%s/%s", atdir, pathname);
							}
							
							strcpy(atdir, fqpath);
							tmp = dirname(atdir);  /* atdir got changed */
							ptr = realpath(tmp, NULL);
							if(ptr == NULL)
							{
								warn("realpath: %s", tmp);
								free(ptr);
								break;
							}
							/* ptr now points to the resolved path name of fqpath's parent directory */
							strcpy(atdir, fqpath);
							tmp = (char*)basename(atdir);  /* atdir got changed */
							sprintf(fqpath, "%s/%s", ptr, tmp);
							free(ptr);
							
							retval = 0;
							strcpy(mp_path, fqpath);
							while(1)
							{
								/* Looking for filesystem's root */
								strcpy(atdir, mp_path);
								tmp = dirname(atdir);  /* dirname will change atdir */
								if(strcmp(tmp, "//") == 0)
								{
									tmp[1] = '\0';
								}
								retval = stat(tmp, &st2);
								if(retval != 0)
								{
									warn("stat: %s", tmp);
									break;
								}
								
								if(st2.st_ino == recycler_inode)
								{
									/* removing from under the Recycler really removes the file */
									goto real_unlink;
									break;
								}
								
								if(strcmp(mp_path, "/") == 0 && strcmp(tmp, "/") == 0)
								{
									/* fqpath is on the root fs */
									break;
								}
								if(st1.st_dev != st2.st_dev)
								{
									/* mp_path is the mountpoint */
									break;
								}
								/* Check the parent directory */
								strcpy(mp_path, tmp);
							}
							if(retval != 0) break;
							
							sprintf(recycled, "%s/%s", recycler, fqpath + strlen(mp_path));
							#ifdef DEBUG
							warnx("Filesystem root: %s", mp_path);
							warnx("Recycled file: %s", recycled);
							#endif
							
							/* Copy directory path onto Recycler */
							strcpy(atdir, recycled);
							if(S_ISDIR(st1.st_mode))
							{
								/* Clone the file (actually the directory) to be removed itself */
								tmp = atdir;
								ptr = fqpath;
							}
							else
							{
								/* Clone the file's parent directory  */
								tmp = dirname(atdir);
								ptr = dirname(fqpath);
							}
							retval = recyclix_clonepath_recursive(tmp, ptr);
							if(retval != 0)
							{
								warn("mkdir: %s", tmp);
								break;
							}
							
							if(S_ISDIR(st1.st_mode))
							{
								goto real_unlink;
							}
							else
							{
								retval = renameat(dirfd, pathname, 0, recycled);
								if(retval != 0)
								{
									warn("renameat");
									warnx("Failed to drop file to the Recycler, %s.", txt_unset);
								}
							}
							
							/* ptr is unusable to continue */
							break;
						}
						/* Looking for next Recycler ... */
					}
				}
				
				if(ptr[0] == '\0')
				{
					#ifdef DEBUG
					warnx("There is no valid Recycler, removing file.");
					#endif
					goto real_unlink;
					break;
				}
				else
				{
					recyclers = ptr+1;
				}
			}
		}
	}
	else
	{
		real_unlink:
		retval = real_unlinkat(dirfd, pathname, flags);
	}

	return retval;
}

int recyclix_clonepath_recursive(char * p_path, char * p_donor)
{
	int (*real_unlink)(const char *pathname) = dlsym(RTLD_NEXT, "unlink");
	int retval;
	char path[_POSIX_PATH_MAX];
	char donor[_POSIX_PATH_MAX];
	char * parent;
	char * dparent;
	struct stat st;
	struct stat donor_st;
	
	#ifdef DEBUG
	warnx("%s: %s", __func__, p_path);
	#endif

	/* save stats of directory being cloned */	
	retval = stat(p_donor, &donor_st);
	if(retval == -1)
	{
		warn("stat: %s", p_donor);
	}
	else
	{
		try_mkdir:
		retval = mkdir(p_path, donor_st.st_mode);
		if(retval == -1)
		{
			if(errno == ENOENT || errno == ENOTDIR)
			{
				strcpy(path, p_path);
				strcpy(donor, p_donor);
				parent = dirname(path);
				dparent = dirname(donor);
				
				retval = real_unlink(parent);
				if(retval == 0)
				{
					goto try_mkdir;
				}
				else if(errno == ENOENT)
				{
					retval = recyclix_clonepath_recursive(parent, dparent);
					if(retval == 0)
					{
						goto try_mkdir;
					}
				}
				else
				{
					warn("unlink: %s", parent);
				}
			}
			else if(errno == EEXIST)
			{
				retval = stat(p_path, &st);
				if(retval == -1)
				{
					warn("stat: %s", p_path);
				}
				else
				{
					if(S_ISDIR(st.st_mode))
					{
						chmod(p_path, donor_st.st_mode);
						retval = 0;
					}
					else
					{
						retval = real_unlink(p_path);
						if(retval == 0)
						{
							goto try_mkdir;
						}
						else
						{
							warn("unlink: %s", p_path);
						}
					}
				}
			}
			else
			{	
				warn("mkdir: %s", p_path);
			}
		}
	}
	if(retval == 0)
	{
		chown(p_path, donor_st.st_uid, donor_st.st_gid);
	}
	return retval;
}

off_t recyclix_tosize(const char * str, regoff_t num_start, int digits, regoff_t unit)
{
	char number[16];
	off_t size;
	
	if(num_start == -1)
	{
		return 0;
	}
	else
	{
		snprintf(number, 16>digits+1 ? digits+1 : 16, "%s", str+num_start);
		size = atoi(number);
	}
	if(str[unit] == 'k' || str[unit] == 'K') size *= 1024;
	else if(str[unit] == 'm' || str[unit] == 'M') size *= 1024*1024;
	else if(str[unit] == 'g' || str[unit] == 'G') size *= 1024*1024*1024;
	return size;
}

int unlink(const char *pathname)
{
	return unlinkat(AT_FDCWD, pathname, 0);
}

