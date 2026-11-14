/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: Linux device number helpers.
 */
#ifndef __WINGSUP_SYS_SYSMACROS_H
#define __WINGSUP_SYS_SYSMACROS_H

#include <sys/types.h>

#ifndef major
#define major(dev)	((unsigned int)(((dev) >> 8) & 0xfff))
#endif
#ifndef minor
#define minor(dev)	((unsigned int)(((dev) & 0xff) | (((dev) >> 12) & 0xfff00)))
#endif
#ifndef makedev
#define makedev(maj, min) \
	((dev_t)((((maj) & 0xfff) << 8) | ((min) & 0xff) | \
		 (((min) & 0xfff00) << 12)))
#endif

#endif /* __WINGSUP_SYS_SYSMACROS_H */
