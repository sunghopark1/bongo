/****************************************************************************
 * <Novell-copyright>
 * Copyright (c) 2001 Novell, Inc. All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, contact Novell, Inc.
 * 
 * To contact Novell about this file by physical or electronic mail, you 
 * may find current contact information at www.novell.com.
 * </Novell-copyright>
 ****************************************************************************/

#include <config.h>
#include <xplutil.h>
#include <xplservice.h>
#include <stdarg.h>

#if defined (SOLARIS) || defined (LINUX) || defined(S390RH)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/param.h>
#include <grp.h>
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#include <unistd.h>

#ifdef HAVE_KSTAT_H
#include <kstat.h>
#include <sys/sysinfo.h>
#endif


int
XplLookupUser(const char *username, uid_t *uid, gid_t *gid)
{
    struct passwd *pw;

#ifdef HAVE_GETPWNAM_R
    struct passwd pwBuf;
    int ret;
    
    char buffer[2048];
    
#if defined(__SVR4) && defined(__sun) && !defined(_POSIX_PTHREAD_SEMANTICS)
    errno = 0;
    pw = getpwnam_r(username, &pwBuf, buffer, sizeof(buffer));
    if (errno != 0 || pw == NULL) {
        printf ("errno: %d, pw: %p\n", errno, pw);
#else
    ret = getpwnam_r(username, &pwBuf, buffer, sizeof(buffer), &pw);
    if (ret != 0 || pw == NULL) {
	printf ("ret: %d, pw: %p\n", ret, pw);
#endif
	return -1;
    }
#else
    /* FIXME: need to do some locking around this.  No
     * uid twiddling is done in threads atm, but it
     * should be fixed anyway. */
    pw = getpwnam (username);
    if (pw == NULL) {
	return -1;
    }
#endif
    
    if (uid) {
	*uid = pw->pw_uid;
    }
    
    if (gid) {
	*gid = pw->pw_gid;
    }
    
    return 0;
}

int
XplSetEffectiveUser(const char *username)
{
    uid_t uid;
    gid_t gid;
    int ret;

    if (!username) {
	return 0;
    }

    /* Try to drop supplemental groups. */
    setgroups(0, NULL);

    if (XplLookupUser(username, &uid, &gid) < 0) {
	return -1;
    }
 
    ret = XplSetEffectiveGroupId(gid);
    if (ret < 0) {
	return ret;
    }

    ret = XplSetEffectiveUserId(uid);

    return ret;
}

int
XplSetEffectiveUserId(uid_t uid)
{
#ifdef MACOSX
    if (geteuid() == uid) {
        return 0;
    }
#endif
    return seteuid(uid);
}


int
XplSetEffectiveGroupId(gid_t gid)
{
#ifdef MACOSX
    if (getegid() == gid) {
        return 0;
    }
#endif
    return setegid(gid);
}


int
XplSetRealUser(const char *username)
{
    uid_t uid;
    gid_t gid;
    int ret;

    if (!username) {
	return 0;
    }

    /* Try to switch back to the real uid first */
    XplSetEffectiveUserId(getuid ());

    /* Try to drop supplemental groups. */
    setgroups(0, NULL);

    if (XplLookupUser(username, &uid, &gid) < 0) {
	return -1;
    }

    ret = XplSetRealGroupId(gid);
    if (ret < 0) {
	return ret;
    }
    
    ret = XplSetRealUserId(uid);
    return ret;
}

int
XplSetRealUserId(uid_t uid)
{
    return setuid(uid);
}


int
XplSetRealGroupId(gid_t gid)
{
    return setgid(gid);
}

FILE *
XplOpenTemp(const char *dir,
            const char *mode,
            char *filename)
{
    int fd;
    FILE *fh;

    strncpy(filename, dir, XPL_MAX_PATH);
    strncat(filename, "/XXXXXX", XPL_MAX_PATH);
    fd = mkstemp(filename);
    if (fd == -1) {
        return NULL;
    }

    fh = fdopen(fd, mode);

    if (fh == NULL) {
        unlink(filename);
        close (fd);
    }
    
    return fh;
}

/* Returns space used in and below Path in kilobytes */
unsigned long
XplGetDiskspaceUsed(unsigned char *Path)
{
	DIR				*DirPtr;
	struct dirent	*Slot;
	struct stat		StatBuf;
	long				Blocks=0L;
	char				Filename[XPL_MAX_PATH+1];

	if ((DirPtr=opendir(Path))==NULL) {
		return(0);
	}
	while ((Slot=readdir(DirPtr))!=NULL) {
		if (strcmp(Slot->d_name, "..")==0) {
			continue;
		}
		sprintf(Filename, "%s/%s", Path, Slot->d_name);
		if (lstat(Filename, &StatBuf)!=0) {
			continue;
		}
		if (S_ISDIR(StatBuf.st_mode)) {
			if (strcmp(Slot->d_name, ".") == 0) {
				Blocks+= StatBuf.st_blocks;
			} else {
				Blocks +=XplGetDiskspaceUsed(Filename);
			}
			continue;
		}
		Blocks+=StatBuf.st_blocks;
	}
	closedir(DirPtr);
	return(Blocks/2);
}

unsigned long
XplGetDiskspaceFree(unsigned char *Path)
{
#if HAVE_STATVFS
	struct statvfs	stv;

	if (statvfs(Path, &stv)==0) {
		return(((unsigned long long)stv.f_bfree*stv.f_frsize)/1024);
#elif HAVE_STATFS
	struct statfs	stv;

	if (statfs(Path, &stv)==0) {
		return(((unsigned long long)stv.f_bfree*stv.f_bsize)/1024);
#else
#error statfs or statvfs are not available
#endif
	} else {
		return(0x7f000000L);
	}
}

unsigned long
XplGetDiskBlocksize(unsigned char *Path)
{
#if HAVE_STATVFS
	struct statvfs	stv;

	if (statvfs(Path, &stv)==0) {
		return(stv.f_frsize);
#elif HAVE_STATFS
	struct statfs	stv;

	if (statfs(Path, &stv)==0) {
		return(stv.f_bsize);
#else
#error statfs or statvfs are not available
#endif
	} else {
		return(1024);
	}
}

int
XplTruncate(const char *path, int64_t length)
{
    return(truncate(path, length));
}

#ifdef HAVE_KSTAT_H
long
XplGetServerUtilization(unsigned long *PreviousSeconds, unsigned long *PreviousIdle)
{
	kstat_ctl_t             *kc;
   kstat_t                 *sysinfo_ks;
	kstat_t                 *cpu_ks;
   sysinfo_t					sysinfo_buff;
	cpu_stat_t              cpu_buff;
   unsigned long				CurrentSeconds;
   unsigned long 				CurrentIdle;
   unsigned long				AverageUtilization;

	kc = kstat_open();
	if (kc == NULL) {
		return(-1);
	}

   /* Get the number of cpu ticks (max of 100 per second) spent in idle */
   cpu_ks = kstat_lookup(kc, "cpu_stat", -1, NULL);                        /* Check first processor in the list.  */
	kstat_read(kc, cpu_ks, &cpu_buff);                                      /* Should be representative even on MP */
   CurrentIdle = cpu_buff.cpu_sysinfo.cpu[CPU_IDLE];

   /* Get the number of seconds since boot time */
	sysinfo_ks = kstat_lookup(kc, "unix", 0, "sysinfo");
   kstat_read(kc, sysinfo_ks, &sysinfo_buff);
   CurrentSeconds = sysinfo_buff.updates;
   
   kstat_close(kc);
  
   AverageUtilization = (100 - (CurrentIdle-*PreviousIdle)/(CurrentSeconds-*PreviousSeconds));
   *PreviousSeconds = CurrentSeconds;
   *PreviousIdle    = CurrentIdle;
	
	return(AverageUtilization);
}

#elif defined(__linux__)
#define	PROC_SUPER_MAGIC		0x9fa0
#define	PROCFS					"/proc"

long
XplGetServerUtilization(unsigned long *previousSeconds, unsigned long *previousIdle)
{
	struct statfs  sb;
	char				Buff2[1024], Buff1[1024], *Ptr;
	int				fd1, fd2;
	long				cpu[5];

	if (statfs(PROCFS, &sb) < 0 || sb.f_type != PROC_SUPER_MAGIC) {
                static int warned = 0;
                if (!warned) {
                    fprintf(stderr, "PROC filesystem is not mounted\n");
                    warned = 1;
                }
		return 0;
	}
	Ptr = Buff1;
	chdir(PROCFS);
	fd1 = open("stat", O_RDONLY);
	read(fd1, Buff1, sizeof(Buff2)-1);
	sleep(1);
	close(fd1);
	fd2 = open("stat", O_RDONLY);
	read(fd2, Buff2, sizeof(Buff2)-1);
	close(fd2);
	Ptr+=4;
	cpu[0] = strtoul(Ptr, &Ptr, 10);
	cpu[0] += strtoul(Ptr, &Ptr, 10);
	cpu[0] += strtoul(Ptr, &Ptr, 10);
	cpu[1] = strtoul(Ptr, NULL, 10);

	Ptr = Buff2;
	Ptr+=4;
	cpu[2] = strtoul(Ptr, &Ptr, 10);
	cpu[2] += strtoul(Ptr, &Ptr, 10);
	cpu[2] += strtoul(Ptr, &Ptr, 10);
	cpu[3] = strtoul(Ptr, NULL, 10);

	cpu[2] -= cpu[0];
	cpu[3] -= cpu[1];
	if (cpu[3] == 0) return(100);
	cpu[4] = (100 *cpu[2]) / (cpu[2] + cpu[3]);
	return(cpu[4]);
}

#endif



#endif

#ifdef NETWARE
#include <nit/nwdir.h>

int
XPLDosToLong(unsigned char *In, int PrefixLen, unsigned char *Out, int OutSize)
{
	unsigned char	*ptr, *OutPtr;
	int				len=0;
	BOOL				Last=TRUE;

	ptr=strchr(In+PrefixLen+1, '/');
	OutPtr=Out;

	do {
		if (ptr) {
			*ptr='\0';
		}

		if (NWGetNameSpaceEntryName(In, NWOS2_NAME_SPACE, OutSize-len, OutPtr)!=0) {
			Out[0]='\0';
			return(0);
		}
		len=strlen(Out);
		if (ptr) {
			Out[len]='/';
			len++;
			*ptr='/';
			ptr=strchr(ptr+1, '/');
		} else {
			Out[len]='\0';
			Last=FALSE;
		}
		OutPtr=Out+len;
	} while (ptr || Last);

	return(len);
}

int
XPLLongToDos(unsigned char *In, unsigned char *Out, int OutSize)
{
	unsigned char	*ptr, *OutPtr;
	int				len=0;
	BOOL				Last=TRUE;

	OutPtr=Out;
	
	ptr=strchr(In, ':');
	if (ptr) {
		if (ptr[1]=='\0') {
			len=strlen(In);
			if (len<OutSize) {
				strcpy(Out, In);
			} else {
				strncpy(Out, In, OutSize);
			}
			return(len);
		}
		memcpy(OutPtr, In, ptr-In+1);
		OutPtr+=ptr-In+1;
		ptr++;
		if (*ptr=='/') {
			*OutPtr='/';
			ptr++;
			OutPtr++;
		}
		ptr=strchr(ptr, '/');
	} else {
		ptr=strchr(In+1, '/');
	}

	do {
		if (ptr) {
			*ptr='\0';
		}

		if (NWGetNameSpaceEntryName(In, NWDOS_NAME_SPACE, OutSize-len, OutPtr)!=0) {
			Out[0]='\0';
			return(0);
		}
		len=strlen(Out);
		if (ptr) {
			Out[len]='/';
			len++;
			*ptr='/';
			ptr=strchr(ptr+1, '/');
		} else {
			Out[len]='\0';
			Last=FALSE;
		}
		OutPtr=Out+len;
	} while (ptr || Last);

	return(len);
}

unsigned long
XPLGetDiskspaceUsed(unsigned char *Path)
{
	struct AnswerStructure	answerBuffer[2];
	unsigned long				SpaceUsed;

	if (ReturnSpaceRestrictionForDirectory(Path, 2, (BYTE *)&answerBuffer, &SpaceUsed)==0) {
		SpaceUsed=(unsigned long)(answerBuffer[0].AMaximumAmount-answerBuffer[0].ACurrentAmount)*4;
	} else {
		SpaceUsed=0;
	}
	return(SpaceUsed);
}

unsigned long
XPLGetDiskspaceFree(unsigned char *Path)
{
	unsigned long Space;

	if (GetAvailableUserDiskSpace(Path, &Space)!=0) {
		return(0x7f000000L);
	} else {
		return(Space);
	}
}

unsigned long
XPLGetDiskBlocksize(unsigned char *PathIn)
{
	VOLUME_STATS	vi;
	unsigned char	*ptr, *ptr2;
	unsigned long	BytesPerBlock=512;
	int				PathVolNumber;
	char				Path[XPL_MAX_PATH+1];

	strcpy(Path, PathIn);
	/* Break down path */
	ptr=strchr(Path, ':');
	if (ptr) {
		*ptr = '\0';
		ptr2 = strchr(Path, '/');
		if (ptr2) {
			*ptr2 = '\0';
			GetVolumeNumber(ptr2+1, &PathVolNumber);
			GetVolumeInformation(0, PathVolNumber, sizeof(vi), &vi);
			BytesPerBlock=vi.sectorsPerBlock*512;
			*ptr=':';
			*ptr2='/';
		} else {
			GetVolumeNumber(Path, &PathVolNumber);
			GetVolumeInformation(0, PathVolNumber, sizeof(vi), &vi);
			BytesPerBlock=vi.sectorsPerBlock*512;
		}
	}
	return(BytesPerBlock);
}
#endif /* NETWARE */

#if defined(LIBC)
unsigned long XPLGetDiskspaceUsed(unsigned char *PathIn)
{
	DIR				*dirP;
	struct dirent	*dirEntry;
	unsigned long	used = 0;
	char				path[XPL_MAX_PATH + 1];

	if ((dirP = opendir(PathIn)) != NULL) {
		while ((dirEntry = readdir(dirP)) != NULL) {
			if (!(dirEntry->d_type & DT_DIR)) {
				used += dirEntry->d_size;
				continue;
			}

			if (dirEntry->d_name[0] != '.') {
				sprintf(path, "%s/%s", PathIn, dirEntry->d_name);
				used += XPLGetDiskspaceUsed(path);
			}
		}

		closedir(dirP);
	}

	return(used);
}

unsigned long XPLGetDiskspaceFree(unsigned char *PathIn)
{
	int						i;
	unsigned long			freeSpace;
	unsigned char			*ptr;
	unsigned char			*ptr2;
	char						path[XPL_MAX_PATH + 1];
	struct volume_info	vi;

	/* Break down path */
	ptr = strchr(PathIn, ':');
	if (ptr != NULL) {
		i = ptr - PathIn;
		memcpy(path, PathIn, i);
		path[i] = '\0';

		ptr2 = strchr(path, '/');
		if (ptr2 == NULL) {
			i = netware_vol_info_from_name(&vi, path);
		} else {
			i = netware_vol_info_from_name(&vi, ptr2 + 1);
		}

		if (i == 0) {
			/*	This number does not include space that is currently available from 
				deleted (limbo) files, nor space the could be reclaimed from the 
				suballocation file system.	*/				
			freeSpace = vi.SectorSize * vi.SectorsPerCluster * vi.FreedClusters;
		} else {
			freeSpace = 0x7f000000L;
		}
	} else {
		freeSpace = 0x7f000000L;
	}

	return(freeSpace);
}

unsigned long XPLGetDiskBlocksize(unsigned char *PathIn)
{
	int						i;
	unsigned long			bytesPerBlock;
	unsigned char			*ptr;
	unsigned char			*ptr2;
	char						path[XPL_MAX_PATH + 1];
	struct volume_info	vi;

	/* Break down path */
	ptr = strchr(PathIn, ':');
	if (ptr != NULL) {
		i = ptr - PathIn;
		memcpy(path, PathIn, i);
		path[i] = '\0';

		ptr2 = strchr(path, '/');
		if (ptr2 == NULL) {
			i = netware_vol_info_from_name(&vi, path);
		} else {
			i = netware_vol_info_from_name(&vi, ptr2 + 1);
		}

		if (i == 0) {
			bytesPerBlock = vi.SectorSize;
		} else {
			bytesPerBlock = 512;
		}
	} else {
		bytesPerBlock = 512;
	}

	return(bytesPerBlock);
}

long XplGetServerUtilization(unsigned long *PreviousSeconds, unsigned long *PreviousIdle)
{
	int					cpu = 0;
	struct cpu_info	info;

	/*
		To view information about all CPUs, pass zero the first time the function is 
		called and do not modify the returned value on subsequent calls. To view 
		information about a specific CPU, set sequence to the CPU number.	*/
	if (netware_cpu_info(&info, &cpu) == 0) {
		return(info.ProcessorUtilization);
	}

	return(0);
}
#endif

#ifdef WIN32

#include <io.h>

int
XplTruncate(const char *path, int64_t length)
{
    /* This needs to fail unless somebody wants to implement this function */
    return(-1);
}

long
XplGetServerUtilization(unsigned long *previousSeconds, unsigned long *previousIdle)
{
    return 1;
}

unsigned long
XplGetDiskspaceUsed(unsigned char *Path)
{
	unsigned long			Used=0L;
	char						PathBuffer[XPL_MAX_PATH+1];
	struct _finddata_t	FindData;
	long						Handle;

	sprintf(PathBuffer, "%s/*.*", Path);

	if ((Handle=_findfirst(PathBuffer, &FindData))==-1L) {
		return(0);
	}

	do {
		if (FindData.name[0]=='.' && ((FindData.name[1]=='\0') || (FindData.name[1]=='.' && FindData.name[2]=='\0'))) {
			continue;
		}
		if ((FindData.attrib & XPL_A_SUBDIR) && (FindData.name[0]!='.')) {
			sprintf(PathBuffer, "%s/%s", Path, FindData.name);
			Used+=XplGetDiskspaceUsed(PathBuffer);
		} else {
			Used+=FindData.size;
		}
	} while (_findnext(Handle, &FindData)==0);

	_findclose(Handle);

	return(Used/1024);
}

unsigned long
XplGetDiskspaceFree(unsigned char *Path)
{
	ULARGE_INTEGER	FreeBytes;
	ULARGE_INTEGER	Dummy1;
	ULARGE_INTEGER	Dummy2;

	if (GetDiskFreeSpaceEx(Path, &FreeBytes, &Dummy1, &Dummy2)) {
		return((unsigned long)(FreeBytes.QuadPart/1024));
	} else {
		return(0x7f000000L);
	}
}

unsigned long
XplGetDiskBlocksize(unsigned char *Path)
{
	return(1024);
}

FILE *
XplOpenTemp(const char *dir,
            char *mode,
            char *filename)
{
    if (GetTempFileName(dir, "xpl", 0, filename) != 0) {
        return fopen(filename, mode);
    } else {
        return NULL;
    }
}

#endif
