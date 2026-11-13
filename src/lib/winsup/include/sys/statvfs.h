/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: filesystem statistics (POSIX flavour).
 */
#ifndef __WINGSUP_SYS_STATVFS_H
#define __WINGSUP_SYS_STATVFS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ST_RDONLY	0x0001

struct statvfs {
	unsigned long f_bsize;
	unsigned long f_frsize;
	unsigned long f_blocks;
	unsigned long f_bfree;
	unsigned long f_bavail;
	unsigned long f_files;
	unsigned long f_ffree;
	unsigned long f_favail;
	unsigned long f_fsid;
	unsigned long f_flag;
	unsigned long f_namemax;
};

int statvfs(const char *path, struct statvfs *buf);
int fstatvfs(int fd, struct statvfs *buf);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_STATVFS_H */
