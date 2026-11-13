/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Shared includes and helpers for the POSIX compatibility shim.
 */
#ifndef __WINGSUP_INTERNAL_H
#define __WINGSUP_INTERNAL_H

/* rand_s() for getrandom(), see <stdlib.h> */
#define _CRT_RAND_S
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/random.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <utime.h>

/* symlink targets are stored in a fixed size buffer */
#define WINGSUP_SYMLINK_MAX	4096

/*
 * Cygwin compatible symlinks: a plain file carrying FILE_ATTRIBUTE_SYSTEM
 * whose content is the target string prefixed by "!<symlink>" and a
 * UTF-16LE BOM.  Decoding it lets the tools round-trip on Windows.
 */
#define CYGLINK_MAGIC		"!<symlink>"
#define CYGLINK_MAGIC_LEN	10
#define CYGLINK_HDR_LEN		(CYGLINK_MAGIC_LEN + 2)

/* translate a Win32 error code to errno, a zero code is a no-op */
void winsup_set_errno(DWORD err);

/*
 * Symlink helpers: native reparse points come first, the Cygwin style marker
 * files written by symlink() are the fallback.  Both return the target length
 * or -1 when the path is not such a symlink.
 */
int winsup_read_native_symlink(const char *path, char *out, size_t outsz);
int winsup_read_cygwin_symlink(const char *path, char *out, size_t outsz);
bool winsup_is_cygwin_symlink(const char *path);

#endif /* __WINGSUP_INTERNAL_H */
