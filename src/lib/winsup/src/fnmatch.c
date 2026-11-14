/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Minimal fnmatch() implementation, the Microsoft CRT does not ship one.
 */
#include <fnmatch.h>

#include <stdbool.h>
#include <string.h>

static int fold_char(int c, int flags)
{
	if (flags & FNM_CASEFOLD)
		return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
	return c;
}

static int match_bracket(const char **pp, int c, int flags)
{
	const char *p = *pp;
	bool negate = false, matched = false;

	if (*p == '!' || *p == '^') {
		negate = true;
		p++;
	}
	/* a ']' right after the opening bracket is a literal */
	for (;;) {
		int lo, hi;

		if (!*p)
			return -1;
		if (*p == ']' && p != *pp + 1 + negate)
			break;
		if (*p == '\\' && !(flags & FNM_NOESCAPE) && p[1])
			p++;
		lo = fold_char((unsigned char)*p++, flags);
		hi = lo;
		if (*p == '-' && p[1] && p[1] != ']') {
			p++;
			if (*p == '\\' && !(flags & FNM_NOESCAPE) && p[1])
				p++;
			hi = fold_char((unsigned char)*p++, flags);
		}
		c = fold_char(c, flags);
		if (c >= lo && c <= hi)
			matched = true;
	}
	if (*p != ']')
		return -1;
	*pp = p + 1;
	return matched != negate;
}

static int fnm(const char *p, const char *s, int flags, bool at_start)
{
	while (*p) {
		switch (*p) {
		case '?':
			if (!*s)
				return FNM_NOMATCH;
			if ((flags & FNM_PATHNAME) && *s == '/')
				return FNM_NOMATCH;
			if ((flags & FNM_PERIOD) && at_start && *s == '.')
				return FNM_NOMATCH;
			at_start = false;
			p++;
			s++;
			continue;
		case '*':
			while (*p == '*')
				p++;
			if ((flags & FNM_PERIOD) && at_start && *s == '.')
				return FNM_NOMATCH;
			for (;; s++) {
				if (!fnm(p, s, flags, at_start))
					return 0;
				if (!*s)
					break;
				if ((flags & FNM_PATHNAME) && *s == '/')
					break;
			}
			return FNM_NOMATCH;
		case '[': {
			int ret;

			if (!*s)
				return FNM_NOMATCH;
			if ((flags & FNM_PATHNAME) && *s == '/')
				return FNM_NOMATCH;
			if ((flags & FNM_PERIOD) && at_start && *s == '.')
				return FNM_NOMATCH;
			ret = match_bracket(&p, (unsigned char)*s, flags);
			if (ret <= 0)
				return FNM_NOMATCH;
			at_start = false;
			s++;
			continue;
		}
		case '\\':
			if (!(flags & FNM_NOESCAPE) && p[1])
				p++;
			/* fall through */
		default:
			if (!*s)
				return FNM_NOMATCH;
			if (fold_char((unsigned char)*p, flags) !=
			    fold_char((unsigned char)*s, flags))
				return FNM_NOMATCH;
			at_start = *s == '/';
			p++;
			s++;
			continue;
		}
	}
	if ((flags & FNM_LEADING_DIR) && *s == '/')
		return 0;
	return *s ? FNM_NOMATCH : 0;
}

int fnmatch(const char *pattern, const char *string, int flags)
{
	return fnm(pattern, string, flags, true);
}
