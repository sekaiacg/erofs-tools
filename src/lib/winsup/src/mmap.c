/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * POSIX memory mapping on top of mman-win32.
 */
#include "internal.h"

/*
 * The Win32 file mapping API only accepts 64 KiB aligned offsets, while POSIX
 * callers may pass any page aligned one.  Map the aligned down region and hand
 * back a pointer shifted by the remainder, then translate it back in munmap().
 */
#define WINGSUP_MAP_ALIGNMENT	65536

/* provided by mman.c, renamed so that the POSIX entry points stay ours */
void *mman_mmap(void *addr, size_t len, int prot, int flags, int fildes,
		off_t off);
int mman_munmap(void *addr, size_t len);

struct winsup_mmap_region {
	void *base;
	void *user;
	size_t mapped;
	struct winsup_mmap_region *next;
};

static CRITICAL_SECTION winsup_mmap_lock;
static struct winsup_mmap_region *winsup_mmap_regions;

static void winsup_init_mmap(void) __attribute__((constructor));
static void winsup_init_mmap(void)
{
	InitializeCriticalSection(&winsup_mmap_lock);
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	struct winsup_mmap_region *r;
	size_t delta;
	off_t aligned;
	void *base;

	if (!len || (flags & MAP_ANONYMOUS))
		return mman_mmap(addr, len, prot, flags, fd, off);

	aligned = off & ~(off_t)(WINGSUP_MAP_ALIGNMENT - 1);
	delta = (size_t)(off - aligned);
	if (!delta)
		return mman_mmap(addr, len, prot, flags, fd, off);

	base = mman_mmap(addr, len + delta, prot, flags, fd, aligned);
	if (base == MAP_FAILED)
		return MAP_FAILED;

	r = malloc(sizeof(*r));
	if (!r) {
		mman_munmap(base, len + delta);
		errno = ENOMEM;
		return MAP_FAILED;
	}
	r->base = base;
	r->user = (char *)base + delta;
	r->mapped = len + delta;

	EnterCriticalSection(&winsup_mmap_lock);
	r->next = winsup_mmap_regions;
	winsup_mmap_regions = r;
	LeaveCriticalSection(&winsup_mmap_lock);
	return r->user;
}

int munmap(void *addr, size_t len)
{
	struct winsup_mmap_region **pp, *r = NULL;
	void *base = addr;
	size_t mapped = len;

	EnterCriticalSection(&winsup_mmap_lock);
	for (pp = &winsup_mmap_regions; *pp; pp = &(*pp)->next) {
		if ((*pp)->user == addr) {
			r = *pp;
			*pp = r->next;
			base = r->base;
			mapped = r->mapped;
			break;
		}
	}
	LeaveCriticalSection(&winsup_mmap_lock);

	free(r);
	return mman_munmap(base, mapped);
}
