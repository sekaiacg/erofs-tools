/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Positioned and vectored I/O on top of the Win32 file API.
 */
#include "internal.h"

static int fd_is_disk_file(int fd)
{
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	return h != INVALID_HANDLE_VALUE && GetFileType(h) == FILE_TYPE_DISK;
}

/*
 * Positioned I/O must not touch the file pointer: POSIX callers rely on
 * pwrite()/pread() leaving it alone, and duplicated file descriptors (the
 * buffer manager dups the output file) share it on Windows.  WriteFile() and
 * ReadFile() move it even when an explicit offset is given, so save and
 * restore it around every transfer.
 */
static bool save_file_position(HANDLE h, LARGE_INTEGER *saved)
{
	LARGE_INTEGER zero;

	zero.QuadPart = 0;
	return SetFilePointerEx(h, zero, saved, FILE_CURRENT) != 0;
}

static void restore_file_position(HANDLE h, const LARGE_INTEGER *saved)
{
	SetFilePointerEx(h, *saved, NULL, FILE_BEGIN);
}

static ssize_t do_pio(int fd, void *buf, size_t count, off_t offset, bool wr)
{
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	LARGE_INTEGER saved;
	bool positioned;
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

	positioned = save_file_position(h, &saved);
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
			winsup_set_errno(err);
			if (positioned)
				restore_file_position(h, &saved);
			return total ? total : -1;
		}
		total += done;
		offset += done;
		buf = (char *)buf + done;
		count -= done;
		if (done < chunk)
			break;
	}
	if (positioned)
		restore_file_position(h, &saved);
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
