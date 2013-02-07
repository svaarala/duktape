/*
 *  Command line execution tool.
 *
 *  Needed for test cases and other manual testing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "duk_api.h"
#include "duk_internal.h"

#define  MEM_LIMIT   (128*1024*1024)  /* 128 MB */

extern void duk_ncurses_register(duk_context *ctx);
extern void duk_socket_register(duk_context *ctx);
extern void duk_fileio_register(duk_context *ctx);

int interactive_mode = 0;

static void set_resource_limits(void) {
	int rc;
	struct rlimit lim;

	rc = getrlimit(RLIMIT_AS, &lim);
	if (rc != 0) {
		fprintf(stderr, "Warning: cannot read RLIMIT_AS\n");
		return;
	}

	if (lim.rlim_max < MEM_LIMIT) {
		fprintf(stderr, "Warning: rlim_max < MEM_LIMIT (%d < %d)\n", (int) lim.rlim_max, (int) MEM_LIMIT);
		return;
	}

	lim.rlim_cur = MEM_LIMIT;
	lim.rlim_max = MEM_LIMIT;

	rc = setrlimit(RLIMIT_AS, &lim);
	if (rc != 0) {
		fprintf(stderr, "Warning: setrlimit failed\n");
		return;
	}

#if 0
	fprintf(stderr, "Set RLIMIT_AS to %d\n", MEM_LIMIT);
#endif
}

static void my_sighandler(int x) {
	fprintf(stderr, "Got signal %d\n", x);
	
}
static void set_sigint_handler(void) {
	(void) signal(SIGINT, my_sighandler);
}

int wrapped_compile_execute(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int comp_flags;

	/* FIXME: uses internal API */
	comp_flags = 0;
	duk_js_compile(thr, comp_flags);

        duk_js_push_closure(thr,
	                   (duk_hcompiledfunction *) duk_get_hobject(ctx, -1),
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV],
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV]);

	if (interactive_mode) {
		duk_hcompiledfunction *f = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);

		if (f && DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) f)) {
			fprintf(stdout, "[bytecode length %d opcodes, registers %d, constants %d, inner functions %d]\n",
			        (int) DUK_HCOMPILEDFUNCTION_GET_CODE_COUNT(f),
			        (int) f->nregs,
			        (int) DUK_HCOMPILEDFUNCTION_GET_CONSTS_COUNT(f),
			        (int) DUK_HCOMPILEDFUNCTION_GET_FUNCS_COUNT(f));
			fflush(stdout);
		} else {
			fprintf(stdout, "[invalid compile result]\n");
			fflush(stdout);
		}
	}

        duk_call(ctx, 0);

        duk_to_string(ctx, -1);

	if (interactive_mode) {
		/* FIXME: in interactive mode, write to stdout? */
		fprintf(stdout, "= %s\n", duk_get_string(ctx, -1));
		fflush(stdout);
	}

        duk_pop(ctx);
	return 0;
}

int handle_fh(duk_heap *heap, duk_context *ctx, FILE *f) {
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

	free(buf);
	buf = NULL;

	interactive_mode = 0;  /* global */

	rc = duk_safe_call(ctx, wrapped_compile_execute, 1 /*nargs*/, 1 /*nret*/, DUK_INVALID_INDEX);
	if (rc != DUK_ERR_EXEC_SUCCESS) {
		duk_to_string(ctx, -1);
		fprintf(stderr, "Error: %s (rc=%d)\n", duk_get_string(ctx, -1), rc);
		fflush(stderr);
		duk_pop(ctx);
		goto error;
	}

	duk_pop(ctx);
	retval = 0;
	/* fall thru */

 error:
	if (buf) {
		free(buf);
	}
	return retval;
}

int handle_file(duk_heap *heap, duk_context *ctx, const char *filename) {
	FILE *f = NULL;
	int retval;

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "failed to open source file: %s\n", filename);
		fflush(stderr);
		goto error;
	}

	retval = handle_fh(heap, ctx, f);

	fclose(f);
	return retval;

 error:
	return -1;
}

int handle_stdin(duk_heap *heap, duk_context *ctx) {
	int retval;

	retval = handle_fh(heap, ctx, stdin);

	return retval;
}

int handle_interactive(duk_heap *heap, duk_context *ctx) {
	const char *prompt = "duk> ";
	char *buffer = NULL;
	int retval = 0;
	int rc;

	/*
	 *  Note: using readline leads to valgrind-reported leaks inside
	 *  readline itself.  Execute code from an input file (and not
	 *  through stdin) for clean valgrind runs.
	 */

	rl_initialize();

	for (;;) {
		if (buffer) {
			free(buffer);
			buffer = NULL;
		}

		buffer = readline(prompt);
		if (!buffer) {
			break;
		}

		if (buffer && buffer[0] != (char) 0) {
			add_history(buffer);
		}

		duk_push_lstring(ctx, buffer, strlen(buffer));

		if (buffer) {
			free(buffer);
			buffer = NULL;
		}

		interactive_mode = 1;  /* global */

		rc = duk_safe_call(ctx, wrapped_compile_execute, 1 /*nargs*/, 1 /*nret*/, DUK_INVALID_INDEX);
		if (rc != DUK_ERR_EXEC_SUCCESS) {
			duk_to_string(ctx, -1);

			/* in interactive mode, write to stdout */
			fprintf(stdout, "Error: %s (rc=%d) [ignored]\n", duk_get_string(ctx, -1), rc);
			fflush(stdout);
			retval = -1;  /* an error 'taints' the execution */
		}

		duk_pop(ctx);
	}

	if (buffer) {
		free(buffer);
		buffer = NULL;
	}

	return retval;
}

int main(int argc, char *argv[]) {
	duk_heap *heap = NULL;
	duk_context *ctx = NULL;
	int retval = 0;
	const char *filename = NULL;
	int bytecode = 0;
	int interactive = 0;
	int i;

	set_resource_limits();
	set_sigint_handler();

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!arg) {
			goto usage;
		}
		if (strcmp(arg, "-b") == 0) {
			bytecode = 1;
		} else if (strlen(arg) > 1 && arg[0] == '-') {
			goto usage;
		} else {
			if (filename) {
				goto usage;
			}
			filename = arg;
		}
	}
	if (!filename) {
		interactive = 1;
	}
	if (bytecode) {
		fprintf(stderr, "bytecode execution not implemented\n");
		fflush(stderr);
		exit(1);
	}

	heap = duk_heap_alloc_default();
	ctx = (duk_context *) heap->heap_thread;

	duk_ncurses_register(ctx);
	duk_socket_register(ctx);
	duk_fileio_register(ctx);

	if (filename) {
		if (strcmp(filename, "-") == 0) {
			if (handle_stdin(heap, ctx) != 0) {
				retval = 1;
				goto cleanup;
			}
		} else {
			if (handle_file(heap, ctx, filename) != 0) {
				retval = 1;
				goto cleanup;
			}
		}
	}

	if (interactive) {
		if (handle_interactive(heap, ctx) != 0) {
			retval = 1;
			goto cleanup;
		}
	}

 cleanup:
	fprintf(stderr, "Cleaning up...\n");
	fflush(stderr);

	if (heap) {
		duk_heap_free(heap);
	}

	return retval;

 usage:
	fprintf(stderr, "Usage: duk [-b] <filename>\n");
	fprintf(stderr, "where\n");
	fprintf(stderr, "   -b      load bytecode instead of Ecmascript code\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "If <filename> is '-', the entire STDIN executed.\n");
	fprintf(stderr, "If <filename> is omitted, interactive mode is started.\n");
	fflush(stderr);
	exit(1);
}

