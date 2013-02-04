/*
 *  Handle fatal error.
 */

#include "duk_internal.h"

void duk_default_fatal_handler(duk_context *ctx, int code) {
	DUK_DPRINT("default fatal handler called, code %d -> calling DUK_PANIC()", code);
	DUK_PANIC(code, "fatal error (default handler), code %d", code);
	DUK_NEVER_HERE();
}


