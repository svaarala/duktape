/*
 *  C entrypoints for dukweb.js
 *
 *  https://github.com/kripken/emscripten/wiki/Interacting-with-code
 */

#include <stdio.h>
#include <string.h>
#include "duktape.h"

static duk_context *dukweb_ctx = NULL;
static char duk__evalbuf[1024 * 1024];

void dukweb_open(void) {
	if (dukweb_ctx) {
		printf("dukweb_open: heap already exists, destroying previous heap first\n");
		duk_destroy_heap(dukweb_ctx);
		dukweb_ctx = NULL;
	}
	printf("dukweb_open: creating heap\n");
	dukweb_ctx = duk_create_heap_default();
}

void dukweb_close(void) {
	if (dukweb_ctx) {
		printf("dukweb_close: destroying heap\n");
		duk_destroy_heap(dukweb_ctx);
		dukweb_ctx = NULL;
	} else {
		printf("dukweb_close: heap doesn't exist, no-op\n");
	}
}

static int dukweb__eval_wrapper(duk_context *ctx) {
	duk_eval(ctx);
	return 1;
}

static int dukweb__tostring_wrapper(duk_context *ctx) {
	duk_to_string(ctx, -1);
	return 1;
}

/* A very limited eval facility: one string input (eval code), one string
 * output (eval result or error, coerced with ToString()).  Data marshalling
 * needs to be implemented on top of this.
 */
const char *dukweb_eval(const char *code) {
	const char *res;
	duk_context *ctx = dukweb_ctx;

	if (!code) {
		sprintf(duk__evalbuf, "\"code argument is null\"");
		return (const char *) duk__evalbuf;
	}
	if (!ctx) {
		sprintf(duk__evalbuf, "\"heap does not exist\"");
		return (const char *) duk__evalbuf;
	}

	printf("dukweb_eval: '%s'\n", code);
	duk_push_string(ctx, code);
	if (duk_safe_call(ctx, dukweb__eval_wrapper, 1 /*nargs*/, 1 /*nrets*/, DUK_INVALID_INDEX)) {
		/* failure */
		(void) duk_safe_call(ctx, dukweb__tostring_wrapper, 1 /*nargs*/, 1 /*nrets*/, DUK_INVALID_INDEX);
		res = duk_get_string(ctx, -1);
		printf("dukweb_eval: result is error: %s\n", res ? res : "(null)");
	} else {
		/* success */
		(void) duk_safe_call(ctx, dukweb__tostring_wrapper, 1 /*nargs*/, 1 /*nrets*/, DUK_INVALID_INDEX);
		res = duk_get_string(ctx, -1);
		printf("dukweb_eval: result is success: %s\n", res ? res : "(null)");
	}

	if (res) {
		size_t len = strlen(res);

		if (len > sizeof(duk__evalbuf) - 1) {
			sprintf(duk__evalbuf, "\"eval result too long\"");
		} else {
			memmove(duk__evalbuf, (const void *) res, len + 1);  /* include NUL */
		}
	} else {
		sprintf(duk__evalbuf, "\"eval result null\"");
	}

	duk_set_top(ctx, 0);

	return (const char *) duk__evalbuf;
}

int main(int argc, char *argv[]) {
	printf("main()\n");
}
