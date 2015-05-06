/*
 *  Logging
 *
 *  Current logging primitive is a sprintf-style log which is convenient
 *  for most C code.  Another useful primitive would be to log N arguments
 *  from value stack (like the Ecmascript binding does).
 */

#include "duk_internal.h"

DUK_EXTERNAL void duk_log_va(duk_context *ctx, duk_int_t level, const char *fmt, va_list ap) {
	/* stridx_logfunc[] must be static to allow initializer with old compilers like BCC */
	static const duk_uint16_t stridx_logfunc[6] = {
		DUK_STRIDX_LC_TRACE, DUK_STRIDX_LC_DEBUG, DUK_STRIDX_LC_INFO,
		DUK_STRIDX_LC_WARN, DUK_STRIDX_LC_ERROR, DUK_STRIDX_LC_FATAL
	};

	DUK_ASSERT_CTX_VALID(ctx);

	if (level < 0) {
		level = 0;
	} else if (level > (int) (sizeof(stridx_logfunc) / sizeof(duk_uint16_t)) - 1) {
		level = (int) (sizeof(stridx_logfunc) / sizeof(duk_uint16_t)) - 1;
	}

	duk_push_hobject_bidx(ctx, DUK_BIDX_LOGGER_CONSTRUCTOR);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_CLOG);
	duk_get_prop_stridx(ctx, -1, stridx_logfunc[level]);
	duk_dup(ctx, -2);

	/* [ ... Logger clog logfunc clog ] */

	duk_push_vsprintf(ctx, fmt, ap);

	/* [ ... Logger clog logfunc clog(=this) msg ] */

	duk_call_method(ctx, 1 /*nargs*/);

	/* [ ... Logger clog res ] */

	duk_pop_3(ctx);
}

DUK_EXTERNAL void duk_log(duk_context *ctx, duk_int_t level, const char *fmt, ...) {
	va_list ap;

	DUK_ASSERT_CTX_VALID(ctx);

	va_start(ap, fmt);
	duk_log_va(ctx, level, fmt, ap);
	va_end(ap);
}
