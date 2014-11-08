/*
 *  Logging support
 */

#include "duk_internal.h"

/* 3-letter log level strings */
DUK_LOCAL const duk_uint8_t duk__log_level_strings[] = {
	(duk_uint8_t) DUK_ASC_UC_T, (duk_uint8_t) DUK_ASC_UC_R, (duk_uint8_t) DUK_ASC_UC_C,
	(duk_uint8_t) DUK_ASC_UC_D, (duk_uint8_t) DUK_ASC_UC_B, (duk_uint8_t) DUK_ASC_UC_G,
	(duk_uint8_t) DUK_ASC_UC_I, (duk_uint8_t) DUK_ASC_UC_N, (duk_uint8_t) DUK_ASC_UC_F,
	(duk_uint8_t) DUK_ASC_UC_W, (duk_uint8_t) DUK_ASC_UC_R, (duk_uint8_t) DUK_ASC_UC_N,
	(duk_uint8_t) DUK_ASC_UC_E, (duk_uint8_t) DUK_ASC_UC_R, (duk_uint8_t) DUK_ASC_UC_R,
	(duk_uint8_t) DUK_ASC_UC_F, (duk_uint8_t) DUK_ASC_UC_T, (duk_uint8_t) DUK_ASC_UC_L
};

/* Constructor */
DUK_INTERNAL duk_ret_t duk_bi_logger_constructor(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t nargs;

	/* Calling as a non-constructor is not meaningful. */
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}

	nargs = duk_get_top(ctx);
	duk_set_top(ctx, 1);

	duk_push_this(ctx);

	/* [ name this ] */

	if (nargs == 0) {
		/* Automatic defaulting of logger name from caller.  This would
		 * work poorly with tail calls, but constructor calls are currently
		 * never tail calls, so tail calls are not an issue now.
		 */

		if (thr->callstack_top >= 2) {
			duk_activation *act_caller = thr->callstack + thr->callstack_top - 2;
			duk_hobject *func_caller;

			func_caller = DUK_ACT_GET_FUNC(act_caller);
			if (func_caller) {
				/* Stripping the filename might be a good idea
				 * ("/foo/bar/quux.js" -> logger name "quux"),
				 * but now used verbatim.
				 */
				duk_push_hobject(ctx, func_caller);
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
DUK_INTERNAL duk_ret_t duk_bi_logger_prototype_fmt(duk_context *ctx) {
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
 *
 * The argument is a buffer whose visible size contains the log message.
 * This function should avoid coercing the buffer to a string to avoid
 * string table traffic.
 */
DUK_INTERNAL duk_ret_t duk_bi_logger_prototype_raw(duk_context *ctx) {
	const char *data;
	duk_size_t data_len;

	DUK_UNREF(ctx);
	DUK_UNREF(data);
	DUK_UNREF(data_len);

#ifdef DUK_USE_FILE_IO
	data = (const char *) duk_require_buffer(ctx, 0, &data_len);
	DUK_FWRITE((const void *) data, 1, data_len, DUK_STDERR);
	DUK_FPUTC((int) '\n', DUK_STDERR);
	DUK_FFLUSH(DUK_STDERR);
#else
	/* nop */
#endif
	return 0;
}

/* Log frontend shared helper, magic value indicates log level.  Provides
 * frontend functions: trace(), debug(), info(), warn(), error(), fatal().
 * This needs to have small footprint, reasonable performance, minimal
 * memory churn, etc.
 */
DUK_INTERNAL duk_ret_t duk_bi_logger_prototype_log_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_double_t now;
	duk_small_int_t entry_lev = duk_get_current_magic(ctx);
	duk_small_int_t logger_lev;
	duk_int_t nargs;
	duk_int_t i;
	duk_size_t tot_len;
	const duk_uint8_t *arg_str;
	duk_size_t arg_len;
	duk_uint8_t *buf, *p;
	const duk_uint8_t *q;
	duk_uint8_t date_buf[DUK_BI_DATE_ISO8601_BUFSIZE];
	duk_size_t date_len;
	duk_small_int_t rc;

	DUK_ASSERT(entry_lev >= 0 && entry_lev <= 5);

	/* XXX: sanitize to printable (and maybe ASCII) */
	/* XXX: better multiline */

	/*
	 *  Logger arguments are:
	 *
	 *    magic: log level (0-5)
	 *    this: logger
	 *    stack: plain log args
	 *
	 *  We want to minimize memory churn so a two-pass approach
	 *  is used: first pass formats arguments and computes final
	 *  string length, second pass copies strings either into a
	 *  pre-allocated and reused buffer (short messages) or into a
	 *  newly allocated fixed buffer.  If the backend function plays
	 *  nice, it won't coerce the buffer to a string (and thus
	 *  intern it).
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

	now = duk_bi_date_get_now(ctx);
	duk_bi_date_format_timeval(now, date_buf);
	date_len = DUK_STRLEN((const char *) date_buf);

	duk_get_prop_stridx(ctx, -2, DUK_STRIDX_LC_N);
	duk_to_string(ctx, -1);
	DUK_ASSERT(duk_is_string(ctx, -1));

	/* [ arg1 ... argN this loggerLevel loggerName ] */

	/*
	 *  Pass 1
	 */

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
			rc = duk_pcall_prop(ctx, -5 /*obj_index*/, 1 /*nargs*/);
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

	if (tot_len <= DUK_BI_LOGGER_SHORT_MSG_LIMIT) {
		duk_hbuffer_dynamic *h_buf;

		DUK_DDD(DUK_DDDPRINT("reuse existing small log message buffer, tot_len %ld", (long) tot_len));

		/* We can assert for all buffer properties because user code
		 * never has access to heap->log_buffer.
		 */

		DUK_ASSERT(thr != NULL);
		DUK_ASSERT(thr->heap != NULL);
		h_buf = thr->heap->log_buffer;
		DUK_ASSERT(h_buf != NULL);
		DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC((duk_hbuffer *) h_buf));
		DUK_ASSERT(DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(h_buf) == DUK_BI_LOGGER_SHORT_MSG_LIMIT);

		/* Set buffer 'visible size' to actual message length and
		 * push it to the stack.
		 */

		DUK_HBUFFER_SET_SIZE((duk_hbuffer *) h_buf, tot_len);
		duk_push_hbuffer(ctx, (duk_hbuffer *) h_buf);
		buf = (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, h_buf);
	} else {
		DUK_DDD(DUK_DDDPRINT("use a one-off large log message buffer, tot_len %ld", (long) tot_len));
		buf = (duk_uint8_t *) duk_push_fixed_buffer(ctx, tot_len);
	}
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

#if defined(DUK_USE_DEBUGGER_SUPPORT) && defined(DUK_USE_DEBUGGER_FWD_LOGGING)
	/* Do debugger forwarding before raw() because the raw() function
	 * doesn't get the log level right now.
	 */
	if (DUK_HEAP_IS_DEBUGGER_ATTACHED(thr->heap)) {
		const char *log_buf;
		duk_size_t sz_buf;
		log_buf = (const char *) duk_get_buffer(ctx, -1, &sz_buf);
		DUK_ASSERT(log_buf != NULL);
		duk_debug_write_notify(thr, DUK_DBG_CMD_LOG);
		duk_debug_write_int(thr, (duk_int32_t) entry_lev);
		duk_debug_write_string(thr, (const char *) log_buf, sz_buf);
		duk_debug_write_eom(thr);
	}
#endif

	/* Call this.raw(msg); look up through the instance allows user to override
	 * the raw() function in the instance or in the prototype for maximum
	 * flexibility.
	 */
	duk_push_hstring_stridx(ctx, DUK_STRIDX_RAW);
	duk_dup(ctx, -2);
	/* [ arg1 ... argN this loggerLevel loggerName buffer 'raw' buffer ] */
	duk_call_prop(ctx, -6, 1);  /* this.raw(buffer) */

	return 0;
}
