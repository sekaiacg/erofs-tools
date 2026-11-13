/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Path helpers: realpath(), mkdir() and utime().
 */
#include "internal.h"

/* snprintf has no realpath() counterpart in the Microsoft CRT */
char *realpath(const char *path, char *resolved)
{
	char *buf = resolved;
	DWORD len;

	if (!path) {
		errno = EINVAL;
		return NULL;
	}
	if (!buf) {
		buf = malloc(PATH_MAX);
		if (!buf) {
			errno = ENOMEM;
			return NULL;
		}
	}
	len = GetFullPathNameA(path, PATH_MAX, buf, NULL);
	if (!len || len >= PATH_MAX) {
		if (!resolved)
			free(buf);
		errno = ENAMETOOLONG;
		return NULL;
	}
	return buf;
}

int winsup_mkdir(const char *path, mode_t mode)
{
	(void)mode;
	if (CreateDirectoryA(path, NULL))
		return 0;
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		errno = EEXIST;
		return -1;
	}
	winsup_set_errno(GetLastError());
	return -1;
}

int winsup_utime(const char *path, const struct utimbuf *times)
{
	/* SetFileTime() rewrites the DOS attributes on some implementations */
	DWORD attrs = GetFileAttributesA(path);
	HANDLE h;
	BOOL ok;

	h = CreateFileA(path, FILE_WRITE_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		winsup_set_errno(GetLastError());
		return -1;
	}

	if (times) {
		/* seconds since 1970 -> 100ns intervals since 1601 */
		unsigned long long atime = (unsigned long long)times->actime + 11644473600ULL;
		unsigned long long mtime = (unsigned long long)times->modtime + 11644473600ULL;
		FILETIME at, mt;

		at.dwLowDateTime = (DWORD)(atime * 10000000ULL);
		at.dwHighDateTime = (DWORD)((atime * 10000000ULL) >> 32);
		mt.dwLowDateTime = (DWORD)(mtime * 10000000ULL);
		mt.dwHighDateTime = (DWORD)((mtime * 10000000ULL) >> 32);
		ok = SetFileTime(h, NULL, &at, &mt);
	} else {
		ok = SetFileTime(h, NULL, NULL, NULL);
	}

	if (!ok) {
		winsup_set_errno(GetLastError());
		CloseHandle(h);
		return -1;
	}
	CloseHandle(h);

	/*
	 * Keep the Cygwin symlink marker files recognized as symlinks.  Other
	 * bits are recreated by the filesystem, so this is only needed when the
	 * SYSTEM bit was set to begin with.
	 */
	if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_SYSTEM))
		SetFileAttributesA(path, attrs &
			~(FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE |
			  FILE_ATTRIBUTE_REPARSE_POINT));
	return 0;
}
