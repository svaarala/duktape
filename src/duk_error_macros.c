/*
 *  Error, fatal, and panic handling.
 */

#include "duk_internal.h"

#define DUK__ERRFMT_BUFSIZE  256  /* size for formatting buffers */

#if defined(DUK_USE_VERBOSE_ERRORS)

#if defined(DUK_USE_VARIADIC_MACROS)
DUK_INTERNAL void duk_err_handle_error(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, ...) {
	va_list ap;
	char msg[DUK__ERRFMT_BUFSIZE];
	va_start(ap, fmt);
	(void) DUK_VSNPRINTF(msg, sizeof(msg), fmt, ap);
	msg[sizeof(msg) - 1] = (char) 0;
	duk_err_create_and_throw(thr, code, msg, filename, line);
	va_end(ap);  /* dead code, but ensures portability (see Linux man page notes) */
}
#else  /* DUK_USE_VARIADIC_MACROS */
DUK_INTERNAL const char *duk_err_file_stash = NULL;
DUK_INTERNAL duk_int_t duk_err_line_stash = 0;

DUK_NORETURN(DUK_LOCAL_DECL void duk__handle_error(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, va_list ap));

DUK_LOCAL void duk__handle_error(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, va_list ap) {
	char msg[DUK__ERRFMT_BUFSIZE];
	(void) DUK_VSNPRINTF(msg, sizeof(msg), fmt, ap);
	msg[sizeof(msg) - 1] = (char) 0;
	duk_err_create_and_throw(thr, code, msg, filename, line);
}

DUK_INTERNAL void duk_err_handle_error(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	duk__handle_error(filename, line, thr, code, fmt, ap);
	va_end(ap);  /* dead code */
}

DUK_INTERNAL void duk_err_handle_error_stash(duk_hthread *thr, duk_errcode_t code, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	duk__handle_error(duk_err_file_stash, duk_err_line_stash, thr, code, fmt, ap);
	va_end(ap);  /* dead code */
}
#endif  /* DUK_USE_VARIADIC_MACROS */

#else  /* DUK_USE_VERBOSE_ERRORS */

#if defined(DUK_USE_VARIADIC_MACROS)
DUK_INTERNAL void duk_err_handle_error(duk_hthread *thr, duk_errcode_t code) {
	duk_err_create_and_throw(thr, code);
}

#else  /* DUK_USE_VARIADIC_MACROS */
DUK_INTERNAL void duk_err_handle_error_nonverbose1(duk_hthread *thr, duk_errcode_t code, const char *fmt, ...) {
	DUK_UNREF(fmt);
	duk_err_create_and_throw(thr, code);
}

DUK_INTERNAL void duk_err_handle_error_nonverbose2(const char *filename, duk_int_t line, duk_hthread *thr, duk_errcode_t code, const char *fmt, ...) {
	DUK_UNREF(filename);
	DUK_UNREF(line);
	DUK_UNREF(fmt);
	duk_err_create_and_throw(thr, code);
}
#endif  /* DUK_USE_VARIADIC_MACROS */

#endif  /* DUK_USE_VERBOSE_ERRORS */

/*
 *  Error throwing helpers
 */

#if defined(DUK_USE_VERBOSE_ERRORS)
#if defined(DUK_USE_PARANOID_ERRORS)
DUK_INTERNAL void duk_err_require_type_index(const char *filename, duk_int_t linenumber, duk_hthread *thr, duk_idx_t index, const char *expect_name) {
	DUK_ERROR_RAW(filename, linenumber, thr, DUK_ERR_TYPE_ERROR, "%s required, found %s (stack index %ld)",
	              expect_name, duk_get_type_name((duk_context *) thr, index), (long) index);
}
#else
DUK_INTERNAL void duk_err_require_type_index(const char *filename, duk_int_t linenumber, duk_hthread *thr, duk_idx_t index, const char *expect_name) {
	DUK_ERROR_RAW(filename, linenumber, thr, DUK_ERR_TYPE_ERROR, "%s required, found %s (stack index %ld)",
	              expect_name, duk_push_string_readable((duk_context *) thr, index), (long) index);
}
#endif
DUK_INTERNAL void duk_err_api_index(const char *filename, duk_int_t linenumber, duk_hthread *thr, duk_idx_t index) {
	DUK_ERROR_RAW(filename, linenumber, thr, DUK_ERR_API_ERROR, "invalid stack index %ld", (long) (index));
}
DUK_INTERNAL void duk_err_api(const char *filename, duk_int_t linenumber, duk_hthread *thr, const char *message) {
	DUK_ERROR_RAW(filename, linenumber, thr, DUK_ERR_API_ERROR, message);
}
#else
DUK_INTERNAL void duk_err_require_type_index(const char *filename, duk_int_t linenumber, duk_hthread *thr, const char *message) {
	DUK_UNREF(filename); DUK_UNREF(linenumber); DUK_UNREF(message);
	DUK_ERROR_RAW(filename, linenumber, thr, DUK_ERR_TYPE_ERROR, message);
}
DUK_INTERNAL void duk_err_api_index(const char *filename, duk_int_t linenumber, duk_hthread *thr) {
	DUK_UNREF(filename); DUK_UNREF(linenumber);
	DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_INDEX);
}
DUK_INTERNAL void duk_err_api(const char *filename, duk_int_t linenumber, duk_hthread *thr, const char *message) {
	DUK_UNREF(filename); DUK_UNREF(linenumber); DUK_UNREF(message);
	DUK_ERROR_RAW(filename, linenumber, thr, DUK_ERR_API_ERROR, message);
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
