/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: getrandom() backed by the CRT entropy source.
 */
#ifndef __WINGSUP_SYS_RANDOM_H
#define __WINGSUP_SYS_RANDOM_H

#include <sys/types.h>
#include <stddef.h>

#ifndef GRND_NONBLOCK
#define GRND_NONBLOCK	0x0001
#endif
#ifndef GRND_RANDOM
#define GRND_RANDOM	0x0002
#endif
#ifndef GRND_INSECURE
#define GRND_INSECURE	0x0004
#endif

#ifdef __cplusplus
extern "C" {
#endif

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_RANDOM_H */
