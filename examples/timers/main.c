/*
 *  Main for minloop command line tool.
 *
 *  Runs a given script from file or stdin inside an eventloop.  The
 *  script can then access setTimeout() etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "duktape.h"
#include "eventloop.h"
#include "duk_console.h"

extern void fileio_push_file_buffer(duk_context *ctx, const char *filename);
extern void fileio_push_file_string(duk_context *ctx, const char *filename);

duk_ret_t wrapped_compile_execute(duk_context *ctx, void *udata) {
	(void) udata;

	/* Finally, launch eventloop.  This call only returns after the
	 * eventloop terminates.
	 */
	eventloop_run(ctx);

	return 0;
}

int handle_fh(duk_context *ctx, FILE *f, const char *filename) {
	char *buf = NULL;
	int len;
	int got;
	int rc;
	int retval = -1;

	if (fseek(f, 0, SEEK_END) < 0) {
		goto error;
	}
	len = (int) ftell(f);
	if (fseek(f, 0, SEEK_SET) < 0) {
		goto error;
	}
	buf = (char *) malloc(len);
	if (!buf) {
		goto error;
	}

	got = fread((void *) buf, (size_t) 1, (size_t) len, f);

	duk_push_lstring(ctx, buf, got);
	duk_push_string(ctx, filename);

	free(buf);
	buf = NULL;

	rc = duk_safe_call(ctx, wrapped_compile_execute, NULL, 2 /*nargs*/, 1 /*nret*/);
	if (rc != DUK_EXEC_SUCCESS) {
		fprintf(stderr, "%s\n", duk_safe_to_stacktrace(ctx, -1));
		fflush(stderr);
		goto error;
	} else {
		duk_pop(ctx);
		retval = 0;
	}
	/* fall thru */

 error:
	if (buf) {
		free(buf);
	}
	return retval;
}

int handle_file(duk_context *ctx, const char *filename) {
	FILE *f = NULL;
	int retval;

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "failed to open source file: %s\n", filename);
		fflush(stderr);
		goto error;
	}

	retval = handle_fh(ctx, f, filename);

	fclose(f);
	return retval;

 error:
	return -1;
}

int handle_stdin(duk_context *ctx) {
	int retval;

	retval = handle_fh(ctx, stdin, "stdin");

	return retval;
}

int main(int argc, char *argv[]) {
	duk_context *ctx = NULL;
	int retval = 0;
	const char *filename = NULL;
	int i;

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!arg) {
			goto usage;
		}
		if (strlen(arg) > 1 && arg[0] == '-') {
			goto usage;
		} else {
			if (filename) {
				goto usage;
			}
			filename = arg;
		}
	}
	if (!filename) {
		goto usage;
	}

	ctx = duk_create_heap_default();

    if (duk_safe_call(ctx, eventloop_register, NULL, 0 /*nargs*/, 1 /*nret*/) != 0) {
        fprintf(stderr, "Unable To Initialize Eventloop: %s\n", duk_safe_to_stacktrace(ctx, -1));
        exit(1);
    }
    duk_pop(ctx);

	duk_console_init(ctx, 0);

	if (strcmp(filename, "-") == 0) {
		if (handle_stdin(ctx) != 0) {
			retval = 1;
			goto cleanup;
		}
	} else {
		if (handle_file(ctx, filename) != 0) {
			retval = 1;
			goto cleanup;
		}
	}

 cleanup:
	if (ctx) {
		duk_destroy_heap(ctx);
	}

	return retval;

 usage:
	fprintf(stderr, "Usage: timers <filename>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Uses a C based eventloop (c_eventloop.{c,js}).\n");
	fprintf(stderr, "If <filename> is '-', the entire STDIN executed.\n");
	fflush(stderr);
	exit(1);
}
