/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: glibc stdio locking helpers, which are no-ops here.
 */
#ifndef __WINGSUP_STDIO_EXT_H
#define __WINGSUP_STDIO_EXT_H

#include <stdio.h>

#define FSETLOCKING_QUERY	0
#define FSETLOCKING_INTERNAL	1
#define FSETLOCKING_BYCALLER	2

#define __fsetlocking(fp, type)	((void)(fp), (void)(type), 0)
#define __fpurge(fp)		((void)(fp))
#define __fpending(fp)		(0)
#define __freading(fp)		(0)

#endif /* __WINGSUP_STDIO_EXT_H */
