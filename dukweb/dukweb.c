/*
 *  C entrypoints for dukweb.js
 *
 *  https://github.com/kripken/emscripten/wiki/Interacting-with-code
 */

#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include "duktape.h"

static duk_context *dukweb_ctx = NULL;
static char duk__evalbuf[1024 * 1024];

static int dukweb__emscripten_run_script(duk_context *ctx) {
	const char *code = duk_get_string(ctx, -1);
	if (!code) {
		return DUK_RET_TYPE_ERROR;
	}
	/* FIXME: return value */
	emscripten_run_script(code);
	return 0;
}

void dukweb_panic_handler(int code, const char *msg) {
	printf("dukweb_panic_handler(), code=%d, msg=%s\n", code, msg);
	fflush(stdout);
	/* FIXME: add code/msg to alert */
	EM_ASM(
		alert('Duktape panic handler called');
	);
	abort();
}

void dukweb_fatal_handler(duk_context *ctx, int code, const char *msg) {
	printf("dukweb_fatal_handler(), code=%d, msg=%s\n", code, msg);
	fflush(stdout);
	/* FIXME: add code/msg to alert */
	EM_ASM(
		alert('Duktape fatal handler called');
	);
	abort();
}

int dukweb_is_open(void) {
	if (dukweb_ctx) {
		return 1;
	}
	return 0;
}

void dukweb_open(void) {
	if (dukweb_ctx) {
		printf("dukweb_open: heap already exists, destroying previous heap first\n");
		duk_destroy_heap(dukweb_ctx);
		dukweb_ctx = NULL;
	}
	printf("dukweb_open: creating heap\n");
	dukweb_ctx = duk_create_heap(NULL, NULL, NULL, NULL, dukweb_fatal_handler);

	/* add a binding to emscripten_run_script(), let init code move it
	 * to a better place
	 */

	duk_push_global_object(dukweb_ctx);
	duk_push_string(dukweb_ctx, "emscripten_run_script");
	duk_push_c_function(dukweb_ctx, dukweb__emscripten_run_script, 1 /*nargs*/);
	duk_put_prop(dukweb_ctx, -3);
	duk_set_top(dukweb_ctx, 0);
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

/* A very limited eval facility: one string input (eval code), one string
 * output (eval result or error, coerced with ToString()).  Data marshalling
 * needs to be implemented on top of this.
 *
 * FIXME: proper return value model which identifies errors from success values
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
	if (duk_peval(ctx) != 0) {
		/* failure */
		res = duk_safe_to_string(ctx, -1);
		printf("dukweb_eval: result is error: %s\n", res ? res : "(null)");
	} else {
		/* success */
		res = duk_safe_to_string(ctx, -1);
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
