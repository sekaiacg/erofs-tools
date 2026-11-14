/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: realpath() is not provided by the Microsoft CRT.
 */
#ifndef __WINGSUP_STDLIB_H
#define __WINGSUP_STDLIB_H

#include_next <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* resolved == NULL allocates the result like the POSIX version. */
char *realpath(const char *path, char *resolved);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_STDLIB_H */
