'use strict';

exports.cTestFrameworkSource = `\
/* START TEST INIT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>  /* INT_MIN, INT_MAX */
#include "duktape.h"

#define  TEST_SAFE_CALL(func)  do { \\
\t\tduk_ret_t _rc; \\
\t\tprintf("*** %s (duk_safe_call)\\n", #func); \\
\t\tfflush(stdout); \\
\t\t_rc = duk_safe_call(ctx, (func), NULL, 0 /*nargs*/, 1 /*nrets*/); \\
\t\tprintf("==> rc=%d, result='%s'\\n", (int) _rc, duk_safe_to_string(ctx, -1)); \\
\t\tfflush(stdout); \\
\t\tduk_pop(ctx); \\
\t} while (0)

#define  TEST_PCALL(func)  do { \\
\t\tduk_ret_t _rc; \\
\t\t printf("*** %s (duk_pcall)\\n", #func); \\
\t\tfflush(stdout); \\
\t\tduk_push_c_function(ctx, (func), 0); \\
\t\t_rc = duk_pcall(ctx, 0); \\
\t\tprintf("==> rc=%d, result='%s'\\n", (int) _rc, duk_safe_to_string(ctx, -1)); \\
\t\tfflush(stdout); \\
\t\tduk_pop(ctx); \\
\t} while (0)

#line 1  /* END TEST INIT */
`;

exports.cTestcaseMain = `\
/* START TEST MAIN */
static duk_ret_t runtests_print_alert_helper(duk_context *ctx, FILE *fh) {
\tduk_idx_t nargs;
\tconst duk_uint8_t *buf;
\tduk_size_t sz_buf;

\tnargs = duk_get_top(ctx);
\tif (nargs == 1 && duk_is_buffer(ctx, 0)) {
\t\tbuf = (const duk_uint8_t *) duk_get_buffer(ctx, 0, &sz_buf);
\t\tfwrite((const void *) buf, 1, (size_t) sz_buf, fh);
\t} else {
\t\tduk_push_string(ctx, " ");
\t\tduk_insert(ctx, 0);
\t\tduk_join(ctx, nargs);
\t\tfprintf(fh, "%s\\n", duk_to_string(ctx, -1));
\t}
\tfflush(fh);
\treturn 0;
}
static duk_ret_t runtests_print(duk_context *ctx) {
\treturn runtests_print_alert_helper(ctx, stdout);
}
static duk_ret_t runtests_alert(duk_context *ctx) {
\treturn runtests_print_alert_helper(ctx, stderr);
}

int main(int argc, char *argv[]) {
\tduk_context *ctx = NULL;

\tctx = duk_create_heap_default();
\tif (!ctx) {
\t\tfprintf(stderr, "cannot allocate heap for testcase\\n");
\t\texit(1);
\t}

\t/* Minimal print() replacement; removed in Duktape 2.x. */
\tduk_push_global_object(ctx);
\tduk_push_string(ctx, "print");
\tduk_push_c_function(ctx, runtests_print, DUK_VARARGS);
\tduk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
\tduk_push_string(ctx, "alert");
\tduk_push_c_function(ctx, runtests_alert, DUK_VARARGS);
\tduk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
\tduk_pop(ctx);

\ttest(ctx);

\tduk_destroy_heap(ctx);
}
/* END TEST MAIN */
`;
