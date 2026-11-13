/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: dirent.d_type is not available, only the DT_* type values
 * are provided so that they can be used as plain file type constants.
 */
#ifndef __WINGSUP_DIRENT_H
#define __WINGSUP_DIRENT_H

#include_next <dirent.h>

#ifndef DT_UNKNOWN
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14
#endif

#endif /* __WINGSUP_DIRENT_H */
