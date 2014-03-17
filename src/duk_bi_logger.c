/*
 *  Logging support
 */

#include "duk_internal.h"

/* 3-letter log level strings */
static const duk_uint8_t duk__log_level_strings[] = {
	(duk_uint8_t) DUK_ASC_UC_T, (duk_uint8_t) DUK_ASC_UC_R, (duk_uint8_t) DUK_ASC_UC_C,
	(duk_uint8_t) DUK_ASC_UC_D, (duk_uint8_t) DUK_ASC_UC_B, (duk_uint8_t) DUK_ASC_UC_G,
	(duk_uint8_t) DUK_ASC_UC_I, (duk_uint8_t) DUK_ASC_UC_N, (duk_uint8_t) DUK_ASC_UC_F,
	(duk_uint8_t) DUK_ASC_UC_W, (duk_uint8_t) DUK_ASC_UC_R, (duk_uint8_t) DUK_ASC_UC_N,
	(duk_uint8_t) DUK_ASC_UC_E, (duk_uint8_t) DUK_ASC_UC_R, (duk_uint8_t) DUK_ASC_UC_R,
	(duk_uint8_t) DUK_ASC_UC_F, (duk_uint8_t) DUK_ASC_UC_T, (duk_uint8_t) DUK_ASC_UC_L
};

/* Constructor */
duk_ret_t duk_bi_logger_constructor(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_int_t nargs;  /* FIXME: type */

	/* Calling as a non-constructor is not meaningful. */
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}

	nargs = duk_get_top(ctx);
	duk_set_top(ctx, 1);

	duk_push_this(ctx);

	/* [ name this ] */

	if (nargs == 0) {
		/* Automatic defaulting of logger name from caller. */
		if (thr->callstack_top >= 2) {
			duk_activation *act_caller = thr->callstack + thr->callstack_top - 2;
			if (act_caller->func) {
				/* FIXME: stripped filename? */
				duk_push_hobject(ctx, act_caller->func);
				duk_get_prop_stridx(ctx, -1, DUK_STRIDX_FILE_NAME);
				duk_replace(ctx, 0);
			}
		}
	}
	/* the stack is unbalanced here on purpose; we only rely on the
	 * initial two values: [ name this ].
	 */

	if (duk_is_string(ctx, 0)) {
		duk_dup(ctx, 0);
		duk_put_prop_stridx(ctx, 1, DUK_STRIDX_LC_N);
	} else {
		/* don't set 'n' at all, inherited value is used as name */
	}

	duk_compact(ctx, 1);

	return 0;  /* keep default instance */
}

/* Default function to format objects.  Tries to use toLogString() but falls
 * back to toString().  Any errors are propagated out without catching.
 */
duk_ret_t duk_bi_logger_prototype_fmt(duk_context *ctx) {
	if (duk_get_prop_stridx(ctx, 0, DUK_STRIDX_TO_LOG_STRING)) {
		/* [ arg toLogString ] */

		duk_dup(ctx, 0);
		duk_call_method(ctx, 0);

		/* [ arg result ] */
		return 1;
	}

	/* [ arg undefined ] */
	duk_pop(ctx);
	duk_to_string(ctx, 0);
	return 1;
}

/* Default function to write a formatted log line.  Writes to stderr,
 * appending a newline to the log line.
 */
duk_ret_t duk_bi_logger_prototype_raw(duk_context *ctx) {
	const char *str;
	duk_size_t len;

	DUK_UNREF(ctx);
	DUK_UNREF(str);
	DUK_UNREF(len);

#ifdef DUK_USE_FILE_IO
	str = duk_require_lstring(ctx, 0, &len);
	fwrite((const void *) str, 1, len, stderr);
	fputc((int) '\n', stderr);
	fflush(stderr);
#else
	/* nop */
#endif
	return 0;
}

/* Log frontend shared helper, provides methods such as info().
 * This needs to have small footprint, reasonable performance,
 * minimal memory churn, etc.
 */
duk_ret_t duk_bi_logger_prototype_log_shared(duk_context *ctx) {
	duk_double_t now;
	duk_small_int_t entry_lev = duk_get_magic(ctx);
	duk_small_int_t logger_lev;
	duk_int_t nargs;
	duk_int_t i;
	duk_size_t tot_len;
	const duk_uint8_t *arg_str;
	duk_size_t arg_len;
	duk_uint8_t *buf, *p;
	const duk_uint8_t *q;
	char date_buf[64];  /*FIXME*/
	duk_size_t date_len;
	duk_small_int_t rc;

	DUK_ASSERT(entry_lev >= 0 && entry_lev <= 5);

	/*
	 *  Logger arguments are:
	 *
	 *    magic: log level (0-5)
	 *    this: logger
	 *    stack: plain log args
	 *
	 *  We want to minimize memory churn so a two-pass approach
	 *  is used: first pass formats arguments and computes final
	 *  string length, second pass copies strings into the result.
	 */

	nargs = duk_get_top(ctx);

	/* [ arg1 ... argN this ] */

	/*
	 *  Log level check
	 */

	duk_push_this(ctx);

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_LC_L);
	logger_lev = (duk_small_int_t) duk_get_int(ctx, -1);
	if (entry_lev < logger_lev) {
		return 0;
	}
	/* log level could be popped but that's not necessary */

	/* FIXME: change date formatting to provide a better timestmap, and
	 * avoid float formatting (portability).
	 */
	now = duk_bi_date_get_now(ctx) / 1000.0;
	DUK_SNPRINTF(date_buf, sizeof(date_buf), "%.3lf", now);
	date_len = DUK_STRLEN(date_buf);

	duk_get_prop_stridx(ctx, -2, DUK_STRIDX_LC_N);
	duk_to_string(ctx, -1);
	DUK_ASSERT(duk_is_string(ctx, -1));

	/* [ arg1 ... argN this loggerLevel loggerName ] */

	/*
	 *  Pass 1
	 */

	/* FIXME: buffer reuse? */
	/* FIXME: logger prefix */
	/* FIXME: sanitize printable */
	/* FIXME: formatting safety */
	/* FIXME: multiline */

	/* Line format: <time> <entryLev> <loggerName>: <msg> */

	tot_len = 0;
	tot_len += 3 +  /* separators: space, space, colon */
	           3 +  /* level string */
	           date_len +  /* time */
	           duk_get_length(ctx, -1);  /* loggerName */

	for (i = 0; i < nargs; i++) {
		/* When formatting an argument to a string, errors may happen from multiple
		 * causes.  In general we want to catch obvious errors like a toLogString()
		 * throwing an error, but we don't currently try to catch every possible
		 * error.  In particular, internal errors (like out of memory or stack) are
		 * not caught.  Also, we expect Error toString() to not throw an error.
		 */
		if (duk_is_object(ctx, i)) {
			/* duk_pcall_prop() may itself throw an error, but we're content
			 * in catching the obvious errors (like toLogString() throwing an
			 * error).
			 */
			duk_push_hstring_stridx(ctx, DUK_STRIDX_FMT);
			duk_dup(ctx, i);
			/* [ arg1 ... argN this loggerLevel loggerName 'fmt' arg ] */
			/* call: this.fmt(arg) */
			rc = duk_pcall_prop(ctx, -5 /*obj_index*/, 1 /*nargs*/, DUK_INVALID_INDEX /*errhandler_index*/);
			if (rc) {
				/* Keep the error as the result (coercing it might fail below,
				 * but we don't catch that now).
				 */
				;
			}
			duk_replace(ctx, i);
		}
		(void) duk_to_lstring(ctx, i, &arg_len);
		tot_len++;  /* sep (even before first one) */
		tot_len += arg_len;
	}

	/*
	 *  Pass 2
	 */

	buf = (duk_uint8_t *) duk_push_fixed_buffer(ctx, tot_len);
	DUK_ASSERT(buf != NULL);
	p = buf;

	DUK_MEMCPY((void *) p, (void *) date_buf, date_len);
	p += date_len;
	*p++ = (duk_uint8_t) DUK_ASC_SPACE;

	q = duk__log_level_strings + (entry_lev * 3);
	DUK_MEMCPY((void *) p, (void *) q, (duk_size_t) 3);
	p += 3;

	*p++ = (duk_uint8_t) DUK_ASC_SPACE;

	arg_str = (const duk_uint8_t *) duk_get_lstring(ctx, -2, &arg_len);
	DUK_MEMCPY((void *) p, (const void *) arg_str, arg_len);
	p += arg_len;

	*p++ = (duk_uint8_t) DUK_ASC_COLON;

	for (i = 0; i < nargs; i++) {
		*p++ = (duk_uint8_t) DUK_ASC_SPACE;

		arg_str = (const duk_uint8_t *) duk_get_lstring(ctx, i, &arg_len);
		DUK_ASSERT(arg_str != NULL);
		DUK_MEMCPY((void *) p, (const void *) arg_str, arg_len);
		p += arg_len;
	}
	DUK_ASSERT(buf + tot_len == p);

	/* [ arg1 ... argN this loggerLevel loggerName buffer ] */

	/* Call this.raw(msg); look up through the instance allows user to override
	 * the raw() function in the instance or in the prototype for maximum
	 * flexibility.
	 */
	duk_push_hstring_stridx(ctx, DUK_STRIDX_RAW);
	duk_push_lstring(ctx, (const char *) buf, tot_len);
	/* [ arg1 ... argN this loggerLevel loggerName buffer 'raw' msg ] */
	duk_call_prop(ctx, -6, 1);  /* this.raw(msg) */

	return 0;
}
