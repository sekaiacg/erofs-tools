/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: the Microsoft CRT has no POSIX user/group ids.
 */
#ifndef __WINGSUP_SYS_TYPES_H
#define __WINGSUP_SYS_TYPES_H

#include_next <sys/types.h>

#ifndef __WINGSUP_UID_GID_T
#define __WINGSUP_UID_GID_T
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned int uint;
#endif

#endif /* __WINGSUP_SYS_TYPES_H */
