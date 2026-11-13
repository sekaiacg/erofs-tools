/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Metadata: lstat() and the file mode/ownership stubs.
 */
#include "internal.h"

static time_t filetime_to_time(FILETIME ft)
{
	unsigned long long t = ((unsigned long long)ft.dwHighDateTime << 32) |
			       ft.dwLowDateTime;

	/* 100ns intervals since 1601 -> seconds since 1970 */
	return (time_t)(t / 10000000ULL) - 11644473600LL;
}

static void fill_symlink_stat(const char *path, struct stat *st, int len)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;

	/* symlink inodes carry the target length as their size */
	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFLNK | 0777;
	st->st_size = len;

	if (GetFileAttributesExA(path, GetFileExInfoStandard, &fad)) {
		st->st_atime = filetime_to_time(fad.ftLastAccessTime);
		st->st_mtime = filetime_to_time(fad.ftLastWriteTime);
		st->st_ctime = filetime_to_time(fad.ftCreationTime);
	}
}

/*
 * Windows resolves every path itself, so lstat() equals stat().  The CRT only
 * knows a read-only attribute, report the usual POSIX permissions instead so
 * that mkfs does not end up with world-writable inodes.  Symlinks are reparse
 * points, but the Cygwin style marker files are used as a fallback.
 */
int lstat(const char *path, struct stat *st)
{
	char target[WINGSUP_SYMLINK_MAX];
	DWORD attrs;
	int ret, len;

	attrs = GetFileAttributesA(path);
	if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_REPARSE_POINT)) {
		len = winsup_read_native_symlink(path, target, sizeof(target));
		if (len > 0) {
			fill_symlink_stat(path, st, len);
			return 0;
		}
	}

	ret = stat(path, st);
	if (ret)
		return ret;

	if (S_ISDIR(st->st_mode)) {
		st->st_mode = S_IFDIR | 0755;
		return 0;
	}
	if (!S_ISREG(st->st_mode))
		return 0;

	st->st_mode = S_IFREG | ((st->st_mode & S_IWRITE) ? 0644 : 0444);

	if (st->st_size > CYGLINK_HDR_LEN && winsup_is_cygwin_symlink(path)) {
		len = winsup_read_cygwin_symlink(path, target, sizeof(target));
		if (len > 0)
			fill_symlink_stat(path, st, len);
	}
	return 0;
}

int link(const char *oldpath, const char *newpath)
{
	if (CreateHardLinkA(newpath, oldpath, NULL))
		return 0;
	winsup_set_errno(GetLastError());
	return -1;
}

int mknod(const char *path, mode_t mode, dev_t dev)
{
	HANDLE h;

	(void)dev;
	if (!S_ISREG(mode)) {
		/* no device nodes, FIFOs or sockets on Windows */
		errno = ENOTSUP;
		return -1;
	}

	h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		winsup_set_errno(GetLastError());
		return -1;
	}
	CloseHandle(h);
	return 0;
}

int chown(const char *path, uid_t owner, gid_t group)
{
	(void)path;
	(void)owner;
	(void)group;
	return 0;
}

int fchown(int fd, uid_t owner, gid_t group)
{
	(void)fd;
	(void)owner;
	(void)group;
	return 0;
}

int lchown(const char *path, uid_t owner, gid_t group)
{
	(void)path;
	(void)owner;
	(void)group;
	return 0;
}

int fchmod(int fd, mode_t mode)
{
	(void)fd;
	(void)mode;
	return 0;
}
