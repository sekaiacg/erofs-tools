/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: POSIX limits which the Microsoft CRT does not define.
 */
#ifndef __WINGSUP_LIMITS_H
#define __WINGSUP_LIMITS_H

#include_next <limits.h>

#ifndef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX	2048
#endif
#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX	260
#endif

#endif /* __WINGSUP_LIMITS_H */
