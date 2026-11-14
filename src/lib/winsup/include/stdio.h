/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: GNU stdio extensions used by this project.
 */
#ifndef __WINGSUP_STDIO_H
#define __WINGSUP_STDIO_H

#include_next <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t getline(char **lineptr, size_t *n, FILE *stream);
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);

int asprintf(char **strp, const char *fmt, ...);
int vasprintf(char **strp, const char *fmt, va_list ap);

/* the CRT does not implement the _unlocked variants */
#define fgets_unlocked(buf, size, fp)		fgets((buf), (size), (fp))
#define fread_unlocked(buf, size, n, fp)	fread((buf), (size), (n), (fp))
#define fwrite_unlocked(buf, size, n, fp)	fwrite((buf), (size), (n), (fp))
#define getc_unlocked(fp)			getc(fp)
#define putc_unlocked(c, fp)			putc((c), (fp))

#ifdef __cplusplus
}
#endif

#endif /* __WINGSUP_STDIO_H */
