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

#define  DUK_DEBUG_BUFSIZE  65536
static char buf[DUK_DEBUG_BUFSIZE];

static const char *get_level_string(int level) {
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
#define  TERM_REVERSE  "\x1b[7m"
#define  TERM_BRIGHT   "\x1b[1m"
#define  TERM_RESET    "\x1b[0m"
#define  TERM_BLUE     "\x1b[34m"
#define  TERM_RED      "\x1b[31m"

static const char *get_term_1(int level) {
	return (const char *) TERM_RED;
}

static const char *get_term_2(int level) {
	switch (level) {
	case DUK_LEVEL_DEBUG:
		return (const char *) (TERM_RESET TERM_BRIGHT);
	case DUK_LEVEL_DDEBUG:
		return (const char *) (TERM_RESET);
	case DUK_LEVEL_DDDEBUG:
		return (const char *) (TERM_RESET TERM_BLUE);
	}
	return (const char *) TERM_RESET;
}

static const char *get_term_3(int level) {
	return (const char *) TERM_RESET;
}

#else

static const char *get_term_1(int level) {
	return (const char *) "";
}

static const char *get_term_2(int level) {
	return (const char *) "";
}

static const char *get_term_3(int level) {
	return (const char *) "";
}

#endif  /* DUK_USE_DPRINT_COLORS */

#ifdef DUK_USE_VARIADIC_MACROS

void duk_debug_log(int level, const char *file, int line, const char *func, char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	memset((void *) buf, 0, (size_t) DUK_DEBUG_BUFSIZE);
	duk_debug_vsnprintf(buf, DUK_DEBUG_BUFSIZE - 1, fmt, ap);

#ifdef DUK_USE_DPRINT_RDTSC
	fprintf(stderr, "%s[%s] <%llu> %s:%d (%s):%s %s%s\n",
		get_term_1(level),
		get_level_string(level),
		duk_rdtsc(),
		file,
		line,
		func,
		get_term_2(level),
		buf,
		get_term_3(level));
#else
	fprintf(stderr, "%s[%s] %s:%d (%s):%s %s%s\n",
		get_term_1(level),
		get_level_string(level),
		file,
		line,
		func,
		get_term_2(level),
		buf,
		get_term_3(level));
#endif
	fflush(stderr);

	va_end(ap);
}

#else  /* DUK_USE_VARIADIC_MACROS */

char duk_debug_file_stash[DUK_DEBUG_STASH_SIZE];
char duk_debug_line_stash[DUK_DEBUG_STASH_SIZE];
char duk_debug_func_stash[DUK_DEBUG_STASH_SIZE];
int duk_debug_level_stash;

void duk_debug_log(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	memset((void *) buf, 0, (size_t) DUK_DEBUG_BUFSIZE);
	duk_debug_vsnprintf(buf, DUK_DEBUG_BUFSIZE - 1, fmt, ap);

#ifdef DUK_USE_DPRINT_RDTSC
	fprintf(stderr, "%s[%s] <%llu> %s:%s (%s):%s %s%s\n",
		get_term_1(level),
		get_level_string(duk_debug_level_stash),
		duk_rdtsc(),
	        duk_debug_file_stash,
	        duk_debug_line_stash,
	        duk_debug_func_stash,
		get_term_2(level),
		buf,
		get_term_3(level));
#else
	fprintf(stderr, "%s[%s] %s:%s (%s):%s %s%s\n",
		get_term_1(level),
		get_level_string(duk_debug_level_stash),
	        duk_debug_file_stash,
	        duk_debug_line_stash,
	        duk_debug_func_stash,
		get_term_2(level),
		buf,
		get_term_3(level));
#endif
	fflush(stderr);

	va_end(ap);
}

#endif  /* DUK_USE_VARIADIC_MACROS */

#else  /* DUK_USE_DEBUG */

/*
 *  Debugging disabled
 */

#endif  /* DUK_USE_DEBUG */

