#include <stdio.h>
#include "duktape.h"
#include "duk_print_alert.h"

int main(int argc, char *argv[]) {
	duk_context *ctx;
	int i;

	ctx = duk_create_heap_default();
	if (!ctx) {
		return 1;
	}

	duk_print_alert_init(ctx);

	for (i = 1; i < argc; i++) {
		printf("Evaling: %s\n", argv[i]);
		duk_eval_string_noresult(ctx, argv[i]);
	}

	printf("Done\n");
	duk_destroy_heap(ctx);
	return 0;
}
