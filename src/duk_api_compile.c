/*
 *  Compilation and evaluation
 */

#include "duk_internal.h"

typedef struct duk__compile_raw_args duk__compile_raw_args;
struct duk__compile_raw_args {
	duk_size_t src_length;  /* should be first on 64-bit platforms */
	const duk_uint8_t *src_buffer;
	duk_uint_t flags;
};

/* Eval is just a wrapper now. */
DUK_EXTERNAL duk_int_t duk_eval_raw(duk_context *ctx, const char *src_buffer, duk_size_t src_length, duk_uint_t flags) {
	duk_uint_t comp_flags;
	duk_int_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: strictness is *not* inherited from the current Duktape/C.
	 * This would be confusing because the current strictness state
	 * depends on whether we're running inside a Duktape/C activation
	 * (= strict mode) or outside of any activation (= non-strict mode).
	 * See tests/api/test-eval-strictness.c for more discussion.
	 */

	/* [ ... source? filename ] (depends on flags) */

	comp_flags = flags;
	comp_flags |= DUK_COMPILE_EVAL;
	rc = duk_compile_raw(ctx, src_buffer, src_length, comp_flags);  /* may be safe, or non-safe depending on flags */

	/* [ ... closure/error ] */

	if (rc != DUK_EXEC_SUCCESS) {
		rc = DUK_EXEC_ERROR;
		goto got_rc;
	}

	duk_push_global_object(ctx);  /* explicit 'this' binding, see GH-164 */

	if (flags & DUK_COMPILE_SAFE) {
		rc = duk_pcall_method(ctx, 0);
	} else {
		duk_call_method(ctx, 0);
		rc = DUK_EXEC_SUCCESS;
	}

	/* [ ... result/error ] */

 got_rc:
	if (flags & DUK_COMPILE_NORESULT) {
		duk_pop(ctx);
	}

	return rc;
}

/* Helper which can be called both directly and with duk_safe_call(). */
DUK_LOCAL duk_ret_t duk__do_compile(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk__compile_raw_args *comp_args;
	duk_uint_t flags;
	duk_small_uint_t comp_flags;
	duk_hcompiledfunction *h_templ;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: strictness is not inherited from the current Duktape/C
	 * context.  Otherwise it would not be possible to compile
	 * non-strict code inside a Duktape/C activation (which is
	 * always strict now).  See tests/api/test-eval-strictness.c
	 * for discussion.
	 */

	/* [ ... source? filename &comp_args ] (depends on flags) */

	comp_args = (duk__compile_raw_args *) duk_require_pointer(ctx, -1);
	flags = comp_args->flags;
	duk_pop(ctx);

	/* [ ... source? filename ] */

	if (!comp_args->src_buffer) {
		duk_hstring *h_sourcecode;

		h_sourcecode = duk_get_hstring(ctx, -2);
		if ((flags & DUK_COMPILE_NOSOURCE) ||  /* args incorrect */
		    (h_sourcecode == NULL)) {          /* e.g. duk_push_string_file_raw() pushed undefined */
			/* XXX: when this error is caused by a nonexistent
			 * file given to duk_peval_file() or similar, the
			 * error message is not the best possible.
			 */
			DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_NO_SOURCECODE);
		}
		DUK_ASSERT(h_sourcecode != NULL);
		comp_args->src_buffer = (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h_sourcecode);
		comp_args->src_length = (duk_size_t) DUK_HSTRING_GET_BYTELEN(h_sourcecode);
	}
	DUK_ASSERT(comp_args->src_buffer != NULL);

	/* XXX: unnecessary translation of flags */
	comp_flags = 0;
	if (flags & DUK_COMPILE_EVAL) {
		comp_flags |= DUK_JS_COMPILE_FLAG_EVAL;
	}
	if (flags & DUK_COMPILE_FUNCTION) {
		comp_flags |= DUK_JS_COMPILE_FLAG_EVAL |
		              DUK_JS_COMPILE_FLAG_FUNCEXPR;
	}
	if (flags & DUK_COMPILE_STRICT) {
		comp_flags |= DUK_JS_COMPILE_FLAG_STRICT;
	}

	/* [ ... source? filename ] */

	duk_js_compile(thr, comp_args->src_buffer, comp_args->src_length, comp_flags);

	/* [ ... source? func_template ] */

	if (flags & DUK_COMPILE_NOSOURCE) {
		;
	} else {
		duk_remove(ctx, -2);
	}

	/* [ ... func_template ] */

	h_templ = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
	DUK_ASSERT(h_templ != NULL);
	duk_js_push_closure(thr,
	                   h_templ,
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV],
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV]);
	duk_remove(ctx, -2);   /* -> [ ... closure ] */

	/* [ ... closure ] */

	return 1;
}

DUK_EXTERNAL duk_int_t duk_compile_raw(duk_context *ctx, const char *src_buffer, duk_size_t src_length, duk_uint_t flags) {
	duk__compile_raw_args comp_args_alloc;
	duk__compile_raw_args *comp_args = &comp_args_alloc;

	DUK_ASSERT_CTX_VALID(ctx);

	if ((flags & DUK_COMPILE_STRLEN) && (src_buffer != NULL)) {
		/* String length is computed here to avoid multiple evaluation
		 * of a macro argument in the calling side.
		 */
		src_length = DUK_STRLEN(src_buffer);
	}

	comp_args->src_buffer = (const duk_uint8_t *) src_buffer;
	comp_args->src_length = src_length;
	comp_args->flags = flags;
	duk_push_pointer(ctx, (void *) comp_args);

	/* [ ... source? filename &comp_args ] (depends on flags) */

	if (flags & DUK_COMPILE_SAFE) {
		duk_int_t rc;
		duk_int_t nargs;
		duk_int_t nrets = 1;

		/* Arguments are either: [ filename &comp_args ] or [ source filename &comp_args ] */
		nargs = (flags & DUK_COMPILE_NOSOURCE) ? 2 : 3;
		rc = duk_safe_call(ctx, duk__do_compile, nargs, nrets);

		/* [ ... closure ] */
		return rc;
	}

	(void) duk__do_compile(ctx);

	/* [ ... closure ] */
	return DUK_EXEC_SUCCESS;
}
