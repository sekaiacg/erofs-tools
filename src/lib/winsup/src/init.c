/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Process wide initialisation and the POSIX identity stubs.
 */
#include "internal.h"

/*
 * The CRT translates CRLF sequences in text mode, which would corrupt image
 * data and extracted files, so make every open binary by default.
 */
static void winsup_set_binary_mode(void) __attribute__((constructor));
static void winsup_set_binary_mode(void)
{
	_set_fmode(_O_BINARY);
}

uid_t getuid(void)
{
	return 0;
}

uid_t geteuid(void)
{
	return 0;
}

gid_t getgid(void)
{
	return 0;
}

gid_t getegid(void)
{
	return 0;
}
