/* primecheck.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "duktape.h"

static duk_ret_t native_prime_check(duk_context *ctx) {
    int val = duk_require_int(ctx, 0);
    int lim = duk_require_int(ctx, 1);
    int i;

    for (i = 2; i <= lim; i++) {
        if (val % i == 0) {
            duk_push_false(ctx);
            return 1;
        }
    }

    duk_push_true(ctx);
    return 1;
}

int main(int argc, const char *argv[]) {
    duk_context *ctx = NULL;

    ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        exit(1);
    }

    duk_push_global_object(ctx);
    duk_push_c_function(ctx, native_prime_check, 2 /*nargs*/);
    duk_put_prop_string(ctx, -2, "primeCheckNative");

    if (duk_peval_file(ctx, "prime.js") != 0) {
        printf("Error: %s\n", duk_safe_to_string(ctx, -1));
        goto finished;
    }
    duk_pop(ctx);  /* ignore result */

    duk_get_prop_string(ctx, -1, "primeTest");
    if (duk_pcall(ctx, 0) != 0) {
        printf("Error: %s\n", duk_safe_to_string(ctx, -1));
    }
    duk_pop(ctx);  /* ignore result */

 finished:
    duk_destroy_heap(ctx);

    exit(0);
}
