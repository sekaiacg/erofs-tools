/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * GNU extensions the Microsoft CRT does not provide.
 */
#include "internal.h"

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
