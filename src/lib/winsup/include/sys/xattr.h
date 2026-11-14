/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: extended attributes do not exist, all calls fail with
 * ENOTSUP so that callers can fall back to their own handling.
 */
#ifndef __WINGSUP_SYS_XATTR_H
#define __WINGSUP_SYS_XATTR_H

#include <sys/types.h>
#include <stddef.h>

#define XATTR_CREATE	0x1
#define XATTR_REPLACE	0x2

#ifndef XATTR_NAME_SELINUX
#define XATTR_NAME_SELINUX	"security.selinux"
#endif
#ifndef XATTR_NAME_SMACK
#define XATTR_NAME_SMACK	"security.SMACK64"
#endif

#ifdef __cplusplus
extern "C" {
#endif

ssize_t getxattr(const char *path, const char *name, void *value, size_t size);
ssize_t lgetxattr(const char *path, const char *name, void *value, size_t size);
ssize_t fgetxattr(int fd, const char *name, void *value, size_t size);
ssize_t setxattr(const char *path, const char *name, const void *value,
		 size_t size, int flags);
ssize_t lsetxattr(const char *path, const char *name, const void *value,
		  size_t size, int flags);
ssize_t fsetxattr(int fd, const char *name, const void *value, size_t size,
		  int flags);
ssize_t listxattr(const char *path, char *list, size_t size);
ssize_t llistxattr(const char *path, char *list, size_t size);
ssize_t flistxattr(int fd, char *list, size_t size);
int removexattr(const char *path, const char *name);
int lremovexattr(const char *path, const char *name);
int fremovexattr(int fd, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_XATTR_H */
