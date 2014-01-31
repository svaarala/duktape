/*
 *  Error handling macros, assertion macro, error codes.
 */

#ifndef DUK_ERROR_H_INCLUDED
#define DUK_ERROR_H_INCLUDED

/*
 *  Error codes (defined in duktape.h)
 *
 *  Error codes are used as a shorthand to throw exceptions from inside
 *  the implementation.  The appropriate Ecmascript object is constructed
 *  based on the code.  Ecmascript code throws objects directly.
 *
 *  The error codes are now defined in the public API header because they
 *  are also used by calling code.
 */

/* for function return codes */
#define DUK_ERR_OK                   0     /* call successful */
#define DUK_ERR_FAIL                 1     /* call failed */

/*
 *  Normal error is thrown with a longjmp() through the current setjmp()
 *  catchpoint record in the duk_heap.  The 'curr_thread' of the duk_heap
 *  identifies the throwing thread.
 *
 *  Panic is thrown without a heap/thread context and cannot be caught.
 *  All bets are off, and the default implementation exits the process.
 *
 *  FIXME: panic should map to the fatal error handler.
 *
 *  Error formatting is not always necessary but there are no separate calls
 *  (to minimize code size).  Error object creation will consume a considerable
 *  amount of time, compared to which formatting is probably trivial.  Note
 *  that special formatting (provided by DUK_DEBUG macros) is NOT available.
 *
 *  The _RAW variants allow the caller to specify file and line.  This makes
 *  it easier to write checked calls which want to use the call site of the
 *  checked function, not the error macro call inside the checked function.
 *
 *  We prefer the standard variadic macros; if they are not available, we
 *  fall back to awkward hacks.
 */

#ifdef DUK_USE_VERBOSE_ERRORS

#ifdef DUK_USE_VARIADIC_MACROS

/* __VA_ARGS__ has comma issues for empty lists, so we mandate at least 1 argument for '...' (format string) */
#define DUK_ERROR(thr,err,...)                    duk_err_handle_error(DUK_FILE_MACRO, (int) DUK_LINE_MACRO, (thr), (err), __VA_ARGS__)
#define DUK_ERROR_RAW(file,line,thr,err,...)      duk_err_handle_error((file), (line), (thr), (err), __VA_ARGS__)
#define DUK_PANIC(err,...)                        duk_err_handle_panic(DUK_FILE_MACRO, DUK_LINE_MACRO, (err), __VA_ARGS__)
#define DUK_PANIC_RAW(file,line,err,...)          duk_err_handle_panic((file), (line), (err), __VA_ARGS__)

#else  /* DUK_USE_VARIADIC_MACROS */

/* Parameter passing here is not thread safe.  We rely on the __FILE__
 * pointer being a constant which can be passed through a global.
 */

#define DUK_ERROR  \
	duk_err_file_stash = (const char *) DUK_FILE_MACRO, \
	duk_err_line_stash = (int) DUK_LINE_MACRO, \
	(void) duk_err_handle_error_stash  /* arguments follow */
#define DUK_ERROR_RAW                             duk_err_handle_error
#define DUK_PANIC  \
	duk_err_file_stash = (const char *) DUK_FILE_MACRO, \
	duk_err_line_stash = (int) DUK_LINE_MACRO, \
	(void) duk_err_handle_panic_stash  /* arguments follow */
#define DUK_PANIC_RAW                             duk_err_handle_panic

#endif  /* DUK_USE_VARIADIC_MACROS */

#else  /* DUK_USE_VERBOSE_ERRORS */

#ifdef DUK_USE_VARIADIC_MACROS

#define DUK_ERROR(thr,err,...)                    duk_err_handle_error((thr), (err))
#define DUK_ERROR_RAW(file,line,thr,err,...)      duk_err_handle_error((thr), (err))
#define DUK_PANIC(err,...)                        duk_err_handle_panic((err))
#define DUK_PANIC_RAW(err,...)                    duk_err_handle_panic((err))

#else  /* DUK_USE_VARIADIC_MACROS */

/* This is sub-optimal because arguments will be passed but ignored, and the strings
 * will go into the object file.  Can't think of how to do this portably and still
 * relatively conveniently.
 */
#define DUK_ERROR                                 duk_err_handle_error_nonverbose1
#define DUK_ERROR_RAW                             duk_err_handle_error_nonverbose2
#define DUK_PANIC                                 duk_err_handle_panic_nonverbose1
#define DUK_PANIC_RAW                             duk_err_handle_panic_nonverbose2

#endif  /* DUK_USE_VARIADIC_MACROS */

#endif  /* DUK_USE_VERBOSE_ERRORS */

/*
 *  Assert macro: failure cause DUK_PANIC().
 */

#ifdef DUK_USE_ASSERTIONS

/* the message should be a compile time constant without formatting (less risk);
 * we don't care about assertion text size because they're not used in production
 * builds.
 */
#define DUK_ASSERT(x)  do { \
	if (!(x)) { \
		DUK_PANIC(DUK_ERR_ASSERTION_ERROR, \
			"assertion failed: " #x \
			" (" DUK_FILE_MACRO ":" DUK_MACRO_STRINGIFY(DUK_LINE_MACRO) ")"); \
	} \
	} while (0)

#else  /* DUK_USE_ASSERTIONS */

#define DUK_ASSERT(x)  do { /* assertion omitted */ } while(0)

#endif  /* DUK_USE_ASSERTIONS */

/* this variant is used when an assert would generate a compile warning by
 * being always true (e.g. >= 0 comparison for an unsigned value
 */
#define DUK_ASSERT_DISABLE(x)  do { /* assertion disabled */ } while(0)

/*
 *  Final panic handler macro (unless defined already)
 */

/* FIXME: Change this is so that if DUK_USER_PANIC_HANDLER defined, map
 * DUK_PANIC_HANDLER to it? Cleaner than allowing user to define directly.
 * In any case, panics should map do fatal error handler in the public API.
 */

#if defined(DUK_PANIC_HANDLER)
/* already defined, good */
#else
#if defined(DUK_USE_PANIC_ABORT)
#define DUK_PANIC_EXIT()  abort()
#elif defined(DUK_USE_PANIC_EXIT)
#define DUK_PANIC_EXIT()  exit(-1)
#elif defined(DUK_USE_PANIC_SEGFAULT)
#define DUK_PANIC_EXIT()  do { \
		/* exit() afterwards to satisfy "noreturn" */ \
		DUK_CAUSE_SEGFAULT(); \
		exit(-1); \
	} while (0)
#else
#error no DUK_USE_PANIC_xxx macro defined
#endif

#ifdef DUK_USE_FILE_IO
#define DUK_PANIC_PRINTMSG(code,msg)  do { \
		fprintf(stderr, "PANIC %d: %s\n", code, msg ? msg : "null"); \
		fflush(stderr); \
	} while (0)
#else
#define DUK_PANIC_PRINTMSG(code,msg)
#endif

#ifdef DUK_USE_GCC_PRAGMAS
#define DUK_PANIC_HANDLER(code,msg)  do { \
		/* GCC pragmas to suppress: warning: the address of 'xxx' will always evaluate as 'true' [-Waddress]' */ \
		_Pragma("GCC diagnostic push"); \
		_Pragma("GCC diagnostic ignored \"-Waddress\""); \
		DUK_PANIC_PRINTMSG((code),(msg)); \
		DUK_PANIC_EXIT(); \
		DUK_UNREACHABLE(); \
		_Pragma("GCC diagnostic pop"); \
	} while (0)
#else
#define DUK_PANIC_HANDLER(code,msg)  do { \
		/* No pragmas to suppress warning, causes unclean build */ \
		DUK_PANIC_PRINTMSG((code),(msg)); \
		DUK_PANIC_EXIT(); \
		DUK_UNREACHABLE(); \
	} while (0)
#endif  /* DUK_USE_GCC_PRAGMAS */
#endif  /* DUK_PANIC_HANDLER */

/*
 *  Assertion helpers
 */

#if defined(DUK_USE_ASSERTIONS) && defined(DUK_USE_REFERENCE_COUNTING)
#define DUK_ASSERT_REFCOUNT_NONZERO_HEAPHDR(h)  do { \
		DUK_ASSERT((h) == NULL || DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) (h)) > 0); \
	} while (0)
#define DUK_ASSERT_REFCOUNT_NONZERO_TVAL(tv)  do { \
		if ((tv) != NULL && DUK_TVAL_IS_HEAP_ALLOCATED((tv))) { \
			DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(DUK_TVAL_GET_HEAPHDR((tv))) > 0); \
		} \
	} while (0)
#else
#define DUK_ASSERT_REFCOUNT_NONZERO_HEAPHDR(h)  /* no refcount check */
#define DUK_ASSERT_REFCOUNT_NONZERO_TVAL(tv)    /* no refcount check */
#endif

/* FIXME: fix typing to match duk_get_top() eventual return value type */
#define DUK_ASSERT_TOP(ctx,n)  DUK_ASSERT((duk_int_t) duk_get_top((ctx)) == (duk_int_t) (n))

#if defined(DUK_USE_ASSERTIONS) && defined(DUK_USE_PACKED_TVAL)
#define DUK_ASSERT_DOUBLE_IS_NORMALIZED(dval)  do { \
		duk_double_union assert_tmp_du; \
		assert_tmp_du.d = (dval); \
		DUK_ASSERT(DUK_DBLUNION_IS_NORMALIZED(&assert_tmp_du)); \
	} while (0)
#else
#define DUK_ASSERT_DOUBLE_IS_NORMALIZED(dval)  /* nop */
#endif

/*
 *  Helper for valstack space
 *
 *  Caller of DUK_ASSERT_VALSTACK_SPACE() estimates the number of free stack entries
 *  required for its own use, and any child calls which are not (a) Duktape API calls
 *  or (b) Duktape calls which involve extending the valstack (e.g. getter call).
 */

#define DUK_VALSTACK_ASSERT_EXTRA  5  /* this is added to checks to allow for Duktape
                                        * API calls in addition to function's own use
                                        */
#if defined(DUK_USE_ASSERTIONS)
#define DUK_ASSERT_VALSTACK_SPACE(thr,n)   do { \
		DUK_ASSERT((thr) != NULL); \
		DUK_ASSERT((thr)->valstack_end - (thr)->valstack_top >= (n) + DUK_VALSTACK_ASSERT_EXTRA); \
	} while (0)
#else
#define DUK_ASSERT_VALSTACK_SPACE(thr,n)   /* no valstack space check */
#endif

/*
 *  Prototypes
 */

#ifdef DUK_USE_VERBOSE_ERRORS
#ifdef DUK_USE_VARIADIC_MACROS
DUK_NORETURN(void duk_err_handle_error(const char *filename, int line, duk_hthread *thr, int code, const char *fmt, ...));
DUK_NORETURN(void duk_err_handle_panic(const char *filename, int line, int code, const char *fmt, ...));
#else  /* DUK_USE_VARIADIC_MACROS */
extern const char *duk_err_file_stash;
extern int duk_err_line_stash;
DUK_NORETURN(void duk_err_handle_error(const char *filename, int line, duk_hthread *thr, int code, const char *fmt, ...));
DUK_NORETURN(void duk_err_handle_error_stash(duk_hthread *thr, int code, const char *fmt, ...));
DUK_NORETURN(void duk_err_handle_panic(const char *filename, int line, int code, const char *fmt, ...));
DUK_NORETURN(void duk_err_handle_panic_stash(int code, const char *fmt, ...));
#endif  /* DUK_USE_VARIADIC_MACROS */
#else  /* DUK_USE_VERBOSE_ERRORS */
#ifdef DUK_USE_VARIADIC_MACROS
DUK_NORETURN(void duk_err_handle_error(duk_hthread *thr, int code));
DUK_NORETURN(void duk_err_handle_panic(int code));
#else  /* DUK_USE_VARIADIC_MACROS */
DUK_NORETURN(void duk_err_handle_error_nonverbose1(duk_hthread *thr, int code, const char *fmt, ...));
DUK_NORETURN(void duk_err_handle_error_nonverbose2(const char *filename, int line, duk_hthread *thr, int code, const char *fmt, ...));
DUK_NORETURN(void duk_err_handle_panic_nonverbose1(int code, const char *fmt, ...));
DUK_NORETURN(void duk_err_handle_panic_nonverbose2(const char *filename, int line, int code, const char *fmt, ...));
#endif  /* DUK_USE_VARIADIC_MACROS */
#endif  /* DUK_USE_VERBOSE_ERRORS */

#ifdef DUK_USE_VERBOSE_ERRORS
DUK_NORETURN(void duk_err_create_and_throw(duk_hthread *thr, duk_uint32_t code, const char *msg, const char *filename, int line));
#else
DUK_NORETURN(void duk_err_create_and_throw(duk_hthread *thr, duk_uint32_t code));
#endif

DUK_NORETURN(void duk_error_throw_from_negative_rc(duk_hthread *thr, int rc));

#ifdef DUK_USE_AUGMENT_ERRORS
void duk_err_augment_error(duk_hthread *thr, duk_hthread *thr_callstack, int err_index, const char *filename, int line, int noblame_fileline);
#endif

DUK_NORETURN(void duk_err_longjmp(duk_hthread *thr));

DUK_NORETURN(void duk_default_fatal_handler(duk_context *ctx, int code));

void duk_err_setup_heap_ljstate(duk_hthread *thr, int lj_type);

duk_hobject *duk_error_prototype_from_code(duk_hthread *thr, int err_code);

#endif  /* DUK_ERROR_H_INCLUDED */

