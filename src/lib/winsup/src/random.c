/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * getrandom() backed by the CRT entropy source.
 */
#include "internal.h"

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
	unsigned char *p = buf;
	size_t left = buflen;

	(void)flags;
	while (left) {
		unsigned int r;
		size_t n = left < sizeof(r) ? left : sizeof(r);

		if (rand_s(&r) != 0) {
			errno = EIO;
			return -1;
		}
		memcpy(p, &r, n);
		p += n;
		left -= n;
	}
	return (ssize_t)buflen;
}
