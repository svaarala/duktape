/*
 *  Command line execution tool.  Used by test cases and other manual testing.
 *
 *  For maximum portability, compile with -DDUK_CMDLINE_BAREBONES
 */

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#ifndef DUK_CMDLINE_BAREBONES
/* Force barebones mode on Windows. */
#define DUK_CMDLINE_BAREBONES
#endif
#endif

#ifdef DUK_CMDLINE_BAREBONES
#define NO_READLINE
#define NO_RLIMIT
#define NO_SIGNAL
#endif

#define  GREET_CODE(variant)  \
	"print(" \
	"'((o) Duktape" variant "'" \
	", " \
	"Math.floor(Duktape.version / 10000) + '.' + " \
	"Math.floor(Duktape.version / 100) % 100 + '.' + " \
	"Duktape.version % 100" \
	");"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NO_SIGNAL
#include <signal.h>
#endif
#ifndef NO_RLIMIT
#include <sys/resource.h>
#endif
#ifndef NO_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "duktape.h"

#define  MEM_LIMIT_NORMAL   (128*1024*1024)   /* 128 MB */
#define  MEM_LIMIT_HIGH     (2047*1024*1024)  /* ~2 GB */
#define  LINEBUF_SIZE       65536

/* FIXME: additional modules should probably only be in some separate tool? */
#if 0
extern void duk_ncurses_register(duk_context *ctx);
extern void duk_socket_register(duk_context *ctx);
extern void duk_fileio_register(duk_context *ctx);
#endif

int interactive_mode = 0;

#ifndef NO_RLIMIT
static void set_resource_limits(rlim_t mem_limit_value) {
	int rc;
	struct rlimit lim;

	rc = getrlimit(RLIMIT_AS, &lim);
	if (rc != 0) {
		fprintf(stderr, "Warning: cannot read RLIMIT_AS\n");
		return;
	}

	if (lim.rlim_max < mem_limit_value) {
		fprintf(stderr, "Warning: rlim_max < mem_limit_value (%d < %d)\n", (int) lim.rlim_max, (int) mem_limit_value);
		return;
	}

	lim.rlim_cur = mem_limit_value;
	lim.rlim_max = mem_limit_value;

	rc = setrlimit(RLIMIT_AS, &lim);
	if (rc != 0) {
		fprintf(stderr, "Warning: setrlimit failed\n");
		return;
	}

#if 0
	fprintf(stderr, "Set RLIMIT_AS to %d\n", (int) mem_limit_value);
#endif
}
#endif  /* NO_RLIMIT */

#ifndef NO_SIGNAL
static void my_sighandler(int x) {
	fprintf(stderr, "Got signal %d\n", x);
	
}
static void set_sigint_handler(void) {
	(void) signal(SIGINT, my_sighandler);
}
#endif  /* NO_SIGNAL */

/* Print error to stderr and pop error. */
static void print_error(duk_context *ctx, FILE *f) {
	if (duk_is_object(ctx, -1) && duk_has_prop_string(ctx, -1, "stack")) {
		/* FIXME: print error objects specially */
		/* FIXME: pcall the string coercion */
		duk_get_prop_string(ctx, -1, "stack");
		if (duk_is_string(ctx, -1)) {
			fprintf(f, "%s\n", duk_get_string(ctx, -1));
			fflush(f);
			duk_pop_2(ctx);
			return;
		} else {
			duk_pop(ctx);
		}
	}
	duk_to_string(ctx, -1);
	fprintf(f, "%s\n", duk_get_string(ctx, -1));
	fflush(f);
	duk_pop(ctx);
}

int wrapped_compile_execute(duk_context *ctx) {
	int comp_flags;

	comp_flags = 0;
	duk_compile(ctx, comp_flags);

#if 0
	/* FIXME: something similar with public API */
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
#endif

	duk_push_global_object(ctx);  /* 'this' binding */
	duk_call_method(ctx, 0);

	if (interactive_mode) {
		/*
		 *  In interactive mode, write to stdout so output won't interleave as easily.
		 *
		 *  NOTE: the ToString() coercion may fail in some cases; for instance,
		 *  if you evaluate:
		 *
		 *    ( {valueOf: function() {return {}}, toString: function() {return {}}});
		 *
		 *  The error is:
		 *
		 *    TypeError: failed to coerce with [[DefaultValue]]
		 *            duk_api.c:1420
		 *
		 *  This should be fixed at some point.
		 */

		duk_to_string(ctx, -1);
		fprintf(stdout, "= %s\n", duk_get_string(ctx, -1));
		fflush(stdout);
	} else {
		/* In non-interactive mode, success results are not written at all.
		 * It is important that the result value is not string coerced,
		 * as the string coercion may cause an error in some cases.
		 */
	}

	duk_pop(ctx);
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

	interactive_mode = 0;  /* global */

	rc = duk_safe_call(ctx, wrapped_compile_execute, 2 /*nargs*/, 1 /*nret*/, DUK_INVALID_INDEX);
	if (rc != DUK_EXEC_SUCCESS) {
		print_error(ctx, stderr);
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

#ifdef NO_READLINE
int handle_interactive(duk_context *ctx) {
	const char *prompt = "duk> ";
	char *buffer = NULL;
	int retval = 0;
	int rc;
	int got_eof = 0;

	duk_eval_string(ctx, GREET_CODE(" [no readline]"));
	duk_pop(ctx);

	buffer = malloc(LINEBUF_SIZE);
	if (!buffer) {
		fprintf(stderr, "failed to allocated a line buffer\n");
		fflush(stderr);
		retval = -1;
		goto done;
	}

	while (!got_eof) {
		size_t idx = 0;

		fwrite(prompt, 1, strlen(prompt), stdout);
		fflush(stdout);

		for (;;) {
			int c = fgetc(stdin);
			if (c == EOF) {
				got_eof = 1;
				break;
			} else if (c == '\n') {
				break;
			} else if (idx >= LINEBUF_SIZE) {
				fprintf(stderr, "line too long\n");
				fflush(stderr);
				retval = -1;
				goto done;
			} else {
				buffer[idx++] = (char) c;
			}
		}

		duk_push_lstring(ctx, buffer, idx);
		duk_push_string(ctx, "input");

		interactive_mode = 1;  /* global */

		rc = duk_safe_call(ctx, wrapped_compile_execute, 2 /*nargs*/, 1 /*nret*/, DUK_INVALID_INDEX);
		if (rc != DUK_EXEC_SUCCESS) {
			/* in interactive mode, write to stdout */
			print_error(ctx, stdout);
			retval = -1;  /* an error 'taints' the execution */
		} else {
			duk_pop(ctx);
		}
	}

 done:
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}

	return retval;
}
#else  /* NO_READLINE */
int handle_interactive(duk_context *ctx) {
	const char *prompt = "duk> ";
	char *buffer = NULL;
	int retval = 0;
	int rc;

	duk_eval_string(ctx, GREET_CODE(""));
	duk_pop(ctx);

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
		duk_push_string(ctx, "input");

		if (buffer) {
			free(buffer);
			buffer = NULL;
		}

		interactive_mode = 1;  /* global */

		rc = duk_safe_call(ctx, wrapped_compile_execute, 2 /*nargs*/, 1 /*nret*/, DUK_INVALID_INDEX);
		if (rc != DUK_EXEC_SUCCESS) {
			/* in interactive mode, write to stdout */
			print_error(ctx, stdout);
			retval = -1;  /* an error 'taints' the execution */
		} else {
			duk_pop(ctx);
		}
	}

	if (buffer) {
		free(buffer);
		buffer = NULL;
	}

	return retval;
}
#endif  /* NO_READLINE */

int main(int argc, char *argv[]) {
	duk_context *ctx = NULL;
	int retval = 0;
	const char *filename = NULL;
	int interactive = 0;
	int memlimit_high = 1;
	int i;

#ifndef NO_SIGNAL
	set_sigint_handler();

	/* This is useful at the global level; libraries should avoid SIGPIPE though */
	/*signal(SIGPIPE, SIG_IGN);*/
#endif

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!arg) {
			goto usage;
		}
		if (strcmp(arg, "-r") == 0) {
			memlimit_high = 0;
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

#ifndef NO_RLIMIT
	set_resource_limits(memlimit_high ? MEM_LIMIT_HIGH : MEM_LIMIT_NORMAL);
#else
	(void) memlimit_high;  /* suppress warning */
#endif

	ctx = duk_create_heap_default();

#if 0
	duk_ncurses_register(ctx);
	duk_socket_register(ctx);
	duk_fileio_register(ctx);
#endif

	if (filename) {
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
	}

	if (interactive) {
		if (handle_interactive(ctx) != 0) {
			retval = 1;
			goto cleanup;
		}
	}

 cleanup:
	if (interactive) {
		fprintf(stderr, "Cleaning up...\n");
		fflush(stderr);
	}

	if (ctx) {
		duk_destroy_heap(ctx);
	}

	return retval;

 usage:
	fprintf(stderr, "Usage: duk [-r] <filename>\n");
	fprintf(stderr, "where\n");
	fprintf(stderr, "   -r      use lower memory limit\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "If <filename> is '-', the entire STDIN executed.\n");
	fprintf(stderr, "If <filename> is omitted, interactive mode is started.\n");
	fflush(stderr);
	exit(1);
}

