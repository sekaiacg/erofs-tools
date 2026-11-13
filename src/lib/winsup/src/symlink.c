/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Symlink support: native reparse points and the Cygwin format.
 */
#include "internal.h"

/* CreateSymbolicLink() succeeds unelevated on Windows 10 1703 and newer */
#ifndef SYMLINK_FLAG_ALLOW_UNPRIVILEGED
#define SYMLINK_FLAG_ALLOW_UNPRIVILEGED	0x2
#endif

/*
 * Native symlinks are reparse points, so they have to be queried through
 * FILE_FLAG_OPEN_REPARSE_POINT, plain stat() resolves them away.
 */
#define WINGSUP_SYMLINK_MAX	4096

/* mingw only ships this in the WDK header, mirror the documented layout */
typedef struct winsup_reparse_buffer {
	ULONG	ReparseTag;
	USHORT	ReparseDataLength;
	USHORT	Reserved;
	union {
		struct {
			USHORT	SubstituteNameOffset;
			USHORT	SubstituteNameLength;
			USHORT	PrintNameOffset;
			USHORT	PrintNameLength;
			ULONG	Flags;
			WCHAR	PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT	SubstituteNameOffset;
			USHORT	SubstituteNameLength;
			USHORT	PrintNameOffset;
			USHORT	PrintNameLength;
			WCHAR	PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR	DataBuffer[1];
		} GenericReparseBuffer;
	} u;
} winsup_reparse_buffer;

static bool is_drive_absolute(const wchar_t *name, int len)
{
	return len > 7 && name[0] == L'\\' && name[1] == L'?' &&
	       name[2] == L'?' && name[3] == L'\\' && name[5] == L':';
}

/*
 * Convert the target of a reparse point to UTF-8.  Symbolic links keep the
 * original target string in the print name, junctions only have a substitute
 * name prefixed by "\??\".
 */
int winsup_parse_symlink(const void *buffer, size_t len, char *out, size_t outsz)
{
	const winsup_reparse_buffer *rb = buffer;
	const wchar_t *name;
	int nlen;

	if (len < sizeof(rb->ReparseTag) + sizeof(rb->ReparseDataLength) +
		  sizeof(rb->Reserved)) {
		errno = EINVAL;
		return -1;
	}

	if (rb->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
		name = rb->u.SymbolicLinkReparseBuffer.PathBuffer +
		       rb->u.SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
		nlen = rb->u.SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);

		/* a relative link is stored verbatim in the substitute name */
		if (!(rb->u.SymbolicLinkReparseBuffer.Flags & SYMLINK_FLAG_RELATIVE) &&
		    rb->u.SymbolicLinkReparseBuffer.PrintNameLength) {
			name = rb->u.SymbolicLinkReparseBuffer.PathBuffer +
			       rb->u.SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t);
			nlen = rb->u.SymbolicLinkReparseBuffer.PrintNameLength / sizeof(wchar_t);
		}
	} else if (rb->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
		name = rb->u.MountPointReparseBuffer.PathBuffer +
		       rb->u.MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
		nlen = rb->u.MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
	} else {
		errno = EINVAL;
		return -1;
	}
	if (nlen <= 0 || nlen > WINGSUP_SYMLINK_MAX) {
		errno = EINVAL;
		return -1;
	}

	/* drop the "\??\" object manager prefix of absolute targets */
	if (is_drive_absolute(name, nlen)) {
		name += 4;
		nlen -= 4;
	}

	nlen = WideCharToMultiByte(CP_UTF8, 0, name, nlen, out, (int)outsz,
				   NULL, NULL);
	if (nlen <= 0) {
		errno = EINVAL;
		return -1;
	}
	return nlen;
}

int winsup_read_native_symlink(const char *path, char *out, size_t outsz)
{
	void *rb;
	DWORD got = 0;
	HANDLE h;
	int ret;

	h = CreateFileA(path, 0,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING,
			FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
			NULL);
	if (h == INVALID_HANDLE_VALUE) {
		winsup_set_errno(GetLastError());
		return -1;
	}

	rb = malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (!rb) {
		CloseHandle(h);
		errno = ENOMEM;
		return -1;
	}
	if (!DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, rb,
			     MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &got, NULL)) {
		winsup_set_errno(GetLastError());
		CloseHandle(h);
		free(rb);
		return -1;
	}
	CloseHandle(h);

	ret = winsup_parse_symlink(rb, got, out, outsz);
	free(rb);
	return ret;
}

int winsup_read_cygwin_symlink(const char *path, char *out, size_t outsz)
{
	unsigned char *buf;
	wchar_t *w;
	DWORD got = 0;
	HANDLE h;
	size_t max, len;
	int ret;

	h = CreateFileA(path, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return -1;

	buf = malloc(PATH_MAX * 2);
	if (!buf) {
		CloseHandle(h);
		errno = ENOMEM;
		return -1;
	}
	if (!ReadFile(h, buf, PATH_MAX * 2, &got, NULL)) {
		DWORD err = GetLastError();

		CloseHandle(h);
		free(buf);
		winsup_set_errno(err);
		return -1;
	}
	CloseHandle(h);

	if (got < CYGLINK_HDR_LEN || memcmp(buf, CYGLINK_MAGIC, CYGLINK_MAGIC_LEN) ||
	    buf[CYGLINK_MAGIC_LEN] != 0xff || buf[CYGLINK_MAGIC_LEN + 1] != 0xfe) {
		free(buf);
		return -1;
	}

	w = (wchar_t *)(buf + CYGLINK_HDR_LEN);
	max = (got - CYGLINK_HDR_LEN) / sizeof(wchar_t);
	for (len = 0; len < max && w[len]; ++len)
		;
	ret = WideCharToMultiByte(CP_UTF8, 0, w, (int)len, out, (int)outsz,
				  NULL, NULL);
	free(buf);
	return ret > 0 ? ret : -1;
}

bool winsup_is_cygwin_symlink(const char *path)
{
	char target[PATH_MAX];

	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_SYSTEM))
		return false;
	return winsup_read_cygwin_symlink(path, target, sizeof(target)) > 0;
}

/*
 * Write a Cygwin style symlink: "!<symlink>" + UTF-16LE BOM + the target in
 * UTF-16LE, flagged with FILE_ATTRIBUTE_SYSTEM so that Cygwin and our own
 * lstat()/readlink() recognise it.
 */
static int write_cygwin_symlink(const char *target, const char *linkpath)
{
	char u16[WINGSUP_SYMLINK_MAX * 2];
	static const char hdr[] = CYGLINK_MAGIC "\xff\xfe";
	int fd, wlen, len;

	wlen = MultiByteToWideChar(CP_UTF8, 0, target, -1, (wchar_t *)u16,
				   (int)(sizeof(u16) / sizeof(wchar_t)));
	if (wlen <= 0) {
		winsup_set_errno(GetLastError());
		return -1;
	}

	fd = _open(linkpath, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, 0700);
	if (fd < 0)
		return -1;

	/* drop the terminating NUL, keep the terminator written below */
	len = (wlen - 1) * (int)sizeof(wchar_t);
	if (_write(fd, hdr, (unsigned int)(sizeof(hdr) - 1)) < 0 ||
	    _write(fd, u16, (unsigned int)len) < 0 ||
	    _write(fd, "\0\0", 2) < 0) {
		int err = errno;

		_close(fd);
		_unlink(linkpath);
		errno = err;
		return -1;
	}
	_close(fd);

	SetFileAttributesA(linkpath, FILE_ATTRIBUTE_SYSTEM);
	return 0;
}

static int create_native_symlink(const char *target, const char *linkpath)
{
	struct stat st;
	DWORD flags = SYMLINK_FLAG_ALLOW_UNPRIVILEGED;
	DWORD attrs;

	if (!stat(target, &st) && S_ISDIR(st.st_mode))
		flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;

	if (!CreateSymbolicLinkA(linkpath, target, flags) &&
	    !(GetLastError() == ERROR_INVALID_PARAMETER &&
	      CreateSymbolicLinkA(linkpath, target,
				  flags & ~SYMLINK_FLAG_ALLOW_UNPRIVILEGED))) {
		winsup_set_errno(GetLastError());
		return -1;
	}

	/* some emulation layers report success without creating the link */
	attrs = GetFileAttributesA(linkpath);
	if (attrs == INVALID_FILE_ATTRIBUTES ||
	    !(attrs & FILE_ATTRIBUTE_REPARSE_POINT)) {
		errno = ENOTSUP;
		return -1;
	}
	return 0;
}

int symlink(const char *target, const char *linkpath)
{
	if (!create_native_symlink(target, linkpath))
		return 0;

	/* only fall back when the platform refused to create a reparse point */
	if (errno == EEXIST || errno == ENAMETOOLONG || errno == EINVAL)
		return -1;
	return write_cygwin_symlink(target, linkpath);
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
	int ret;

	if (GetFileAttributesA(path) & FILE_ATTRIBUTE_REPARSE_POINT) {
		ret = winsup_read_native_symlink(path, buf, bufsiz);
		if (ret > 0)
			return ret;
	}

	ret = winsup_read_cygwin_symlink(path, buf, bufsiz);
	if (ret < 0) {
		if (!errno)
			errno = EINVAL;
		return -1;
	}
	return ret;
}
