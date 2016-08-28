/*
 *  Error, fatal, and panic handling.
 */

#include "duk_internal.h"

#define DUK__ERRFMT_BUFSIZE  256  /* size for formatting buffers */

#if defined(DUK_USE_VERBOSE_ERRORS)

DUK_INTERNAL void duk_err_handle_error_fmt(duk_hthread *thr, const char *filename, duk_uint_t line_and_code, const char *fmt, ...) {
	va_list ap;
	char msg[DUK__ERRFMT_BUFSIZE];
	va_start(ap, fmt);
	(void) DUK_VSNPRINTF(msg, sizeof(msg), fmt, ap);
	msg[sizeof(msg) - 1] = (char) 0;
	duk_err_create_and_throw(thr, (duk_errcode_t) (line_and_code >> 24), msg, filename, (duk_int_t) (line_and_code & 0x00ffffffL));
	va_end(ap);  /* dead code, but ensures portability (see Linux man page notes) */
}

DUK_INTERNAL void duk_err_handle_error(duk_hthread *thr, const char *filename, duk_uint_t line_and_code, const char *msg) {
	duk_err_create_and_throw(thr, (duk_errcode_t) (line_and_code >> 24), msg, filename, (duk_int_t) (line_and_code & 0x00ffffffL));
}

#else  /* DUK_USE_VERBOSE_ERRORS */

DUK_INTERNAL void duk_err_handle_error(duk_hthread *thr, duk_errcode_t code) {
	duk_err_create_and_throw(thr, code);
}

#endif  /* DUK_USE_VERBOSE_ERRORS */

/*
 *  Error throwing helpers
 */

#if defined(DUK_USE_VERBOSE_ERRORS)
#if defined(DUK_USE_PARANOID_ERRORS)
DUK_INTERNAL void duk_err_require_type_index(duk_hthread *thr, const char *filename, duk_int_t linenumber, duk_idx_t index, const char *expect_name) {
	DUK_ERROR_RAW_FMT3(thr, filename, linenumber, DUK_ERR_TYPE_ERROR, "%s required, found %s (stack index %ld)",
	                   expect_name, duk_get_type_name((duk_context *) thr, index), (long) index);
}
#else
DUK_INTERNAL void duk_err_require_type_index(duk_hthread *thr, const char *filename, duk_int_t linenumber, duk_idx_t index, const char *expect_name) {
	DUK_ERROR_RAW_FMT3(thr, filename, linenumber, DUK_ERR_TYPE_ERROR, "%s required, found %s (stack index %ld)",
	                   expect_name, duk_push_string_readable((duk_context *) thr, index), (long) index);
}
#endif
DUK_INTERNAL void duk_err_range(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message) {
	DUK_ERROR_RAW(thr, filename, linenumber, DUK_ERR_RANGE_ERROR, message);
}
DUK_INTERNAL void duk_err_api_index(duk_hthread *thr, const char *filename, duk_int_t linenumber, duk_idx_t index) {
	DUK_ERROR_RAW_FMT1(thr, filename, linenumber, DUK_ERR_API_ERROR, "invalid stack index %ld", (long) (index));
}
DUK_INTERNAL void duk_err_api(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message) {
	DUK_ERROR_RAW(thr, filename, linenumber, DUK_ERR_API_ERROR, message);
}
DUK_INTERNAL void duk_err_unimplemented_defmsg(duk_hthread *thr, const char *filename, duk_int_t linenumber) {
	DUK_ERROR_RAW(thr, filename, linenumber, DUK_ERR_UNIMPLEMENTED_ERROR, DUK_STR_UNIMPLEMENTED);
}
#if !defined(DUK_USE_BYTECODE_DUMP_SUPPORT)
DUK_INTERNAL void duk_err_unsupported_defmsg(duk_hthread *thr, const char *filename, duk_int_t linenumber) {
	DUK_ERROR_RAW(thr, filename, linenumber, DUK_ERR_UNSUPPORTED_ERROR, DUK_STR_UNSUPPORTED);
}
#endif
DUK_INTERNAL void duk_err_internal_defmsg(duk_hthread *thr, const char *filename, duk_int_t linenumber) {
	DUK_ERROR_RAW(thr, filename, linenumber, DUK_ERR_INTERNAL_ERROR, DUK_STR_INTERNAL_ERROR);
}
DUK_INTERNAL void duk_err_internal(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message) {
	DUK_ERROR_RAW(thr, filename, linenumber, DUK_ERR_INTERNAL_ERROR, message);
}
DUK_INTERNAL void duk_err_alloc(duk_hthread *thr, const char *filename, duk_int_t linenumber, const char *message) {
	DUK_ERROR_RAW(thr, filename, linenumber, DUK_ERR_ALLOC_ERROR, message);
}
#else
/* The file/line arguments are NULL and 0, they're ignored by DUK_ERROR_RAW()
 * when non-verbose errors are used.
 */
DUK_INTERNAL void duk_err_type(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, 0, DUK_ERR_TYPE_ERROR, NULL);
}
DUK_INTERNAL void duk_err_api(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, 0, DUK_ERR_API_ERROR, NULL);
}
DUK_INTERNAL void duk_err_range(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, 0, DUK_ERR_RANGE_ERROR, NULL);
}
DUK_INTERNAL void duk_err_syntax(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, 0, DUK_ERR_SYNTAX_ERROR, NULL);
}
DUK_INTERNAL void duk_err_unimplemented(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, 0, DUK_ERR_UNIMPLEMENTED_ERROR, NULL);
}
DUK_INTERNAL void duk_err_unsupported(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, 0, DUK_ERR_UNSUPPORTED_ERROR, NULL);
}
DUK_INTERNAL void duk_err_internal(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, 0, DUK_ERR_INTERNAL_ERROR, NULL);
}
DUK_INTERNAL void duk_err_alloc(duk_hthread *thr) {
	DUK_ERROR_RAW(thr, NULL, thr, DUK_ERR_ALLOC_ERROR, NULL);
}
#endif

/*
 *  Default fatal error handler
 */

DUK_INTERNAL void duk_default_fatal_handler(duk_context *ctx, duk_errcode_t code, const char *msg) {
	DUK_UNREF(ctx);
#if defined(DUK_USE_FILE_IO)
	DUK_FPRINTF(DUK_STDERR, "FATAL %ld: %s\n", (long) code, (const char *) (msg ? msg : "null"));
	DUK_FFLUSH(DUK_STDERR);
#else
	/* omit print */
#endif
	DUK_D(DUK_DPRINT("default fatal handler called, code %ld -> calling DUK_PANIC()", (long) code));
	DUK_PANIC(code, msg);
	DUK_UNREACHABLE();
}

/*
 *  Default panic handler
 */

#if !defined(DUK_USE_PANIC_HANDLER)
DUK_INTERNAL void duk_default_panic_handler(duk_errcode_t code, const char *msg) {
#if defined(DUK_USE_FILE_IO)
	DUK_FPRINTF(DUK_STDERR, "PANIC %ld: %s ("
#if defined(DUK_USE_PANIC_ABORT)
	            "calling abort"
#elif defined(DUK_USE_PANIC_EXIT)
	            "calling exit"
#elif defined(DUK_USE_PANIC_SEGFAULT)
	            "segfaulting on purpose"
#else
#error no DUK_USE_PANIC_xxx macro defined
#endif
	            ")\n", (long) code, (const char *) (msg ? msg : "null"));
	DUK_FFLUSH(DUK_STDERR);
#else
	/* omit print */
	DUK_UNREF(code);
	DUK_UNREF(msg);
#endif

#if defined(DUK_USE_PANIC_ABORT)
	DUK_ABORT();
#elif defined(DUK_USE_PANIC_EXIT)
	DUK_EXIT(-1);
#elif defined(DUK_USE_PANIC_SEGFAULT)
	/* exit() afterwards to satisfy "noreturn" */
	DUK_CAUSE_SEGFAULT();  /* SCANBUILD: "Dereference of null pointer", normal */
	DUK_EXIT(-1);
#else
#error no DUK_USE_PANIC_xxx macro defined
#endif

	DUK_UNREACHABLE();
}
#endif  /* !DUK_USE_PANIC_HANDLER */

#undef DUK__ERRFMT_BUFSIZE
