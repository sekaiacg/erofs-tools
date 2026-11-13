/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Filesystem statistics, unsupported on Windows.
 */
#include "internal.h"

int statfs(const char *path, struct statfs *buf)
{
	(void)path;
	(void)buf;
	errno = ENOTSUP;
	return -1;
}

int fstatfs(int fd, struct statfs *buf)
{
	(void)fd;
	(void)buf;
	errno = ENOTSUP;
	return -1;
}

int statvfs(const char *path, struct statvfs *buf)
{
	(void)path;
	(void)buf;
	errno = ENOTSUP;
	return -1;
}

int fstatvfs(int fd, struct statvfs *buf)
{
	(void)fd;
	(void)buf;
	errno = ENOTSUP;
	return -1;
}
