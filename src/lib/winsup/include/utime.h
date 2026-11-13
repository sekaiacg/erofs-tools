/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: the CRT utime() refuses to touch directories, this variant
 * opens them with FILE_FLAG_BACKUP_SEMANTICS like the Win32 API expects.
 */
#ifndef __WINGSUP_UTIME_H
#define __WINGSUP_UTIME_H

#include_next <utime.h>

#ifdef __cplusplus
extern "C" {
#endif

int winsup_utime(const char *path, const struct utimbuf *times);

#ifdef __cplusplus
}
#endif

#ifndef __WINGSUP_UTIME_MACRO
#define __WINGSUP_UTIME_MACRO
#define utime(path, times)	winsup_utime((path), (times))
#endif

#endif /* __WINGSUP_UTIME_H */
