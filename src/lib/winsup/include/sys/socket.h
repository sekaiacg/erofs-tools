/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: only needed so that sources disabling their socket code can
 * still include the POSIX headers.
 */
#ifndef __WINGSUP_SYS_SOCKET_H
#define __WINGSUP_SYS_SOCKET_H

#include <sys/types.h>

#ifndef AF_UNIX
#define AF_UNIX		1
#define PF_UNIX		AF_UNIX
#endif
#ifndef AF_LOCAL
#define AF_LOCAL	AF_UNIX
#define PF_LOCAL	AF_UNIX
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM	1
#define SOCK_DGRAM	2
#endif
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC	0
#endif

#endif /* __WINGSUP_SYS_SOCKET_H */
