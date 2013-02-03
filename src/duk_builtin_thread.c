/*
 *  Thread builtins
 */

#include "duk_internal.h"

int duk_builtin_thread_prototype_to_string(duk_context *ctx) {
	/* FIXME: what to print, something special or just [object Object]? */
	/* FIXME: state */
	duk_push_sprintf(ctx, "[object Thread]");
	return 1;
}

