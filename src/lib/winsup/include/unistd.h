/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: POSIX I/O entry points missing from the Microsoft CRT.
 */
#ifndef __WINGSUP_UNISTD_H
#define __WINGSUP_UNISTD_H

#include_next <unistd.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

int fsync(int fd);
int fdatasync(int fd);

int getpagesize(void);

#define _SC_PAGESIZE		30
#define _SC_PAGE_SIZE		_SC_PAGESIZE
#define _SC_NPROCESSORS_ONLN	84
long sysconf(int name);

int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *path, char *buf, size_t bufsiz);

int link(const char *oldpath, const char *newpath);

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);

int chown(const char *path, uid_t owner, gid_t group);
int fchown(int fd, uid_t owner, gid_t group);
int lchown(const char *path, uid_t owner, gid_t group);
int fchmod(int fd, mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_UNISTD_H */
