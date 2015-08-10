/*
 *  Error handling macros, assertion macro, error codes.
 *
 *  There are three level of 'errors':
 *
 *    1. Ordinary errors, relative to a thread, cause a longjmp, catchable.
 *    2. Fatal errors, relative to a heap, cause fatal handler to be called.
 *    3. Panic errors, unrelated to a heap and cause a process exit.
 *
 *  Panics are used by the default fatal error handler and by debug code
 *  such as assertions.  By providing a proper fatal error handler, user
 *  code can avoid panics in non-debug builds.
 */

#ifndef DUK_ERROR_H_INCLUDED
#define DUK_ERROR_H_INCLUDED

/*
 *  Error codes: defined in duktape.h
 *
 *  Error codes are used as a shorthand to throw exceptions from inside
 *  the implementation.  The appropriate Ecmascript object is constructed
 *  based on the code.  Ecmascript code throws objects directly.  The error
 *  codes are defined in the public API header because they are also used
 *  by calling code.
 */

/*
 *  Normal error
 *
 *  Normal error is thrown with a longjmp() through the current setjmp()
 *  catchpoint record in the duk_heap.  The 'curr_thread' of the duk_heap
 *  identifies the throwing thread.
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
#define DUK_ERROR(thr,err,...)                    duk_err_handle_error(DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (thr), (err), __VA_ARGS__)
#define DUK_ERROR_RAW(file,line,thr,err,...)      duk_err_handle_error((file), (line), (thr), (err), __VA_ARGS__)

#else  /* DUK_USE_VARIADIC_MACROS */

/* Parameter passing here is not thread safe.  We rely on the __FILE__
 * pointer being a constant which can be passed through a global.
 */

#define DUK_ERROR  \
	(void) (duk_err_file_stash = (const char *) DUK_FILE_MACRO, \
	        duk_err_line_stash = (duk_int_t) DUK_LINE_MACRO, \
	        duk_err_handle_error_stash)  /* arguments follow */
#define DUK_ERROR_RAW                             duk_err_handle_error

#endif  /* DUK_USE_VARIADIC_MACROS */

#else  /* DUK_USE_VERBOSE_ERRORS */

#ifdef DUK_USE_VARIADIC_MACROS

#define DUK_ERROR(thr,err,...)                    duk_err_handle_error((thr), (err))
#define DUK_ERROR_RAW(file,line,thr,err,...)      duk_err_handle_error((thr), (err))

#else  /* DUK_USE_VARIADIC_MACROS */

/* This is sub-optimal because arguments will be passed but ignored, and the strings
 * will go into the object file.  Can't think of how to do this portably and still
 * relatively conveniently.
 */
#define DUK_ERROR                                 duk_err_handle_error_nonverbose1
#define DUK_ERROR_RAW                             duk_err_handle_error_nonverbose2

#endif  /* DUK_USE_VARIADIC_MACROS */

#endif  /* DUK_USE_VERBOSE_ERRORS */

/*
 *  Fatal error
 *
 *  There are no fatal error macros at the moment.  There are so few call
 *  sites that the fatal error handler is called directly.
 */

/*
 *  Panic error
 *
 *  Panic errors are not relative to either a heap or a thread, and cause
 *  DUK_PANIC() macro to be invoked.  Unless a user provides DUK_USE_PANIC_HANDLER,
 *  DUK_PANIC() calls a helper which prints out the error and causes a process
 *  exit.
 *
 *  The user can override the macro to provide custom handling.  A macro is
 *  used to allow the user to have inline panic handling if desired (without
 *  causing a potentially risky function call).
 *
 *  Panics are only used in debug code such as assertions, and by the default
 *  fatal error handler.
 */

#if defined(DUK_USE_PANIC_HANDLER)
/* already defined, good */
#define DUK_PANIC(code,msg)  DUK_USE_PANIC_HANDLER((code),(msg))
#else
#define DUK_PANIC(code,msg)  duk_default_panic_handler((code),(msg))
#endif  /* DUK_USE_PANIC_HANDLER */

/*
 *  Assert macro: failure causes panic.
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

/* Assertion compatible inside a comma expression, evaluates to void.
 * Currently not compatible with DUK_USE_PANIC_HANDLER() which may have
 * a statement block.
 */
#if defined(DUK_USE_PANIC_HANDLER)
/* XXX: resolve macro definition issue or call through a helper function? */
#define DUK_ASSERT_EXPR(x)  ((void) 0)
#else
#define DUK_ASSERT_EXPR(x) \
	((void) ((x) ? 0 : (DUK_PANIC(DUK_ERR_ASSERTION_ERROR, \
				"assertion failed: " #x \
				" (" DUK_FILE_MACRO ":" DUK_MACRO_STRINGIFY(DUK_LINE_MACRO) ")"), 0)))
#endif

#else  /* DUK_USE_ASSERTIONS */

#define DUK_ASSERT(x)  do { /* assertion omitted */ } while (0)

#define DUK_ASSERT_EXPR(x)  ((void) 0)

#endif  /* DUK_USE_ASSERTIONS */

/* this variant is used when an assert would generate a compile warning by
 * being always true (e.g. >= 0 comparison for an unsigned value
 */
#define DUK_ASSERT_DISABLE(x)  do { /* assertion disabled */ } while (0)

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

#define DUK_ASSERT_TOP(ctx,n)  DUK_ASSERT((duk_idx_t) duk_get_top((ctx)) == (duk_idx_t) (n))

#if defined(DUK_USE_ASSERTIONS) && defined(DUK_USE_PACKED_TVAL)
#define DUK_ASSERT_DOUBLE_IS_NORMALIZED(dval)  do { \
		duk_double_union duk__assert_tmp_du; \
		duk__assert_tmp_du.d = (dval); \
		DUK_ASSERT(DUK_DBLUNION_IS_NORMALIZED(&duk__assert_tmp_du)); \
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
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, ...));
#else  /* DUK_USE_VARIADIC_MACROS */
#if !defined(DUK_SINGLE_FILE)
DUK_INTERNAL_DECL const char *duk_err_file_stash;
DUK_INTERNAL_DECL duk_int_t duk_err_line_stash;
#endif  /* !DUK_SINGLE_FILE */
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, ...));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error_stash(duk_hthread *thr, duk_errcode_t code, const char *fmt, ...));
#endif  /* DUK_USE_VARIADIC_MACROS */
#else  /* DUK_USE_VERBOSE_ERRORS */
#ifdef DUK_USE_VARIADIC_MACROS
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error(duk_hthread *thr, duk_errcode_t code));
#else  /* DUK_USE_VARIADIC_MACROS */
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error_nonverbose1(duk_hthread *thr, duk_errcode_t code, const char *fmt, ...));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error_nonverbose2(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, ...));
#endif  /* DUK_USE_VARIADIC_MACROS */
#endif  /* DUK_USE_VERBOSE_ERRORS */

#ifdef DUK_USE_VERBOSE_ERRORS
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_create_and_throw(duk_hthread *thr, duk_errcode_t code, const char *msg, const char *filename, duk_int_t line));
#else
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_create_and_throw(duk_hthread *thr, duk_errcode_t code));
#endif

DUK_NORETURN(DUK_INTERNAL_DECL void duk_error_throw_from_negative_rc(duk_hthread *thr, duk_ret_t rc));

#if defined(DUK_USE_AUGMENT_ERROR_CREATE)
DUK_INTERNAL_DECL void duk_err_augment_error_create(duk_hthread *thr, duk_hthread *thr_callstack, const char *filename, duk_int_t line, duk_bool_t noblame_fileline);
#endif
#if defined(DUK_USE_AUGMENT_ERROR_THROW)
DUK_INTERNAL_DECL void duk_err_augment_error_throw(duk_hthread *thr);
#endif

DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_longjmp(duk_hthread *thr));

DUK_NORETURN(DUK_INTERNAL_DECL void duk_default_fatal_handler(duk_context *ctx, duk_errcode_t code, const char *msg));

#if !defined(DUK_USE_PANIC_HANDLER)
DUK_NORETURN(DUK_INTERNAL_DECL void duk_default_panic_handler(duk_errcode_t code, const char *msg));
#endif

DUK_INTERNAL_DECL void duk_err_setup_heap_ljstate(duk_hthread *thr, duk_small_int_t lj_type);

DUK_INTERNAL_DECL duk_hobject *duk_error_prototype_from_code(duk_hthread *thr, duk_errcode_t err_code);

#endif  /* DUK_ERROR_H_INCLUDED */
