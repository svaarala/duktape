/*
 *  Command line execution tool.  Useful for test cases and manual testing.
 *
 *  To enable readline and other fancy stuff, compile with -DDUK_CMDLINE_FANCY.
 *  It is not the default to maximize portability.  You can also compile in
 *  support for example allocators, grep for DUK_CMDLINE_*.
 */

#ifndef DUK_CMDLINE_FANCY
#define NO_READLINE
#define NO_RLIMIT
#define NO_SIGNAL
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || \
    defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
/* Suppress warnings about plain fopen() etc. */
#define _CRT_SECURE_NO_WARNINGS
#endif

#define  GREET_CODE(variant)  \
	"print('((o) Duktape" variant " ' + " \
	"Math.floor(Duktape.version / 10000) + '.' + " \
	"Math.floor(Duktape.version / 100) % 100 + '.' + " \
	"Duktape.version % 100" \
	", '(" DUK_GIT_DESCRIBE ")');"

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
#ifdef DUK_CMDLINE_ALLOC_LOGGING
#include "duk_alloc_logging.h"
#endif
#ifdef DUK_CMDLINE_ALLOC_TORTURE
#include "duk_alloc_torture.h"
#endif
#ifdef DUK_CMDLINE_ALLOC_HYBRID
#include "duk_alloc_hybrid.h"
#endif
#include "duktape.h"

#ifdef DUK_CMDLINE_AJSHEAP
/* Defined in duk_cmdline_ajduk.c or alljoyn.js headers. */
void ajsheap_init(void);
void ajsheap_dump(void);
void ajsheap_register(duk_context *ctx);
void ajsheap_start_exec_timeout(void);
void ajsheap_clear_exec_timeout(void);
void *ajsheap_alloc_wrapped(void *udata, duk_size_t size);
void *ajsheap_realloc_wrapped(void *udata, void *ptr, duk_size_t size);
void ajsheap_free_wrapped(void *udata, void *ptr);
void *AJS_Alloc(void *udata, duk_size_t size);
void *AJS_Realloc(void *udata, void *ptr, duk_size_t size);
void AJS_Free(void *udata, void *ptr);
#endif

#ifdef DUK_CMDLINE_DEBUGGER_SUPPORT
#include "duk_trans_socket.h"
#endif

#define  MEM_LIMIT_NORMAL   (128*1024*1024)   /* 128 MB */
#define  MEM_LIMIT_HIGH     (2047*1024*1024)  /* ~2 GB */
#define  LINEBUF_SIZE       65536

static int interactive_mode = 0;

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
	fflush(stderr);
}
static void set_sigint_handler(void) {
	(void) signal(SIGINT, my_sighandler);
}
#endif  /* NO_SIGNAL */

static int get_stack_raw(duk_context *ctx) {
	if (!duk_is_object(ctx, -1)) {
		return 1;
	}
	if (!duk_has_prop_string(ctx, -1, "stack")) {
		return 1;
	}
	if (!duk_is_error(ctx, -1)) {
		/* Not an Error instance, don't read "stack". */
		return 1;
	}

	duk_get_prop_string(ctx, -1, "stack");  /* caller coerces */
	duk_remove(ctx, -2);
	return 1;
}

/* Print error to stderr and pop error. */
static void print_pop_error(duk_context *ctx, FILE *f) {
	/* Print error objects with a stack trace specially.
	 * Note that getting the stack trace may throw an error
	 * so this also needs to be safe call wrapped.
	 */
	(void) duk_safe_call(ctx, get_stack_raw, 1 /*nargs*/, 1 /*nrets*/);
	fprintf(f, "%s\n", duk_safe_to_string(ctx, -1));
	fflush(f);
	duk_pop(ctx);
}

static int wrapped_compile_execute(duk_context *ctx) {
	const char *src_data;
	duk_size_t src_len;
	int comp_flags;

	/* XXX: Here it'd be nice to get some stats for the compilation result
	 * when a suitable command line is given (e.g. code size, constant
	 * count, function count.  These are available internally but not through
	 * the public API.
	 */

	/* Use duk_compile_lstring_filename() variant which avoids interning
	 * the source code.  This only really matters for low memory environments.
	 */

	/* [ ... bytecode_filename src_data src_len filename ] */

	src_data = (const char *) duk_require_pointer(ctx, -3);
	src_len = (duk_size_t) duk_require_uint(ctx, -2);

	if (src_data != NULL && src_len >= 2 && src_data[0] == (char) 0xff) {
		/* Bytecode. */
		duk_push_lstring(ctx, src_data, src_len);
		duk_to_buffer(ctx, -1, NULL);
		duk_load_function(ctx);
	} else {
		/* Source code. */
		comp_flags = 0;
		duk_compile_lstring_filename(ctx, comp_flags, src_data, src_len);
	}

	/* [ ... bytecode_filename src_data src_len function ] */

	/* Optional bytecode dump. */
	if (duk_is_string(ctx, -4)) {
		FILE *f;
		void *bc_ptr;
		duk_size_t bc_len;
		size_t wrote;

		duk_dup_top(ctx);
		duk_dump_function(ctx);
		bc_ptr = duk_require_buffer(ctx, -1, &bc_len);
		f = fopen(duk_require_string(ctx, -5), "wb");
		if (!f) {
			duk_error(ctx, DUK_ERR_ERROR, "failed to open bytecode output file");
		}
		wrote = fwrite(bc_ptr, 1, (size_t) bc_len, f);  /* XXX: handle partial writes */
		(void) fclose(f);
		if (wrote != bc_len) {
			duk_error(ctx, DUK_ERR_ERROR, "failed to write all bytecode");
		}

		return 0;  /* duk_safe_call() cleans up */
	}

#if 0
	/* Manual test for bytecode dump/load cycle: dump and load before
	 * execution.  Enable manually, then run "make qecmatest" for a
	 * reasonably good coverage of different functions and programs.
	 */
	duk_dump_function(ctx);
	duk_load_function(ctx);
#endif

#if defined(DUK_CMDLINE_AJSHEAP)
	ajsheap_start_exec_timeout();
#endif

	duk_push_global_object(ctx);  /* 'this' binding */
	duk_call_method(ctx, 0);

#if defined(DUK_CMDLINE_AJSHEAP)
	ajsheap_clear_exec_timeout();
#endif

	if (interactive_mode) {
		/*
		 *  In interactive mode, write to stdout so output won't
		 *  interleave as easily.
		 *
		 *  NOTE: the ToString() coercion may fail in some cases;
		 *  for instance, if you evaluate:
		 *
		 *    ( {valueOf: function() {return {}},
		 *       toString: function() {return {}}});
		 *
		 *  The error is:
		 *
		 *    TypeError: failed to coerce with [[DefaultValue]]
		 *            duk_api.c:1420
		 *
		 *  These are handled now by the caller which also has stack
		 *  trace printing support.  User code can print out errors
		 *  safely using duk_safe_to_string().
		 */

		fprintf(stdout, "= %s\n", duk_to_string(ctx, -1));
		fflush(stdout);
	} else {
		/* In non-interactive mode, success results are not written at all.
		 * It is important that the result value is not string coerced,
		 * as the string coercion may cause an error in some cases.
		 */
	}

	return 0;  /* duk_safe_call() cleans up */
}

static int handle_fh(duk_context *ctx, FILE *f, const char *filename, const char *bytecode_filename) {
	char *buf = NULL;
	int len;
	size_t got;
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

	duk_push_string(ctx, bytecode_filename);
	duk_push_pointer(ctx, (void *) buf);
	duk_push_uint(ctx, (duk_uint_t) got);
	duk_push_string(ctx, filename);

	interactive_mode = 0;  /* global */

	rc = duk_safe_call(ctx, wrapped_compile_execute, 4 /*nargs*/, 1 /*nret*/);

#if defined(DUK_CMDLINE_AJSHEAP)
	ajsheap_clear_exec_timeout();
#endif

	free(buf);
	buf = NULL;

	if (rc != DUK_EXEC_SUCCESS) {
		print_pop_error(ctx, stderr);
		goto error;
	} else {
		duk_pop(ctx);
		retval = 0;
	}
	/* fall thru */

 cleanup:
	if (buf) {
		free(buf);
	}
	return retval;

 error:
	fprintf(stderr, "error in executing file %s\n", filename);
	fflush(stderr);
	goto cleanup;
}

static int handle_file(duk_context *ctx, const char *filename, const char *bytecode_filename) {
	FILE *f = NULL;
	int retval;

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "failed to open source file: %s\n", filename);
		fflush(stderr);
		goto error;
	}

	retval = handle_fh(ctx, f, filename, bytecode_filename);

	fclose(f);
	return retval;

 error:
	return -1;
}

static int handle_eval(duk_context *ctx, const char *code) {
	int rc;
	int retval = -1;

	duk_push_pointer(ctx, (void *) code);
	duk_push_uint(ctx, (duk_uint_t) strlen(code));
	duk_push_string(ctx, "eval");

	interactive_mode = 0;  /* global */

	rc = duk_safe_call(ctx, wrapped_compile_execute, 3 /*nargs*/, 1 /*nret*/);

#if defined(DUK_CMDLINE_AJSHEAP)
	ajsheap_clear_exec_timeout();
#endif

	if (rc != DUK_EXEC_SUCCESS) {
		print_pop_error(ctx, stderr);
	} else {
		duk_pop(ctx);
		retval = 0;
	}

	return retval;
}

#ifdef NO_READLINE
static int handle_interactive(duk_context *ctx) {
	const char *prompt = "duk> ";
	char *buffer = NULL;
	int retval = 0;
	int rc;
	int got_eof = 0;

	duk_eval_string(ctx, GREET_CODE(" [no readline]"));
	duk_pop(ctx);

	buffer = (char *) malloc(LINEBUF_SIZE);
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

		duk_push_pointer(ctx, (void *) buffer);
		duk_push_uint(ctx, (duk_uint_t) idx);
		duk_push_string(ctx, "input");

		interactive_mode = 1;  /* global */

		rc = duk_safe_call(ctx, wrapped_compile_execute, 3 /*nargs*/, 1 /*nret*/);

#if defined(DUK_CMDLINE_AJSHEAP)
		ajsheap_clear_exec_timeout();
#endif

		if (rc != DUK_EXEC_SUCCESS) {
			/* in interactive mode, write to stdout */
			print_pop_error(ctx, stdout);
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
static int handle_interactive(duk_context *ctx) {
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

		duk_push_pointer(ctx, (void *) buffer);
		duk_push_uint(ctx, (duk_uint_t) strlen(buffer));
		duk_push_string(ctx, "input");

		interactive_mode = 1;  /* global */

		rc = duk_safe_call(ctx, wrapped_compile_execute, 3 /*nargs*/, 1 /*nret*/);

#if defined(DUK_CMDLINE_AJSHEAP)
		ajsheap_clear_exec_timeout();
#endif

		if (buffer) {
			free(buffer);
			buffer = NULL;
		}

		if (rc != DUK_EXEC_SUCCESS) {
			/* in interactive mode, write to stdout */
			print_pop_error(ctx, stdout);
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

#ifdef DUK_CMDLINE_DEBUGGER_SUPPORT
static void debugger_detached(void *udata) {
	fprintf(stderr, "Debugger detached, udata: %p\n", (void *) udata);
	fflush(stderr);
}
#endif

#define  ALLOC_DEFAULT  0
#define  ALLOC_LOGGING  1
#define  ALLOC_TORTURE  2
#define  ALLOC_HYBRID   3
#define  ALLOC_AJSHEAP  4

static duk_context *create_duktape_heap(int alloc_provider, int debugger) {
	duk_context *ctx;

	ctx = NULL;
	if (!ctx && alloc_provider == ALLOC_LOGGING) {
#ifdef DUK_CMDLINE_ALLOC_LOGGING
		ctx = duk_create_heap(duk_alloc_logging,
		                      duk_realloc_logging,
		                      duk_free_logging,
		                      (void *) 0xdeadbeef,
		                      NULL);
#else
		fprintf(stderr, "Warning: option --alloc-logging ignored, no logging allocator support\n");
		fflush(stderr);
#endif
	}
	if (!ctx && alloc_provider == ALLOC_TORTURE) {
#ifdef DUK_CMDLINE_ALLOC_TORTURE
		ctx = duk_create_heap(duk_alloc_torture,
		                      duk_realloc_torture,
		                      duk_free_torture,
		                      (void *) 0xdeadbeef,
		                      NULL);
#else
		fprintf(stderr, "Warning: option --alloc-torture ignored, no torture allocator support\n");
		fflush(stderr);
#endif
	}
	if (!ctx && alloc_provider == ALLOC_HYBRID) {
#ifdef DUK_CMDLINE_ALLOC_HYBRID
		void *udata = duk_alloc_hybrid_init();
		if (!udata) {
			fprintf(stderr, "Failed to init hybrid allocator\n");
			fflush(stderr);
		} else {
			ctx = duk_create_heap(duk_alloc_hybrid,
			                      duk_realloc_hybrid,
			                      duk_free_hybrid,
			                      udata,
			                      NULL);
		}
#else
		fprintf(stderr, "Warning: option --alloc-hybrid ignored, no hybrid allocator support\n");
		fflush(stderr);
#endif
	}
	if (!ctx && alloc_provider == ALLOC_AJSHEAP) {
#ifdef DUK_CMDLINE_AJSHEAP
		ajsheap_init();

		ctx = duk_create_heap(
			ajsheap_log ? ajsheap_alloc_wrapped : AJS_Alloc,
			ajsheap_log ? ajsheap_realloc_wrapped : AJS_Realloc,
			ajsheap_log ? ajsheap_free_wrapped : AJS_Free,
			(void *) 0xdeadbeef,  /* heap_udata: ignored by AjsHeap, use as marker */
			NULL
		);                /* fatal_handler */
#else
		fprintf(stderr, "Warning: option --alloc-ajsheap ignored, no ajsheap allocator support\n");
		fflush(stderr);
#endif
	}
	if (!ctx && alloc_provider == ALLOC_DEFAULT) {
		ctx = duk_create_heap_default();
	}

	if (!ctx) {
		fprintf(stderr, "Failed to create Duktape heap\n");
		fflush(stderr);
		exit(-1);
	}

#ifdef DUK_CMDLINE_AJSHEAP
	if (alloc_provider == ALLOC_AJSHEAP) {
		fprintf(stdout, "Pool dump after heap creation\n");
		ajsheap_dump();
	}
#endif

#ifdef DUK_CMDLINE_AJSHEAP
	if (alloc_provider == ALLOC_AJSHEAP) {
		ajsheap_register(ctx);
	}
#endif

	if (debugger) {
#ifdef DUK_CMDLINE_DEBUGGER_SUPPORT
		fprintf(stderr, "Debugger enabled, create socket and wait for connection\n");
		fflush(stderr);
		duk_trans_socket_init();
		duk_trans_socket_waitconn();
		fprintf(stderr, "Debugger connected, call duk_debugger_attach() and then execute requested file(s)/eval\n");
		fflush(stderr);
		duk_debugger_attach(ctx,
		                    duk_trans_socket_read_cb,
		                    duk_trans_socket_write_cb,
		                    duk_trans_socket_peek_cb,
		                    duk_trans_socket_read_flush_cb,
		                    duk_trans_socket_write_flush_cb,
		                    debugger_detached,
		                    (void *) 0xbeef1234);
#else
		fprintf(stderr, "Warning: option --debugger ignored, no debugger support\n");
		fflush(stderr);
#endif
	}

#if 0
	/* Manual test for duk_debugger_cooperate() */
	{
		for (i = 0; i < 60; i++) {
			printf("cooperate: %d\n", i);
			usleep(1000000);
			duk_debugger_cooperate(ctx);
		}
	}
#endif

	return ctx;
}

static void destroy_duktape_heap(duk_context *ctx, int alloc_provider) {
	(void) alloc_provider;

#ifdef DUK_CMDLINE_AJSHEAP
	if (alloc_provider == ALLOC_AJSHEAP) {
		fprintf(stdout, "Pool dump before duk_destroy_heap(), before forced gc\n");
		ajsheap_dump();

		duk_gc(ctx, 0);

		fprintf(stdout, "Pool dump before duk_destroy_heap(), after forced gc\n");
		ajsheap_dump();
	}
#endif

	if (ctx) {
		duk_destroy_heap(ctx);
	}

#ifdef DUK_CMDLINE_AJSHEAP
	if (alloc_provider == ALLOC_AJSHEAP) {
		fprintf(stdout, "Pool dump after duk_destroy_heap() (should have zero allocs)\n");
		ajsheap_dump();
	}
#endif
}

int main(int argc, char *argv[]) {
	duk_context *ctx = NULL;
	int retval = 0;
	int have_files = 0;
	int have_eval = 0;
	int interactive = 0;
	int memlimit_high = 1;
	int alloc_provider = ALLOC_DEFAULT;
	int ajsheap_log = 0;
	int debugger = 0;
	int recreate_heap = 0;
	int verbose = 0;
	const char *compile_filename = NULL;
	int i;

#ifdef DUK_CMDLINE_AJSHEAP
	alloc_provider = ALLOC_AJSHEAP;
#endif
	(void) ajsheap_log;

	/*
	 *  Signal handling setup
	 */

#ifndef NO_SIGNAL
	set_sigint_handler();

	/* This is useful at the global level; libraries should avoid SIGPIPE though */
	/*signal(SIGPIPE, SIG_IGN);*/
#endif

	/*
	 *  Parse options
	 */

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!arg) {
			goto usage;
		}
		if (strcmp(arg, "--restrict-memory") == 0) {
			memlimit_high = 0;
		} else if (strcmp(arg, "-i") == 0) {
			interactive = 1;
		} else if (strcmp(arg, "-c") == 0) {
			if (i == argc - 1) {
				goto usage;
			}
			i++;
			compile_filename = argv[i];
		} else if (strcmp(arg, "-e") == 0) {
			have_eval = 1;
			if (i == argc - 1) {
				goto usage;
			}
			i++;  /* skip code */
		} else if (strcmp(arg, "--alloc-default") == 0) {
			alloc_provider = ALLOC_DEFAULT;
		} else if (strcmp(arg, "--alloc-logging") == 0) {
			alloc_provider = ALLOC_LOGGING;
		} else if (strcmp(arg, "--alloc-torture") == 0) {
			alloc_provider = ALLOC_TORTURE;
		} else if (strcmp(arg, "--alloc-hybrid") == 0) {
			alloc_provider = ALLOC_HYBRID;
		} else if (strcmp(arg, "--alloc-ajsheap") == 0) {
			alloc_provider = ALLOC_AJSHEAP;
		} else if (strcmp(arg, "--ajsheap-log") == 0) {
			ajsheap_log = 1;
		} else if (strcmp(arg, "--debugger") == 0) {
			debugger = 1;
		} else if (strcmp(arg, "--recreate-heap") == 0) {
			recreate_heap = 1;
		} else if (strcmp(arg, "--verbose") == 0) {
			verbose = 1;
		} else if (strlen(arg) >= 1 && arg[0] == '-') {
			goto usage;
		} else {
			have_files = 1;
		}
	}
	if (!have_files && !have_eval) {
		interactive = 1;
	}

	/*
	 *  Memory limit
	 */

#ifndef NO_RLIMIT
	set_resource_limits(memlimit_high ? MEM_LIMIT_HIGH : MEM_LIMIT_NORMAL);
#else
	if (memlimit_high == 0) {
		fprintf(stderr, "Warning: option --restrict-memory ignored, no rlimit support\n");
		fflush(stderr);
	}
#endif

	/*
	 *  Create heap
	 */

	ctx = create_duktape_heap(alloc_provider, debugger);

	/*
	 *  Execute any argument file(s)
	 */

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!arg) {
			continue;
		} else if (strlen(arg) == 2 && strcmp(arg, "-e") == 0) {
			/* Here we know the eval arg exists but check anyway */
			if (i == argc - 1) {
				retval = 1;
				goto cleanup;
			}
			if (handle_eval(ctx, argv[i + 1]) != 0) {
				retval = 1;
				goto cleanup;
			}
			i++;  /* skip code */
			continue;
		} else if (strlen(arg) == 2 && strcmp(arg, "-c") == 0) {
			i++;  /* skip filename */
			continue;
		} else if (strlen(arg) >= 1 && arg[0] == '-') {
			continue;
		}

		if (verbose) {
			fprintf(stderr, "*** Executing file: %s\n", arg);
			fflush(stderr);
		}

		if (handle_file(ctx, arg, compile_filename) != 0) {
			retval = 1;
			goto cleanup;
		}

		if (recreate_heap) {
			if (verbose) {
				fprintf(stderr, "*** Recreating heap...\n");
				fflush(stderr);
			}

			destroy_duktape_heap(ctx, alloc_provider);
			ctx = create_duktape_heap(alloc_provider, debugger);
		}
	}

	/*
	 *  Enter interactive mode if options indicate it
	 */

	if (interactive) {
		if (handle_interactive(ctx) != 0) {
			retval = 1;
			goto cleanup;
		}
	}

	/*
	 *  Cleanup and exit
	 */

 cleanup:
	if (interactive) {
		fprintf(stderr, "Cleaning up...\n");
		fflush(stderr);
	}

	if (ctx) {
		destroy_duktape_heap(ctx, alloc_provider);
	}
	ctx = NULL;

	return retval;

	/*
	 *  Usage
	 */

 usage:
	fprintf(stderr, "Usage: duk [options] [<filenames>]\n"
	                "\n"
	                "   -i                 enter interactive mode after executing argument file(s) / eval code\n"
	                "   -e CODE            evaluate code\n"
			"   -c FILE            compile into bytecode (use with only one file argument)\n"
			"   --verbose          verbose messages to stderr\n"
	                "   --restrict-memory  use lower memory limit (used by test runner)\n"
	                "   --alloc-default    use Duktape default allocator\n"
#ifdef DUK_CMDLINE_ALLOC_LOGGING
	                "   --alloc-logging    use logging allocator (writes to /tmp)\n"
#endif
#ifdef DUK_CMDLINE_ALLOC_TORTURE
	                "   --alloc-torture    use torture allocator\n"
#endif
#ifdef DUK_CMDLINE_ALLOC_HYBRID
	                "   --alloc-hybrid     use hybrid allocator\n"
#endif
#ifdef DUK_CMDLINE_AJSHEAP
	                "   --alloc-ajsheap    use ajsheap allocator (enabled by default with 'ajduk')\n"
	                "   --ajsheap-log      write alloc log to /tmp/ajduk-alloc-log.txt\n"
#endif
#ifdef DUK_CMDLINE_DEBUGGER_SUPPORT
			"   --debugger         start example debugger\n"
#endif
			"   --recreate-heap    recreate heap after every file\n"
	                "\n"
	                "If <filename> is omitted, interactive mode is started automatically.\n");
	fflush(stderr);
	exit(1);
}
