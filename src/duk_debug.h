/*
 *  Debugging macros, DUK_DPRINT() and its variants in particular.
 *
 *  DUK_DPRINT() allows formatted debug prints, and supports standard
 *  and Duktape specific formatters.  See duk_debug_vsnprintf.c for details.
 */

#ifndef __DUK_DEBUG_H
#define __DUK_DEBUG_H

#include "duk_features.h"  /* rely on DUK_USE_VARIADIC_MACROS */
#include "duk_bittypes.h"
#include "duk_forwdecl.h"

#ifdef DUK_USE_DEBUG

/*
 *  Exposed debug macros: debugging enabled
 */

#include <stdarg.h>

#define  DUK_LEVEL_DEBUG    1
#define  DUK_LEVEL_DDEBUG   2
#define  DUK_LEVEL_DDDEBUG  3

#ifdef DUK_USE_VARIADIC_MACROS

/* Note: combining __FILE__, __LINE__, and __func__ into fmt would be
 * possible compile time, but waste some space with shared function names.
 */
#define  _DUK_DEBUG_LOG(lev,...)  duk_debug_log((lev), __FILE__, (int) __LINE__, __func__, __VA_ARGS__);

#define  DUK_DPRINT(...)          _DUK_DEBUG_LOG(DUK_LEVEL_DEBUG, __VA_ARGS__)

#ifdef DUK_USE_DDEBUG
#define  DUK_DDPRINT(...)         _DUK_DEBUG_LOG(DUK_LEVEL_DDEBUG, __VA_ARGS__)
#else
#define  DUK_DDPRINT(...)
#endif

#ifdef DUK_USE_DDDEBUG
#define  DUK_DDDPRINT(...)        _DUK_DEBUG_LOG(DUK_LEVEL_DDDEBUG, __VA_ARGS__)
#else
#define  DUK_DDDPRINT(...)
#endif

#else  /* DUK_USE_VARIADIC_MACROS */

#define  _DUK_DEBUG_STASH(lev)    \
	(void) memset((void *) duk_debug_file_stash, 0, (size_t) DUK_DEBUG_STASH_SIZE), \
	(void) memset((void *) duk_debug_line_stash, 0, (size_t) DUK_DEBUG_STASH_SIZE), \
	(void) memset((void *) duk_debug_func_stash, 0, (size_t) DUK_DEBUG_STASH_SIZE), \
	(void) snprintf(duk_debug_file_stash, DUK_DEBUG_STASH_SIZE - 1, "%s", __FILE__), \
	(void) snprintf(duk_debug_line_stash, DUK_DEBUG_STASH_SIZE - 1, "%d", (int) __LINE__), \
	(void) snprintf(duk_debug_func_stash, DUK_DEBUG_STASH_SIZE - 1, "%s", __func__), \
	(void) (duk_debug_level_stash = (lev))

#ifdef DUK_USE_DEBUG
#define  DUK_DPRINT  _DUK_DEBUG_STASH(DUK_LEVEL_DEBUG), (void) duk_debug_log  /* args go here in parens */
#else
#define  DUK_DPRINT  0 && 
#endif

#ifdef DUK_USE_DDEBUG
#define  DUK_DDPRINT  _DUK_DEBUG_STASH(DUK_LEVEL_DDEBUG), (void) duk_debug_log  /* args go here in parens */
#else
#define  DUK_DDPRINT  0 && 
#endif

#ifdef DUK_USE_DDDEBUG
#define  DUK_DDDPRINT  _DUK_DEBUG_STASH(DUK_LEVEL_DDDEBUG), (void) duk_debug_log  /* args go here in parens */
#else
#define  DUK_DDDPRINT  0 && 
#endif

#endif  /* DUK_USE_VARIADIC_MACROS */

/* object dumpers */

#define  DUK_DEBUG_DUMP_HEAP(x)               duk_debug_dump_heap((x))
#define  DUK_DEBUG_DUMP_HSTRING(x)            /*FIXME*/
#define  DUK_DEBUG_DUMP_HOBJECT(x)            duk_debug_dump_hobject((x))
#define  DUK_DEBUG_DUMP_HCOMPILEDFUNCTION(x)  /*FIXME*/
#define  DUK_DEBUG_DUMP_HNATIVEFUNCTION(x)    /*FIXME*/
#define  DUK_DEBUG_DUMP_HTHREAD(thr)          duk_debug_dump_hobject((duk_hobject *) (thr))
#define  DUK_DEBUG_DUMP_CALLSTACK(thr)        duk_debug_dump_callstack((thr))
#define  DUK_DEBUG_DUMP_ACTIVATION(thr,act)   duk_debug_dump_activation((thr),(act))

/* summary macros */

#define  DUK_DEBUG_SUMMARY_INIT()  do { \
		memset(duk_debug_summary_buf, 0, sizeof(duk_debug_summary_buf)); \
		duk_debug_summary_idx = 0; \
	} while (0)

#define  DUK_DEBUG_SUMMARY_CHAR(ch)  do { \
		duk_debug_summary_buf[duk_debug_summary_idx++] = (ch); \
		if (duk_debug_summary_idx >= sizeof(duk_debug_summary_buf) - 1) { \
			duk_debug_summary_buf[duk_debug_summary_idx++] = (char) 0; \
			DUK_DPRINT("    %s", duk_debug_summary_buf); \
			DUK_DEBUG_SUMMARY_INIT(); \
		} \
	} while (0)

#define  DUK_DEBUG_SUMMARY_FINISH()  do { \
		if (duk_debug_summary_idx > 0) { \
			duk_debug_summary_buf[duk_debug_summary_idx++] = (char) 0; \
			DUK_DPRINT("    %s", duk_debug_summary_buf); \
			DUK_DEBUG_SUMMARY_INIT(); \
		} \
	} while (0)

#else  /* DUK_USE_DEBUG */

/*
 *  Exposed debug macros: debugging disabled
 */

#ifdef DUK_USE_VARIADIC_MACROS

#define  DUK_DPRINT(...)
#define  DUK_DDPRINT(...)
#define  DUK_DDDPRINT(...)

#else  /* DUK_USE_VARIADIC_MACROS */

#define  DUK_DPRINT    0 &&   /* args go here in parens */
#define  DUK_DDPRINT   0 && 
#define  DUK_DDDPRINT  0 && 

#endif  /* DUK_USE_VARIADIC_MACROS */

#define  DUK_DEBUG_DUMP_HEAP(x)
#define  DUK_DEBUG_DUMP_HSTRING(x)
#define  DUK_DEBUG_DUMP_HOBJECT(x)
#define  DUK_DEBUG_DUMP_HCOMPILEDFUNCTION(x)
#define  DUK_DEBUG_DUMP_HNATIVEFUNCTION(x)
#define  DUK_DEBUG_DUMP_HTHREAD(x)

#define  DUK_DEBUG_SUMMARY_INIT()
#define  DUK_DEBUG_SUMMARY_CHAR(ch)
#define  DUK_DEBUG_SUMMARY_FINISH()

#endif  /* DUK_DEBUG */

/*
 *  Structs
 */

#ifdef DUK_USE_DEBUG
struct duk_fixedbuffer {
	duk_u8 *buffer;
	duk_u32 length;
	duk_u32 offset;
	int truncated;
};
#endif

/*
 *  Prototypes
 */

#ifdef DUK_USE_DEBUG
int duk_debug_vsnprintf(char *str, size_t size, const char *format, va_list ap);
int duk_debug_snprintf(char *str, size_t size, const char *format, ...);

#ifdef DUK_USE_VARIADIC_MACROS
void duk_debug_log(int level, const char *file, int line, const char *func, char *fmt, ...);
#else
/* parameter passing, not thread safe */
#define  DUK_DEBUG_STASH_SIZE  256
extern char duk_debug_file_stash[DUK_DEBUG_STASH_SIZE];
extern char duk_debug_line_stash[DUK_DEBUG_STASH_SIZE];
extern char duk_debug_func_stash[DUK_DEBUG_STASH_SIZE];
extern int duk_debug_level_stash;
extern void duk_debug_log(char *fmt, ...);
#endif

void duk_fb_put_bytes(duk_fixedbuffer *fb, duk_u8 *buffer, duk_u32 length);
void duk_fb_put_byte(duk_fixedbuffer *fb, duk_u8 x);
void duk_fb_put_cstring(duk_fixedbuffer *fb, char *x);
void duk_fb_sprintf(duk_fixedbuffer *fb, const char *fmt, ...);
int duk_fb_is_full(duk_fixedbuffer *fb);

void duk_debug_dump_heap(duk_heap *heap);
void duk_debug_heap_graphviz(duk_heap *heap);
void duk_debug_dump_hobject(duk_hobject *obj);
void duk_debug_dump_hthread(duk_hthread *thr);
void duk_debug_dump_callstack(duk_hthread *thr);
void duk_debug_dump_activation(duk_hthread *thr, duk_activation *act);

#define  DUK_DEBUG_SUMMARY_BUF_SIZE  76
extern char duk_debug_summary_buf[DUK_DEBUG_SUMMARY_BUF_SIZE];
extern int duk_debug_summary_idx;

#endif  /* DUK_USE_DEBUG */

#endif  /* __DUK_DEBUG_H */

