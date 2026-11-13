/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: mmap() is emulated with a private heap copy, which is
 * enough for the read-only mappings used by this project.
 */
#ifndef __WINGSUP_SYS_MMAN_H
#define __WINGSUP_SYS_MMAN_H

#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROT_NONE	0x0
#define PROT_READ	0x1
#define PROT_WRITE	0x2
#define PROT_EXEC	0x4

#define MAP_SHARED	0x01
#define MAP_PRIVATE	0x02
#define MAP_FIXED	0x10
#define MAP_ANONYMOUS	0x20
#define MAP_ANON	MAP_ANONYMOUS

#define MAP_FAILED	((void *)-1)

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int msync(void *addr, size_t length, int flags);
int mprotect(void *addr, size_t length, int prot);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_MMAN_H */
