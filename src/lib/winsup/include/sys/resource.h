/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: resource limits are emulated since the OS has no rlimit.
 */
#ifndef __WINGSUP_SYS_RESOURCE_H
#define __WINGSUP_SYS_RESOURCE_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long rlim_t;

struct rlimit {
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

#define RLIM_INFINITY	((rlim_t)-1)
#define RLIMIT_CPU	0
#define RLIMIT_FSIZE	1
#define RLIMIT_DATA	2
#define RLIMIT_STACK	3
#define RLIMIT_CORE	4
#define RLIMIT_NOFILE	7

int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_RESOURCE_H */
