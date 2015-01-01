/*
 *  Function built-ins
 */

#include "duk_internal.h"

DUK_INTERNAL duk_ret_t duk_bi_function_constructor(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_sourcecode;
	duk_idx_t nargs;
	duk_idx_t i;
	duk_small_uint_t comp_flags;
	duk_hcompiledfunction *func;
	duk_hobject *outer_lex_env;
	duk_hobject *outer_var_env;

	/* normal and constructor calls have identical semantics */

	nargs = duk_get_top(ctx);
	for (i = 0; i < nargs; i++) {
		duk_to_string(ctx, i);
	}

	if (nargs == 0) {
		duk_push_string(ctx, "");
		duk_push_string(ctx, "");
	} else if (nargs == 1) {
		/* XXX: cover this with the generic >1 case? */
		duk_push_string(ctx, "");
	} else {
		duk_insert(ctx, 0);   /* [ arg1 ... argN-1 body] -> [body arg1 ... argN-1] */
		duk_push_string(ctx, ",");
		duk_insert(ctx, 1);
		duk_join(ctx, nargs - 1);
	}

	/* [ body formals ], formals is comma separated list that needs to be parsed */

	DUK_ASSERT_TOP(ctx, 2);

	/* XXX: this placeholder is not always correct, but use for now.
	 * It will fail in corner cases; see test-dev-func-cons-args.js.
	 */
	duk_push_string(ctx, "function(");
	duk_dup(ctx, 1);
	duk_push_string(ctx, "){");
	duk_dup(ctx, 0);
	duk_push_string(ctx, "}");
	duk_concat(ctx, 5);

	/* [ body formals source ] */

	DUK_ASSERT_TOP(ctx, 3);

	/* strictness is not inherited, intentional */
	comp_flags = DUK_JS_COMPILE_FLAG_FUNCEXPR;

	duk_push_hstring_stridx(ctx, DUK_STRIDX_COMPILE);  /* XXX: copy from caller? */  /* XXX: ignored now */
	h_sourcecode = duk_require_hstring(ctx, -2);
	duk_js_compile(thr,
	               (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h_sourcecode),
	               (duk_size_t) DUK_HSTRING_GET_BYTELEN(h_sourcecode),
	               comp_flags);
	func = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) func));

	/* [ body formals source template ] */

	/* only outer_lex_env matters, as functions always get a new
	 * variable declaration environment.
	 */

	outer_lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	outer_var_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];

	duk_js_push_closure(thr, func, outer_var_env, outer_lex_env);

	/* [ body formals source template closure ] */

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_function_prototype(duk_context *ctx) {
	/* ignore arguments, return undefined (E5 Section 15.3.4) */
	DUK_UNREF(ctx);
	return 0;
}

DUK_INTERNAL duk_ret_t duk_bi_function_prototype_to_string(duk_context *ctx) {
	duk_tval *tv;

	/*
	 *  E5 Section 15.3.4.2 places few requirements on the output of
	 *  this function:
	 *
	 *    - The result is an implementation dependent representation
	 *      of the function; in particular
	 *
	 *    - The result must follow the syntax of a FunctionDeclaration.
	 *      In particular, the function must have a name (even in the
	 *      case of an anonymous function or a function with an empty
	 *      name).
	 *
	 *    - Note in particular that the output does NOT need to compile
	 *      into anything useful.
	 */


	/* XXX: faster internal way to get this */
	duk_push_this(ctx);
	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_IS_OBJECT(tv)) {
		duk_hobject *obj = DUK_TVAL_GET_OBJECT(tv);
		const char *func_name = DUK_STR_ANON;

		/* XXX: rework, it would be nice to avoid C formatting functions to
		 * ensure there are no Unicode issues.
		 */

		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_NAME);
		if (!duk_is_undefined(ctx, -1)) {
			func_name = duk_to_string(ctx, -1);
			DUK_ASSERT(func_name != NULL);

			if (func_name[0] == (char) 0) {
				func_name = DUK_STR_ANON;
			}
		}

		if (DUK_HOBJECT_HAS_COMPILEDFUNCTION(obj)) {
			/* XXX: actual source, if available */
			duk_push_sprintf(ctx, "function %s() {/* ecmascript */}", (const char *) func_name);
		} else if (DUK_HOBJECT_HAS_NATIVEFUNCTION(obj)) {
			duk_push_sprintf(ctx, "function %s() {/* native */}", (const char *) func_name);
		} else if (DUK_HOBJECT_HAS_BOUND(obj)) {
			duk_push_sprintf(ctx, "function %s() {/* bound */}", (const char *) func_name);
		} else {
			goto type_error;
		}
	} else if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
		duk_push_lightfunc_tostring(ctx, tv);
	} else {
		goto type_error;
	}

	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_function_prototype_apply(duk_context *ctx) {
	duk_idx_t len;
	duk_idx_t i;

	DUK_ASSERT_TOP(ctx, 2);  /* not a vararg function */

	duk_push_this(ctx);
	if (!duk_is_callable(ctx, -1)) {
		DUK_DDD(DUK_DDDPRINT("func is not callable"));
		goto type_error;
	}
	duk_insert(ctx, 0);
	DUK_ASSERT_TOP(ctx, 3);

	DUK_DDD(DUK_DDDPRINT("func=%!iT, thisArg=%!iT, argArray=%!iT",
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1),
	                     (duk_tval *) duk_get_tval(ctx, 2)));

	/* [ func thisArg argArray ] */

	if (duk_is_null_or_undefined(ctx, 2)) {
		DUK_DDD(DUK_DDDPRINT("argArray is null/undefined, no args"));
		len = 0;
	} else if (!duk_is_object(ctx, 2)) {
		goto type_error;
	} else {
		DUK_DDD(DUK_DDDPRINT("argArray is an object"));

		/* XXX: make this an internal helper */
		duk_get_prop_stridx(ctx, 2, DUK_STRIDX_LENGTH);
		len = (duk_idx_t) duk_to_uint32(ctx, -1);  /* ToUint32() coercion required */
		duk_pop(ctx);

		duk_require_stack(ctx, len);

		DUK_DDD(DUK_DDDPRINT("argArray length is %ld", (long) len));
		for (i = 0; i < len; i++) {
			duk_get_prop_index(ctx, 2, i);
		}
	}
	duk_remove(ctx, 2);
	DUK_ASSERT_TOP(ctx, 2 + len);

	/* [ func thisArg arg1 ... argN ] */

	DUK_DDD(DUK_DDDPRINT("apply, func=%!iT, thisArg=%!iT, len=%ld",
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1),
	                     (long) len));
	duk_call_method(ctx, len);
	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_function_prototype_call(duk_context *ctx) {
	duk_idx_t nargs;

	/* Step 1 is not necessary because duk_call_method() will take
	 * care of it.
	 */

	/* vararg function, thisArg needs special handling */
	nargs = duk_get_top(ctx);  /* = 1 + arg count */
	if (nargs == 0) {
		duk_push_undefined(ctx);
		nargs++;
	}
	DUK_ASSERT(nargs >= 1);

	/* [ thisArg arg1 ... argN ] */

	duk_push_this(ctx);  /* 'func' in the algorithm */
	duk_insert(ctx, 0);

	/* [ func thisArg arg1 ... argN ] */

	DUK_DDD(DUK_DDDPRINT("func=%!iT, thisArg=%!iT, argcount=%ld, top=%ld",
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1),
	                     (long) (nargs - 1),
	                     (long) duk_get_top(ctx)));
	duk_call_method(ctx, nargs - 1);
	return 1;
}

/* XXX: the implementation now assumes "chained" bound functions,
 * whereas "collapsed" bound functions (where there is ever only
 * one bound function which directly points to a non-bound, final
 * function) would require a "collapsing" implementation which
 * merges argument lists etc here.
 */
DUK_INTERNAL duk_ret_t duk_bi_function_prototype_bind(duk_context *ctx) {
	duk_hobject *h_bound;
	duk_hobject *h_target;
	duk_idx_t nargs;
	duk_idx_t i;

	/* vararg function, careful arg handling (e.g. thisArg may not be present) */
	nargs = duk_get_top(ctx);  /* = 1 + arg count */
	if (nargs == 0) {
		duk_push_undefined(ctx);
		nargs++;
	}
	DUK_ASSERT(nargs >= 1);

	duk_push_this(ctx);
	if (!duk_is_callable(ctx, -1)) {
		DUK_DDD(DUK_DDDPRINT("func is not callable"));
		goto type_error;
	}

	/* [ thisArg arg1 ... argN func ]  (thisArg+args == nargs total) */
	DUK_ASSERT_TOP(ctx, nargs + 1);

	/* create bound function object */
	duk_push_object_helper(ctx,
	                       DUK_HOBJECT_FLAG_EXTENSIBLE |
	                       DUK_HOBJECT_FLAG_BOUND |
	                       DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION),
	                       DUK_BIDX_FUNCTION_PROTOTYPE);
	h_bound = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h_bound != NULL);

	/* [ thisArg arg1 ... argN func boundFunc ] */
	duk_dup(ctx, -2);  /* func */
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_TARGET, DUK_PROPDESC_FLAGS_NONE);

	duk_dup(ctx, 0);   /* thisArg */
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_THIS, DUK_PROPDESC_FLAGS_NONE);

	duk_push_array(ctx);

	/* [ thisArg arg1 ... argN func boundFunc argArray ] */

	for (i = 0; i < nargs - 1; i++) {
		duk_dup(ctx, 1 + i);
		duk_put_prop_index(ctx, -2, i);
	}
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_ARGS, DUK_PROPDESC_FLAGS_NONE);

	/* [ thisArg arg1 ... argN func boundFunc ] */

	/* bound function 'length' property is interesting */
	h_target = duk_get_hobject(ctx, -2);
	if (h_target == NULL ||  /* lightfunc */
	    DUK_HOBJECT_GET_CLASS_NUMBER(h_target) == DUK_HOBJECT_CLASS_FUNCTION) {
		/* For lightfuncs, simply read the virtual property. */
		duk_int_t tmp;
		duk_get_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH);
		tmp = duk_to_int(ctx, -1) - (nargs - 1);  /* step 15.a */
		duk_pop(ctx);
		duk_push_int(ctx, (tmp < 0 ? 0 : tmp));
	} else {
		duk_push_int(ctx, 0);
	}
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_NONE);  /* attrs in E5 Section 15.3.5.1 */

	/* caller and arguments must use the same thrower, [[ThrowTypeError]] */
	duk_xdef_prop_stridx_thrower(ctx, -1, DUK_STRIDX_CALLER, DUK_PROPDESC_FLAGS_NONE);
	duk_xdef_prop_stridx_thrower(ctx, -1, DUK_STRIDX_LC_ARGUMENTS, DUK_PROPDESC_FLAGS_NONE);

	/* these non-standard properties are copied for convenience */
	/* XXX: 'copy properties' API call? */
	duk_get_prop_stridx(ctx, -2, DUK_STRIDX_NAME);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_WC);
	duk_get_prop_stridx(ctx, -2, DUK_STRIDX_FILE_NAME);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_FILE_NAME, DUK_PROPDESC_FLAGS_WC);

	/* The 'strict' flag is copied to get the special [[Get]] of E5.1
	 * Section 15.3.5.4 to apply when a 'caller' value is a strict bound
	 * function.  Not sure if this is correct, because the specification
	 * is a bit ambiguous on this point but it would make sense.
	 */
	if (h_target == NULL) {
		/* Lightfuncs are always strict. */
		DUK_HOBJECT_SET_STRICT(h_bound);
	} else if (DUK_HOBJECT_HAS_STRICT(h_target)) {
		DUK_HOBJECT_SET_STRICT(h_bound);
	}
	DUK_DDD(DUK_DDDPRINT("created bound function: %!iT", (duk_tval *) duk_get_tval(ctx, -1)));

	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}
