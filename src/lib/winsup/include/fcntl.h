/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: POSIX open() flags with no Windows counterpart.
 */
#ifndef __WINGSUP_FCNTL_H
#define __WINGSUP_FCNTL_H

#include_next <fcntl.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW	0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC	0
#endif
#ifndef O_DIRECT
#define O_DIRECT	0
#endif
#ifndef O_DSYNC
#define O_DSYNC		0
#endif
#ifndef O_SYNC
#define O_SYNC		0
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK	0
#endif

#endif /* __WINGSUP_FCNTL_H */
