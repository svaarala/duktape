#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "duktape.h"

int main(int argc, const char *argv[]) {
    duk_context *ctx = NULL;
    char line[4096];
    char idx;
    int ch;

    ctx = duk_create_heap_default();
    if (!ctx) { exit(1); }

    duk_eval_file(ctx, "process.js");
    duk_pop(ctx);  /* pop eval result */

    memset(line, 0, sizeof(line));
    idx = 0;
    for (;;) {
        if (idx >= sizeof(line)) { exit(1); }

        ch = fgetc(stdin);
        if (ch == 0x0a) {
            line[idx++] = '\0';

            duk_push_global_object(ctx);
            duk_get_prop_string(ctx, -1, "processLine");
            duk_push_string(ctx, line);
            duk_call(ctx, 1);
            printf("%s\n", duk_to_string(ctx, -1));
            duk_pop(ctx);

            idx = 0;
        } else if (ch == EOF) {
            break;
        } else {
            line[idx++] = (char) ch;
        }
    }

    duk_destroy_heap(ctx);

    exit(0);
}
