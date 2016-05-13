/* Built-in logging framework removed in Duktape 2.x. */
/*---
{
    "skip": true
}
---*/

/*===
*** test_1 (duk_safe_call)
TIMESTAMP TRC C: trace: 123
TIMESTAMP DBG C: debug: 123
TIMESTAMP INF C: info: 123
TIMESTAMP WRN C: warn: 123
TIMESTAMP ERR C: error: 123
TIMESTAMP FTL C: fatal: 123
TIMESTAMP TRC C: clamped trace: 123
TIMESTAMP TRC C: clamped trace: 123
TIMESTAMP FTL C: clamped fatal: 123
TIMESTAMP FTL C: clamped fatal: 123
TIMESTAMP INF C: long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long and formatted: 123
final top: 0
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
TIMESTAMP FTL C: fatal error: 123 quux
==> rc=0, result='undefined'
===*/

#define  CHARS_100 \
	"long long long long long long long long long long " \
	"long long long long long long long long long long "

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* Force log level to output all logs. */
	duk_eval_string(ctx, "Duktape.Logger.clog.l = 0;");
	duk_pop(ctx);

	/* Replace raw() function so that we log to stdout, and replace timestamp
	 * with a fixed string to be able to build an expect string.
	 */
	duk_eval_string(ctx, "Duktape.Logger.prototype.raw = function (msg) {\n"
	                     "    msg = String(msg);  // arg is a buffer\n"
	                     "    msg = msg.replace(/\\S+/, 'TIMESTAMP')\n"
	                     "    print(msg);\n"
	                     "}");
	duk_pop(ctx);

	duk_log(ctx, DUK_LOG_TRACE, "trace: %d", 123);
	duk_log(ctx, DUK_LOG_DEBUG, "debug: %d", 123);
	duk_log(ctx, DUK_LOG_INFO, "info: %d", 123);
	duk_log(ctx, DUK_LOG_WARN, "warn: %d", 123);
	duk_log(ctx, DUK_LOG_ERROR, "error: %d", 123);
	duk_log(ctx, DUK_LOG_FATAL, "fatal: %d", 123);

	/* Invalid negative log level is clamped to 0 == DUK_LOG_TRACE */
	duk_log(ctx, -1, "clamped trace: %d", 123);
	duk_log(ctx, -123, "clamped trace: %d", 123);

	/* Too large positive log level is clamped to 5 == DUK_LOG_FATAL */
	duk_log(ctx, 6, "clamped fatal: %d", 123);
	duk_log(ctx, 123, "clamped fatal: %d", 123);

	/* Very long test log messages are also (now) supported */
	duk_log(ctx, DUK_LOG_INFO,
	        /* 1000 chars prefix */
	        CHARS_100 CHARS_100 CHARS_100 CHARS_100 CHARS_100
	        CHARS_100 CHARS_100 CHARS_100 CHARS_100 CHARS_100
	        "and formatted: %d", 123);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Vararg */
static void my_log_write(duk_context *ctx, duk_int_t level, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	duk_log_va(ctx, level, fmt, ap);
	va_end(ap);
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	/* Rely on the log "censoring" set up in test_1(). */

	my_log_write(ctx, DUK_LOG_FATAL, "fatal error: %d %s", 123, "quux");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
