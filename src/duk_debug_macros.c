/*
 *  Debugging macro calls.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

/*
 *  Debugging enabled
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* for one-char summaries (usable for e.g. valstack) */
char duk_debug_summary_buf[DUK_DEBUG_SUMMARY_BUF_SIZE];
int duk_debug_summary_idx;

#define DUK__DEBUG_BUFSIZE  DUK_USE_DEBUG_BUFSIZE
static char duk__debug_buf[DUK__DEBUG_BUFSIZE];

static const char *duk__get_level_string(int level) {
	switch (level) {
	case DUK_LEVEL_DEBUG:
		return "D";
	case DUK_LEVEL_DDEBUG:
		return "DD";
	case DUK_LEVEL_DDDEBUG:
		return "DDD";
	}
	return "???";
}

#ifdef DUK_USE_DPRINT_COLORS

/* http://en.wikipedia.org/wiki/ANSI_escape_code */
#define DUK__TERM_REVERSE  "\x1b[7m"
#define DUK__TERM_BRIGHT   "\x1b[1m"
#define DUK__TERM_RESET    "\x1b[0m"
#define DUK__TERM_BLUE     "\x1b[34m"
#define DUK__TERM_RED      "\x1b[31m"

static const char *duk__get_term_1(int level) {
	DUK_UNREF(level);
	return (const char *) DUK__TERM_RED;
}

static const char *duk__get_term_2(int level) {
	switch (level) {
	case DUK_LEVEL_DEBUG:
		return (const char *) (DUK__TERM_RESET DUK__TERM_BRIGHT);
	case DUK_LEVEL_DDEBUG:
		return (const char *) (DUK__TERM_RESET);
	case DUK_LEVEL_DDDEBUG:
		return (const char *) (DUK__TERM_RESET DUK__TERM_BLUE);
	}
	return (const char *) DUK__TERM_RESET;
}

static const char *duk__get_term_3(int level) {
	DUK_UNREF(level);
	return (const char *) DUK__TERM_RESET;
}

#else

static const char *duk__get_term_1(int level) {
	return (const char *) "";
}

static const char *duk__get_term_2(int level) {
	return (const char *) "";
}

static const char *duk__get_term_3(int level) {
	return (const char *) "";
}

#endif  /* DUK_USE_DPRINT_COLORS */

#ifdef DUK_USE_VARIADIC_MACROS

void duk_debug_log(int level, const char *file, int line, const char *func, char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	DUK_MEMZERO((void *) duk__debug_buf, (size_t) DUK__DEBUG_BUFSIZE);
	duk_debug_vsnprintf(duk__debug_buf, DUK__DEBUG_BUFSIZE - 1, fmt, ap);

#ifdef DUK_USE_DPRINT_RDTSC
	DUK_FPRINTF(DUK_STDERR, "%s[%s] <%llu> %s:%d (%s):%s %s%s\n",
	            duk__get_term_1(level),
	            duk__get_level_string(level),
	            duk_rdtsc(),
	            file,
	            line,
	            func,
	            duk__get_term_2(level),
	            duk__debug_buf,
	            duk__get_term_3(level));
#else
	DUK_FPRINTF(DUK_STDERR, "%s[%s] %s:%d (%s):%s %s%s\n",
	            duk__get_term_1(level),
	            duk__get_level_string(level),
	            file,
	            line,
	            func,
	            duk__get_term_2(level),
	            duk__debug_buf,
	            duk__get_term_3(level));
#endif
	DUK_FFLUSH(DUK_STDERR);

	va_end(ap);
}

#else  /* DUK_USE_VARIADIC_MACROS */

char duk_debug_file_stash[DUK_DEBUG_STASH_SIZE];
char duk_debug_line_stash[DUK_DEBUG_STASH_SIZE];
char duk_debug_func_stash[DUK_DEBUG_STASH_SIZE];
int duk_debug_level_stash;

void duk_debug_log(char *fmt, ...) {
	va_list ap;
	int level = duk_debug_level_stash;

	va_start(ap, fmt);

	DUK_MEMZERO((void *) duk__debug_buf, (size_t) DUK__DEBUG_BUFSIZE);
	duk_debug_vsnprintf(duk__debug_buf, DUK__DEBUG_BUFSIZE - 1, fmt, ap);

#ifdef DUK_USE_DPRINT_RDTSC
	DUK_FPRINTF(DUK_STDERR, "%s[%s] <%llu> %s:%s (%s):%s %s%s\n",
	            duk__get_term_1(level),
	            duk__get_level_string(duk_debug_level_stash),
	            duk_rdtsc(),
	            duk_debug_file_stash,
	            duk_debug_line_stash,
	            duk_debug_func_stash,
	            duk__get_term_2(level),
	            duk__debug_buf,
	            duk__get_term_3(level));
#else
	DUK_FPRINTF(DUK_STDERR, "%s[%s] %s:%s (%s):%s %s%s\n",
	            duk__get_term_1(level),
	            duk__get_level_string(duk_debug_level_stash),
	            duk_debug_file_stash,
	            duk_debug_line_stash,
	            duk_debug_func_stash,
	            duk__get_term_2(level),
	            duk__debug_buf,
	            duk__get_term_3(level));
#endif
	DUK_FFLUSH(DUK_STDERR);

	va_end(ap);
}

#endif  /* DUK_USE_VARIADIC_MACROS */

#else  /* DUK_USE_DEBUG */

/*
 *  Debugging disabled
 */

#endif  /* DUK_USE_DEBUG */

