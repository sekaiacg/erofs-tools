/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * POSIX compatibility shims for the Microsoft C runtime.
 */
#define _CRT_RAND_S
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <utime.h>

#define SYMLINK_FLAG_ALLOW_UNPRIVILEGED	0x2

/*
 * The CRT translates CRLF sequences in text mode, which would corrupt image
 * data and extracted files, so make every open binary by default.
 */
static void winsup_set_binary_mode(void) __attribute__((constructor));
static void winsup_set_binary_mode(void)
{
	_set_fmode(_O_BINARY);
}

static void set_errno_from_win32(DWORD err)
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

static int fd_is_disk_file(int fd)
{
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	return h != INVALID_HANDLE_VALUE && GetFileType(h) == FILE_TYPE_DISK;
}

/*
 * Positioned I/O must not touch the CRT file pointer, otherwise concurrent
 * readers/writers would corrupt each other's offsets.
 */
static ssize_t do_pio(int fd, void *buf, size_t count, off_t offset, bool wr)
{
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	OVERLAPPED ov;
	ssize_t total = 0;

	if (fd < 0 || h == INVALID_HANDLE_VALUE) {
		errno = EBADF;
		return -1;
	}

	if (!fd_is_disk_file(fd)) {
		if (_lseeki64(fd, offset, SEEK_SET) < 0)
			return -1;
		return wr ? _write(fd, buf, (unsigned int)count)
			  : _read(fd, buf, (unsigned int)count);
	}

	memset(&ov, 0, sizeof(ov));
	while (count) {
		DWORD chunk = count > (1U << 30) ? (1U << 30) : (DWORD)count;
		DWORD done = 0;
		BOOL ok;

		ov.Offset = (DWORD)((unsigned long long)offset & 0xffffffff);
		ov.OffsetHigh = (DWORD)((unsigned long long)offset >> 32);
		ok = wr ? WriteFile(h, buf, chunk, &done, &ov)
			: ReadFile(h, buf, chunk, &done, &ov);
		if (!ok) {
			DWORD err = GetLastError();

			if (!wr && err == ERROR_HANDLE_EOF)
				break;
			set_errno_from_win32(err);
			return total ? total : -1;
		}
		total += done;
		offset += done;
		buf = (char *)buf + done;
		count -= done;
		if (done < chunk)
			break;
	}
	return total;
}

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
	return do_pio(fd, buf, count, offset, false);
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	return do_pio(fd, (void *)buf, count, offset, true);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t total = 0;
	int i;

	for (i = 0; i < iovcnt; ++i) {
		ssize_t ret = _read(fd, iov[i].iov_base, (unsigned int)iov[i].iov_len);

		if (ret < 0)
			return total ? total : -1;
		total += ret;
		if ((size_t)ret != iov[i].iov_len)
			break;
	}
	return total;
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t total = 0;
	int i;

	for (i = 0; i < iovcnt; ++i) {
		ssize_t ret = _write(fd, iov[i].iov_base, (unsigned int)iov[i].iov_len);

		if (ret < 0)
			return total ? total : -1;
		total += ret;
		if ((size_t)ret != iov[i].iov_len)
			break;
	}
	return total;
}

ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	ssize_t total = 0;
	int i;

	for (i = 0; i < iovcnt; ++i) {
		ssize_t ret = pread(fd, iov[i].iov_base, iov[i].iov_len, offset);

		if (ret < 0)
			return total ? total : -1;
		total += ret;
		offset += ret;
		if ((size_t)ret != iov[i].iov_len)
			break;
	}
	return total;
}

ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	ssize_t total = 0;
	int i;

	for (i = 0; i < iovcnt; ++i) {
		ssize_t ret = pwrite(fd, iov[i].iov_base, iov[i].iov_len, offset);

		if (ret < 0)
			return total ? total : -1;
		total += ret;
		offset += ret;
		if ((size_t)ret != iov[i].iov_len)
			break;
	}
	return total;
}

int fsync(int fd)
{
	return _commit(fd);
}

int fdatasync(int fd)
{
	return _commit(fd);
}

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


/*
 * Cygwin compatible symlinks: a plain file carrying FILE_ATTRIBUTE_SYSTEM
 * whose content is the target string prefixed by "!<symlink>" and a
 * UTF-16LE BOM.  Decoding it lets the tools round-trip on Windows.
 */
#define CYGLINK_MAGIC		"!<symlink>"
#define CYGLINK_MAGIC_LEN	10
#define CYGLINK_HDR_LEN		(CYGLINK_MAGIC_LEN + 2)

static int read_cygwin_symlink(const char *path, char *out, size_t outsz)
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
		set_errno_from_win32(err);
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

static bool is_cygwin_symlink(const char *path)
{
	char target[PATH_MAX];

	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_SYSTEM))
		return false;
	return read_cygwin_symlink(path, target, sizeof(target)) > 0;
}

/*
 * Windows resolves every path itself, so lstat() equals stat().  The CRT only
 * knows a read-only attribute, report the usual POSIX permissions instead so
 * that mkfs does not end up with world-writable inodes.
 */
int lstat(const char *path, struct stat *st)
{
	int ret = stat(path, st);

	if (ret)
		return ret;

	if (S_ISDIR(st->st_mode)) {
		st->st_mode = S_IFDIR | 0755;
		return 0;
	}
	if (!S_ISREG(st->st_mode))
		return 0;

	st->st_mode = S_IFREG | ((st->st_mode & S_IWRITE) ? 0644 : 0444);

	if (st->st_size > CYGLINK_HDR_LEN && is_cygwin_symlink(path)) {
		char target[PATH_MAX];
		int len = read_cygwin_symlink(path, target, sizeof(target));

		if (len > 0) {
			/* symlink inodes carry the target length as their size */
			st->st_mode = S_IFLNK | 0777;
			st->st_size = len;
		}
	}
	return 0;
}

int symlink(const char *target, const char *linkpath)
{
	struct stat st;
	DWORD flags = SYMLINK_FLAG_ALLOW_UNPRIVILEGED;

	if (!stat(target, &st) && S_ISDIR(st.st_mode))
		flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;

	if (CreateSymbolicLinkA(linkpath, target, flags))
		return 0;
	/* the unprivileged flag is rejected by pre-Windows 10 builds */
	if (GetLastError() == ERROR_INVALID_PARAMETER &&
	    CreateSymbolicLinkA(linkpath, target,
				flags & ~SYMLINK_FLAG_ALLOW_UNPRIVILEGED))
		return 0;
	set_errno_from_win32(GetLastError());
	return -1;
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
	int ret = read_cygwin_symlink(path, buf, bufsiz);

	if (ret < 0) {
		if (!errno)
			errno = EINVAL;
		return -1;
	}
	return ret;
}

int link(const char *oldpath, const char *newpath)
{
	if (CreateHardLinkA(newpath, oldpath, NULL))
		return 0;
	set_errno_from_win32(GetLastError());
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
		set_errno_from_win32(GetLastError());
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

int winsup_mkdir(const char *path, mode_t mode)
{
	(void)mode;
	if (CreateDirectoryA(path, NULL))
		return 0;
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		errno = EEXIST;
		return -1;
	}
	set_errno_from_win32(GetLastError());
	return -1;
}

ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
	size_t len = 0;
	int c;
	char *line = *lineptr;

	if (!lineptr || !n || !stream) {
		errno = EINVAL;
		return -1;
	}
	if (!line || !*n) {
		*n = 256;
		line = malloc(*n);
		if (!line) {
			errno = ENOMEM;
			return -1;
		}
		*lineptr = line;
	}

	while ((c = fgetc(stream)) != EOF) {
		if (len + 1 >= *n) {
			size_t nsize = *n * 2;
			char *tmp = realloc(line, nsize);

			if (!tmp) {
				errno = ENOMEM;
				return -1;
			}
			line = tmp;
			*n = nsize;
			*lineptr = line;
		}
		line[len++] = (char)c;
		if (c == delim)
			break;
	}
	if (!len && c == EOF)
		return -1;
	line[len] = '\0';
	return (ssize_t)len;
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
	return getdelim(lineptr, n, '\n', stream);
}

int vasprintf(char **strp, const char *fmt, va_list ap)
{
	va_list copy;
	int len;
	char *buf;

	va_copy(copy, ap);
	len = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);
	if (len < 0)
		return -1;

	buf = malloc((size_t)len + 1);
	if (!buf)
		return -1;
	if (vsnprintf(buf, (size_t)len + 1, fmt, ap) != len) {
		free(buf);
		return -1;
	}
	*strp = buf;
	return len;
}

int asprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vasprintf(strp, fmt, ap);
	va_end(ap);
	return ret;
}

/*
 * Read-only file mappings are emulated with a private heap copy.  All users
 * in this project only read from the mapped range.
 */
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *buf;

	(void)addr;
	(void)prot;
	if (!length) {
		errno = EINVAL;
		return MAP_FAILED;
	}
	if (!(flags & MAP_ANONYMOUS) && fd < 0) {
		errno = EBADF;
		return MAP_FAILED;
	}
	if (prot & PROT_WRITE) {
		errno = ENOTSUP;
		return MAP_FAILED;
	}

	buf = malloc(length);
	if (!buf) {
		errno = ENOMEM;
		return MAP_FAILED;
	}
	if (flags & MAP_ANONYMOUS) {
		memset(buf, 0, length);
		return buf;
	}
	if (pread(fd, buf, length, offset) < 0) {
		free(buf);
		return MAP_FAILED;
	}
	return buf;
}

int munmap(void *addr, size_t length)
{
	(void)length;
	if (!addr || addr == MAP_FAILED) {
		errno = EINVAL;
		return -1;
	}
	free(addr);
	return 0;
}

int msync(void *addr, size_t length, int flags)
{
	(void)addr;
	(void)length;
	(void)flags;
	return 0;
}

int mprotect(void *addr, size_t length, int prot)
{
	(void)addr;
	(void)length;
	(void)prot;
	return 0;
}

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

ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
	(void)path;
	(void)name;
	(void)value;
	(void)size;
	errno = ENOTSUP;
	return -1;
}

ssize_t lgetxattr(const char *path, const char *name, void *value, size_t size)
{
	return getxattr(path, name, value, size);
}

ssize_t fgetxattr(int fd, const char *name, void *value, size_t size)
{
	(void)fd;
	(void)name;
	(void)value;
	(void)size;
	errno = ENOTSUP;
	return -1;
}

ssize_t setxattr(const char *path, const char *name, const void *value,
		 size_t size, int flags)
{
	(void)path;
	(void)name;
	(void)value;
	(void)size;
	(void)flags;
	errno = ENOTSUP;
	return -1;
}

ssize_t lsetxattr(const char *path, const char *name, const void *value,
		  size_t size, int flags)
{
	return setxattr(path, name, value, size, flags);
}

ssize_t fsetxattr(int fd, const char *name, const void *value, size_t size,
		  int flags)
{
	(void)fd;
	(void)name;
	(void)value;
	(void)size;
	(void)flags;
	errno = ENOTSUP;
	return -1;
}

ssize_t listxattr(const char *path, char *list, size_t size)
{
	(void)path;
	(void)list;
	(void)size;
	errno = ENOTSUP;
	return -1;
}

ssize_t llistxattr(const char *path, char *list, size_t size)
{
	return listxattr(path, list, size);
}

ssize_t flistxattr(int fd, char *list, size_t size)
{
	(void)fd;
	(void)list;
	(void)size;
	errno = ENOTSUP;
	return -1;
}

int removexattr(const char *path, const char *name)
{
	(void)path;
	(void)name;
	errno = ENOTSUP;
	return -1;
}

int lremovexattr(const char *path, const char *name)
{
	return removexattr(path, name);
}

int fremovexattr(int fd, const char *name)
{
	(void)fd;
	(void)name;
	errno = ENOTSUP;
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
		set_errno_from_win32(GetLastError());
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
		set_errno_from_win32(GetLastError());
		CloseHandle(h);
		return -1;
	}
	CloseHandle(h);

	/* keep the Cygwin symlink marker files recognized as symlinks */
	if (attrs != INVALID_FILE_ATTRIBUTES)
		SetFileAttributesA(path, attrs &
			~(FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE |
			  FILE_ATTRIBUTE_REPARSE_POINT));
	return 0;
}
