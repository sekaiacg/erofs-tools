/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: filesystem statistics.
 */
#ifndef __WINGSUP_SYS_STATFS_H
#define __WINGSUP_SYS_STATFS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct statfs {
	long f_type;
	long f_bsize;
	long f_blocks;
	long f_bfree;
	long f_bavail;
	long f_files;
	long f_ffree;
	long f_fsid;
	long f_namelen;
};

int statfs(const char *path, struct statfs *buf);
int fstatfs(int fd, struct statfs *buf);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_STATFS_H */
