/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: only the terminal size query (TIOCGWINSZ) is supported.
 */
#ifndef __WINGSUP_SYS_IOCTL_H
#define __WINGSUP_SYS_IOCTL_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __WINGSUP_WINSIZE
#define __WINGSUP_WINSIZE
struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};
#endif

#define TIOCGWINSZ	0x5413

int ioctl(int fd, unsigned long request, ...);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_IOCTL_H */
