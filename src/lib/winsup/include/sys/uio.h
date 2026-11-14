/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: scatter/gather I/O declarations.
 */
#ifndef __WINGSUP_SYS_UIO_H
#define __WINGSUP_SYS_UIO_H

#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __WINGSUP_IOVEC
#define __WINGSUP_IOVEC
struct iovec {
	void *iov_base;
	size_t iov_len;
};
#endif

#ifndef UIO_MAXIOV
#define UIO_MAXIOV 1024
#endif

ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_SYS_UIO_H */
