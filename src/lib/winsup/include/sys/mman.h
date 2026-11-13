/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: memory mapping is provided by the bundled mman-win32, a thin
 * wrapper around the Win32 file mapping API.
 */
#ifndef __WINGSUP_SYS_MMAN_H
#define __WINGSUP_SYS_MMAN_H

/* let the toolchain pick the default, mman.h would otherwise target XP */
#include <_mingw.h>
#include <mman.h>

#endif /* __WINGSUP_SYS_MMAN_H */
