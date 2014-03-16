/*
 *  Logging
 *
 *  Current logging primitive is a sprintf-style log which is convenient
 *  for most C code.  Another useful primitive would be to log N arguments
 *  from value stack (like the Ecmascript binding does).
 */

#include "duk_internal.h"

/* FIXME: dynamic? shared code with sprintf string pusher? */
#define DUK__LOGFMT_BUFSIZE  256  /* size for formatting buffers */

void duk_log(duk_context *ctx, int level, const char *fmt, ...) {
	duk_hthread *thr = (duk_hthread *) ctx;
	va_list ap;
	char buf[DUK__LOGFMT_BUFSIZE];

	va_start(ap, fmt);
	DUK_VSNPRINTF(buf, sizeof(buf), fmt, ap);
	buf[sizeof(buf) - 1] = (char) 0;
	va_end(ap);

	duk_push_hobject(ctx, thr->builtins[DUK_BIDX_LOGGER_CONSTRUCTOR]);
	duk_get_prop_string(ctx, -1, "clog");  /*FIXME*/
	duk_get_prop_string(ctx, -1, "info");  /*FIXME:level*/

	/* [ ... Logger clog info ] */

	duk_dup(ctx, -2);
	duk_push_string(ctx, buf);  /*FIXME: duk_push_vsprintf? */

	/* [ ... Logger clog info clog msg ] */

	duk_call_method(ctx, 1 /*nargs*/);

	/* [ ... Logger clog res ] */

	duk_pop_3(ctx);
}

