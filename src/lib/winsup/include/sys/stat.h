/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: lstat(), mknod() and a POSIX mkdir() have no Microsoft CRT
 * counterpart, and several inode types are not described by its stat().
 */
#ifndef __WINGSUP_SYS_STAT_H
#define __WINGSUP_SYS_STAT_H

#include_next <sys/stat.h>

#ifndef S_IFLNK
#define S_IFLNK		0xA000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK	0xC000
#endif
#ifndef S_ISLNK
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Windows has no inode-level links, every lookup resolves the path. */
int lstat(const char *path, struct stat *st);

int mknod(const char *path, mode_t mode, dev_t dev);

int winsup_mkdir(const char *path, mode_t mode);

#ifdef __cplusplus
}
#endif

/* the CRT only provides the single argument _mkdir() */
#ifndef __WINGSUP_MKDIR
#define __WINGSUP_MKDIR
#define mkdir(path, mode)	winsup_mkdir((path), (mode))
#endif

#endif /* __WINGSUP_SYS_STAT_H */
