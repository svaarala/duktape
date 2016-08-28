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
 *  Error formatting is usually unnecessary.  The error macros provide a
 *  zero argument version (no formatting) and separate macros for small
 *  argument counts.  Variadic macros are not used to avoid portability
 *  issues and avoid the need for stash-based workarounds when they're not
 *  available.  Vararg calls are avoided for non-formatted error calls
 *  because vararg call sites are larger than normal, and there are a lot
 *  of call sites with no formatting.
 *
 *  Note that special formatting provided by debug macros is NOT available.
 *
 *  The _RAW variants allow the caller to specify file and line.  This makes
 *  it easier to write checked calls which want to use the call site of the
 *  checked function, not the error macro call inside the checked function.
 */

#if defined(DUK_USE_VERBOSE_ERRORS)

/* Because there are quite many call sites, pack error code (require at most
 * 8-bit) into a single argument.
 */
#define DUK_ERROR(thr,err,msg) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) DUK_LINE_MACRO; \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error((thr), DUK_FILE_MACRO, (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (msg)); \
	} while (0)
#define DUK_ERROR_RAW(thr,file,line,err,msg) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) (line); \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error((thr), (file), (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (msg)); \
	} while (0)

#define DUK_ERROR_FMT1(thr,err,fmt,arg1) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) DUK_LINE_MACRO; \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), DUK_FILE_MACRO, (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1)); \
	} while (0)
#define DUK_ERROR_RAW_FMT1(thr,file,line,err,fmt,arg1) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) (line); \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), (file), (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1)); \
	} while (0)

#define DUK_ERROR_FMT2(thr,err,fmt,arg1,arg2) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) DUK_LINE_MACRO; \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), DUK_FILE_MACRO, (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1), (arg2)); \
	} while (0)
#define DUK_ERROR_RAW_FMT2(thr,file,line,err,fmt,arg1,arg2) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) (line); \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), (file), (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1), (arg2)); \
	} while (0)

#define DUK_ERROR_FMT3(thr,err,fmt,arg1,arg2,arg3) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) DUK_LINE_MACRO; \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), DUK_FILE_MACRO, (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1), (arg2), (arg3)); \
	} while (0)
#define DUK_ERROR_RAW_FMT3(thr,file,line,err,fmt,arg1,arg2,arg3) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) (line); \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), (file), (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1), (arg2), (arg3)); \
	} while (0)

#define DUK_ERROR_FMT4(thr,err,fmt,arg1,arg2,arg3,arg4) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) DUK_LINE_MACRO; \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), DUK_FILE_MACRO, (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1), (arg2), (arg3), (arg4)); \
	} while (0)
#define DUK_ERROR_RAW_FMT4(thr,file,line,err,fmt,arg1,arg2,arg3,arg4) do { \
		duk_errcode_t duk__err = (err); duk_int_t duk__line = (duk_int_t) (line); \
		DUK_ASSERT(duk__err >= 0 && duk__err <= 0xff); DUK_ASSERT(duk__line >= 0 && duk__line <= 0x00ffffffL); \
		duk_err_handle_error_fmt((thr), (file), (((duk_uint_t) duk__err) << 24) | ((duk_uint_t) duk__line), (fmt), (arg1), (arg2), (arg3), (arg4)); \
	} while (0)

#else  /* DUK_USE_VERBOSE_ERRORS */

#define DUK_ERROR(thr,err,msg)                    duk_err_handle_error((thr), (err))
#define DUK_ERROR_RAW(thr,file,line,err,msg)      duk_err_handle_error((thr), (err))

#define DUK_ERROR_FMT1(thr,err,fmt,arg1) DUK_ERROR((thr),(err),(fmt))
#define DUK_ERROR_RAW_FMT1(thr,file,line,err,fmt,arg1) DUK_ERROR_RAW((thr),(file),(line),(err),(fmt))

#define DUK_ERROR_FMT2(thr,err,fmt,arg1,arg2) DUK_ERROR((thr),(err),(fmt))
#define DUK_ERROR_RAW_FMT2(thr,file,line,err,fmt,arg1,arg2) DUK_ERROR_RAW((thr),(file),(line),(err),(fmt))

#define DUK_ERROR_FMT3(thr,err,fmt,arg1,arg2,arg3) DUK_ERROR((thr),(err),(fmt))
#define DUK_ERROR_RAW_FMT3(thr,file,line,err,fmt,arg1,arg2,arg3) DUK_ERROR_RAW((thr),(file),(line),(err),(fmt))

#define DUK_ERROR_FMT4(thr,err,fmt,arg1,arg2,arg3,arg4) DUK_ERROR((thr),(err),(fmt))
#define DUK_ERROR_RAW_FMT4(thr,file,line,err,fmt,arg1,arg2,arg3,arg4) DUK_ERROR_RAW((thr),(file),(line),(err),(fmt))

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

#if defined(DUK_USE_ASSERTIONS)

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
 *  Error throwing helpers
 *
 *  The goal is to provide verbose and configurable error messages.  Call
 *  sites should be clean in source code and compile to a small footprint.
 *  Small footprint is also useful for performance because small cold paths
 *  reduce code cache pressure.  Adding macros here only makes sense if there
 *  are enough call sites to get concrete benefits.
 */

#if defined(DUK_USE_VERBOSE_ERRORS)
/* Verbose errors with key/value summaries (non-paranoid) or without key/value
 * summaries (paranoid, for some security sensitive environments), the paranoid
 * vs. non-paranoid distinction affects only a few specific errors.
 */
#if defined(DUK_USE_PARANOID_ERRORS)
#define DUK_ERROR_REQUIRE_TYPE_INDEX(thr,index,expectname,lowmemstr) do { \
		duk_err_require_type_index((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (index), (expectname)); \
	} while (0)
#else  /* DUK_USE_PARANOID_ERRORS */
#define DUK_ERROR_REQUIRE_TYPE_INDEX(thr,index,expectname,lowmemstr) do { \
		duk_err_require_type_index((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (index), (expectname)); \
	} while (0)
#endif  /* DUK_USE_PARANOID_ERRORS */

#define DUK_ERROR_UNIMPLEMENTED(thr,msg) do { \
		DUK_ERROR((thr), DUK_ERR_UNIMPLEMENTED_ERROR, (msg)); \
	} while (0)
#define DUK_ERROR_UNIMPLEMENTED_DEFMSG(thr) do { \
		duk_err_unimplemented_defmsg((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO); \
	} while (0)
#define DUK_ERROR_UNSUPPORTED(thr,msg) do { \
		DUK_ERROR((thr), DUK_ERR_UNSUPPORTED_ERROR, (msg)); \
	} while (0)
#if !defined(DUK_USE_BYTECODE_DUMP_SUPPORT)
#define DUK_ERROR_UNSUPPORTED_DEFMSG(thr) do { \
		duk_err_unsupported_defmsg((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO); \
	} while (0)
#endif
#define DUK_ERROR_INTERNAL(thr,msg) do { \
		duk_err_internal((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (msg)); \
	} while (0)
#define DUK_ERROR_INTERNAL_DEFMSG(thr) do { \
		duk_err_internal_defmsg((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO); \
	} while (0)
#define DUK_ERROR_ALLOC(thr,msg) do { \
		duk_err_alloc((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (msg)); \
	} while (0)
#define DUK_ERROR_ALLOC_DEFMSG(thr) do { \
		DUK_ERROR_ALLOC((thr), DUK_STR_ALLOC_FAILED); \
	} while (0)
/* DUK_ERR_ASSERTION_ERROR: no macros needed */
#define DUK_ERROR_API_INDEX(thr,index) do { \
		duk_err_api_index((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (index)); \
	} while (0)
#define DUK_ERROR_API(thr,msg) do { \
		duk_err_api((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (msg)); \
	} while (0)
/* DUK_ERR_UNCAUGHT_ERROR: no macros needed */
/* DUK_ERR_ERROR: no macros needed */
/* DUK_ERR_EVAL: no macros needed */
#define DUK_ERROR_RANGE(thr,msg) do { \
		duk_err_range((thr), DUK_FILE_MACRO, (duk_int_t) DUK_LINE_MACRO, (msg)); \
	} while (0)
/* DUK_ERR_REFERENCE_ERROR: no macros needed */
#define DUK_ERROR_SYNTAX(thr,msg) do { \
		DUK_ERROR((thr), DUK_ERR_SYNTAX_ERROR, (msg)); \
	} while (0)
#define DUK_ERROR_TYPE(thr,msg) do { \
		DUK_ERROR((thr), DUK_ERR_TYPE_ERROR, (msg)); \
	} while (0)
/* DUK_ERR_URI_ERROR: no macros needed */
#else  /* DUK_USE_VERBOSE_ERRORS */
/* Non-verbose errors for low memory targets: no file, line, or message. */

#define DUK_ERROR_REQUIRE_TYPE_INDEX(thr,index,expectname,lowmemstr) do { \
		duk_err_type((thr)); \
	} while (0)

#define DUK_ERROR_UNIMPLEMENTED(thr,msg) do { \
		duk_err_unimplemented((thr)); \
	} while (0)
#define DUK_ERROR_UNIMPLEMENTED_DEFMSG(thr) do { \
		duk_err_unimplemented((thr)); \
	} while (0)
#define DUK_ERROR_UNSUPPORTED(thr,msg) do { \
		duk_err_unsupported((thr)); \
	} while (0)
#define DUK_ERROR_UNSUPPORTED_DEFMSG(thr) do { \
		duk_err_unsupported((thr)); \
	} while (0)
#define DUK_ERROR_INTERNAL(thr,msg) do { \
		duk_err_internal((thr)); \
	} while (0)
#define DUK_ERROR_INTERNAL_DEFMSG(thr) do { \
		duk_err_internal((thr)); \
	} while (0)
#define DUK_ERROR_ALLOC(thr,msg) do { \
		duk_err_alloc((thr)); \
	} while (0)
#define DUK_ERROR_ALLOC_DEFMSG(thr) do { \
		duk_err_alloc((thr)); \
	} while (0)
#define DUK_ERROR_API_INDEX(thr,index) do { \
		duk_err_api((thr)); \
	} while (0)
#define DUK_ERROR_API(thr,msg) do { \
		duk_err_api((thr)); \
	} while (0)
#define DUK_ERROR_RANGE(thr,msg) do { \
		duk_err_range((thr)); \
	} while (0)
#define DUK_ERROR_SYNTAX(thr,msg) do { \
		duk_err_syntax((thr)); \
	} while (0)
#define DUK_ERROR_TYPE(thr,msg) do { \
		duk_err_type((thr)); \
	} while (0)
#endif  /* DUK_USE_VERBOSE_ERRORS */

/*
 *  Prototypes
 */

#if defined(DUK_USE_VERBOSE_ERRORS)
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error(duk_hthread *thr, const char *filename, duk_uint_t line_and_code, const char *msg));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error_fmt(duk_hthread *thr, const char *filename, duk_uint_t line_and_code, const char *fmt, ...));
#else  /* DUK_USE_VERBOSE_ERRORS */
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_handle_error(duk_hthread *thr, duk_errcode_t code));
#endif  /* DUK_USE_VERBOSE_ERRORS */

#if defined(DUK_USE_VERBOSE_ERRORS)
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

#if defined(DUK_USE_VERBOSE_ERRORS)
#if defined(DUK_USE_PARANOID_ERRORS)
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_require_type_index(duk_hthread *thr, const char *filename, duk_int_t linenumber, duk_idx_t index, const char *expect_name));
#else
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_require_type_index(duk_hthread *thr, const char *filename, duk_int_t linenumber, duk_idx_t index, const char *expect_name));
#endif
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_api_index(duk_hthread *thr, const char *filename, duk_int_t linenumber, duk_idx_t index));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_api(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_range(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_unimplemented_defmsg(duk_hthread *thr, const char *filename, duk_int_t linenumber));
#if !defined(DUK_USE_BYTECODE_DUMP_SUPPORT)
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_unsupported_defmsg(duk_hthread *thr, const char *filename, duk_int_t linenumber));
#endif
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_internal_defmsg(duk_hthread *thr, const char *filename, duk_int_t linenumber));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_internal(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_alloc(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message));
#else  /* DUK_VERBOSE_ERRORS */
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_range(duk_hthread *thr));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_syntax(duk_hthread *thr));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_type(duk_hthread *thr));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_api(duk_hthread *thr));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_unimplemented(duk_hthread *thr));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_unsupported(duk_hthread *thr));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_internal(duk_hthread *thr));
DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_alloc(duk_hthread *thr));
#endif /* DUK_VERBOSE_ERRORS */

DUK_NORETURN(DUK_INTERNAL_DECL void duk_err_longjmp(duk_hthread *thr));

DUK_NORETURN(DUK_INTERNAL_DECL void duk_default_fatal_handler(duk_context *ctx, duk_errcode_t code, const char *msg));

#if !defined(DUK_USE_PANIC_HANDLER)
DUK_NORETURN(DUK_INTERNAL_DECL void duk_default_panic_handler(duk_errcode_t code, const char *msg));
#endif

DUK_INTERNAL_DECL void duk_err_setup_heap_ljstate(duk_hthread *thr, duk_small_int_t lj_type);

DUK_INTERNAL_DECL duk_hobject *duk_error_prototype_from_code(duk_hthread *thr, duk_errcode_t err_code);

#endif  /* DUK_ERROR_H_INCLUDED */
