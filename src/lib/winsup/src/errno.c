/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Translates Win32 error codes to errno.
 */
#include "internal.h"

void winsup_set_errno(DWORD err)
{
	switch (err) {
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_INVALID_DRIVE:
		errno = ENOENT;
		break;
	case ERROR_ACCESS_DENIED:
	case ERROR_SHARING_VIOLATION:
	case ERROR_LOCK_VIOLATION:
		errno = EACCES;
		break;
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS:
		errno = EEXIST;
		break;
	case ERROR_INVALID_HANDLE:
		errno = EBADF;
		break;
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:
		errno = ENOMEM;
		break;
	case ERROR_DISK_FULL:
	case ERROR_HANDLE_DISK_FULL:
		errno = ENOSPC;
		break;
	case ERROR_WRITE_PROTECT:
		errno = EROFS;
		break;
	case ERROR_DIRECTORY:
		errno = ENOTDIR;
		break;
	case ERROR_DIR_NOT_EMPTY:
		errno = ENOTEMPTY;
		break;
	case ERROR_FILENAME_EXCED_RANGE:
		errno = ENAMETOOLONG;
		break;
	case ERROR_INVALID_PARAMETER:
		errno = EINVAL;
		break;
	case ERROR_BROKEN_PIPE:
		errno = EPIPE;
		break;
	case ERROR_NOT_SUPPORTED:
	case ERROR_INVALID_FUNCTION:
		errno = ENOTSUP;
		break;
	default:
		errno = EIO;
		break;
	}
}
