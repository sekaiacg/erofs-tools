/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: shell-style pattern matching for the Android libcutils code.
 */
#ifndef __WINGSUP_FNMATCH_H
#define __WINGSUP_FNMATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define FNM_NOMATCH	1

#define FNM_NOESCAPE	0x01
#define FNM_PATHNAME	0x02
#define FNM_PERIOD	0x04
#define FNM_LEADING_DIR	0x08
#define FNM_CASEFOLD	0x10

int fnmatch(const char *pattern, const char *string, int flags);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_FNMATCH_H */
