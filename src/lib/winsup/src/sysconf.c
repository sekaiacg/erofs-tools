/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * The few system configuration queries the code base needs.
 */
#include "internal.h"

int getpagesize(void)
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);
	return (int)si.dwPageSize;
}

long sysconf(int name)
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);
	switch (name) {
	case _SC_PAGESIZE:
		return (long)si.dwPageSize;
	case _SC_NPROCESSORS_ONLN:
		return (long)si.dwNumberOfProcessors;
	default:
		errno = EINVAL;
		return -1;
	}
}

int getrlimit(int resource, struct rlimit *rlim)
{
	if (!rlim) {
		errno = EINVAL;
		return -1;
	}

	if (resource == RLIMIT_NOFILE) {
		/* matches the CRT default stream limit */
		rlim->rlim_cur = 2048;
		rlim->rlim_max = 2048;
		return 0;
	}
	rlim->rlim_cur = RLIM_INFINITY;
	rlim->rlim_max = RLIM_INFINITY;
	return 0;
}

int setrlimit(int resource, const struct rlimit *rlim)
{
	(void)resource;
	(void)rlim;
	return 0;
}

int ioctl(int fd, unsigned long request, ...)
{
	va_list ap;

	va_start(ap, request);
	if (request == TIOCGWINSZ) {
		struct winsize *ws = va_arg(ap, struct winsize *);
		HANDLE h = (HANDLE)_get_osfhandle(fd);
		CONSOLE_SCREEN_BUFFER_INFO info;

		va_end(ap);
		if (h == INVALID_HANDLE_VALUE ||
		    !GetConsoleScreenBufferInfo(h, &info)) {
			errno = ENOTTY;
			return -1;
		}
		ws->ws_col = info.srWindow.Right - info.srWindow.Left + 1;
		ws->ws_row = info.srWindow.Bottom - info.srWindow.Top + 1;
		ws->ws_xpixel = 0;
		ws->ws_ypixel = 0;
		return 0;
	}
	va_end(ap);
	errno = ENOTTY;
	return -1;
}
