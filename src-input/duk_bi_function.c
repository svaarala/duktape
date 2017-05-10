/*
 *  Function built-ins
 */

#include "duk_internal.h"

/* Needed even when Function built-in is disabled. */
DUK_INTERNAL duk_ret_t duk_bi_function_prototype(duk_context *ctx) {
	/* ignore arguments, return undefined (E5 Section 15.3.4) */
	DUK_UNREF(ctx);
	return 0;
}

#if defined(DUK_USE_FUNCTION_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_function_constructor(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_sourcecode;
	duk_idx_t nargs;
	duk_idx_t i;
	duk_small_uint_t comp_flags;
	duk_hcompfunc *func;
	duk_hobject *outer_lex_env;
	duk_hobject *outer_var_env;

	/* normal and constructor calls have identical semantics */

	nargs = duk_get_top(ctx);
	for (i = 0; i < nargs; i++) {
		duk_to_string(ctx, i);  /* Rejects Symbols during coercion. */
	}

	if (nargs == 0) {
		duk_push_hstring_empty(ctx);
		duk_push_hstring_empty(ctx);
	} else if (nargs == 1) {
		/* XXX: cover this with the generic >1 case? */
		duk_push_hstring_empty(ctx);
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
	duk_dup_1(ctx);
	duk_push_string(ctx, "){");
	duk_dup_0(ctx);
	duk_push_string(ctx, "}");
	duk_concat(ctx, 5);

	/* [ body formals source ] */

	DUK_ASSERT_TOP(ctx, 3);

	/* strictness is not inherited, intentional */
	comp_flags = DUK_COMPILE_FUNCEXPR;

	duk_push_hstring_stridx(ctx, DUK_STRIDX_COMPILE);  /* XXX: copy from caller? */  /* XXX: ignored now */
	h_sourcecode = duk_require_hstring(ctx, -2);  /* no symbol check needed; -2 is concat'd code */
	duk_js_compile(thr,
	               (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h_sourcecode),
	               (duk_size_t) DUK_HSTRING_GET_BYTELEN(h_sourcecode),
	               comp_flags);

	/* Force .name to 'anonymous' (ES2015). */
	duk_push_string(ctx, "anonymous");
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_C);

	func = (duk_hcompfunc *) duk_known_hobject(ctx, -1);
	DUK_ASSERT(DUK_HOBJECT_IS_COMPFUNC((duk_hobject *) func));
	DUK_ASSERT(DUK_HOBJECT_HAS_CONSTRUCTABLE((duk_hobject *) func));

	/* [ body formals source template ] */

	/* only outer_lex_env matters, as functions always get a new
	 * variable declaration environment.
	 */

	outer_lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	outer_var_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];

	duk_js_push_closure(thr, func, outer_var_env, outer_lex_env, 1 /*add_auto_proto*/);

	/* [ body formals source template closure ] */

	return 1;
}
#endif  /* DUK_USE_FUNCTION_BUILTIN */

#if defined(DUK_USE_FUNCTION_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_function_prototype_to_string(duk_context *ctx) {
	duk_tval *tv;

	/*
	 *  E5 Section 15.3.4.2 places few requirements on the output of
	 *  this function: the result is implementation dependent, must
	 *  follow FunctionDeclaration syntax (in particular, must have a
	 *  name even for anonymous functions or functions with empty name).
	 *  The output does NOT need to compile into anything useful.
	 *
	 *  E6 Section 19.2.3.5 changes the requirements completely: the
	 *  result must either eval() to a functionally equivalent object
	 *  OR eval() to a SyntaxError.
	 *
	 *  We opt for the SyntaxError approach for now, with a syntax that
	 *  mimics V8's native function syntax:
	 *
	 *      'function cos() { [native code] }'
	 *
	 *  but extended with [ecmascript code], [bound code], and
	 *  [lightfunc code].
	 */

	duk_push_this(ctx);
	tv = DUK_GET_TVAL_NEGIDX(ctx, -1);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_IS_OBJECT(tv)) {
		duk_hobject *obj = DUK_TVAL_GET_OBJECT(tv);
		const char *func_name;

		/* Function name: missing/undefined is mapped to empty string,
		 * otherwise coerce to string.  No handling for invalid identifier
		 * characters or e.g. '{' in the function name.  This doesn't
		 * really matter as long as a SyntaxError results.  Technically
		 * if the name contained a suitable prefix followed by '//' it
		 * might cause the result to parse without error.
		 */
		duk_get_prop_stridx_short(ctx, -1, DUK_STRIDX_NAME);
		if (duk_is_undefined(ctx, -1)) {
			func_name = "";
		} else {
			func_name = duk_to_string(ctx, -1);
			DUK_ASSERT(func_name != NULL);
		}

		if (DUK_HOBJECT_IS_COMPFUNC(obj)) {
			duk_push_sprintf(ctx, "function %s() { [ecmascript code] }", (const char *) func_name);
		} else if (DUK_HOBJECT_IS_NATFUNC(obj)) {
			duk_push_sprintf(ctx, "function %s() { [native code] }", (const char *) func_name);
		} else if (DUK_HOBJECT_IS_BOUNDFUNC(obj)) {
			duk_push_sprintf(ctx, "function %s() { [bound code] }", (const char *) func_name);
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
	DUK_DCERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
}
#endif

#if defined(DUK_USE_FUNCTION_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_function_prototype_apply(duk_context *ctx) {
	/*
	 *  magic = 0: Function.prototype.apply()
	 *  magic = 1: Reflect.apply()
	 *  magic = 2: Reflect.construct()
	 */

	duk_idx_t idx_args;
	duk_idx_t len;
	duk_idx_t i;
	duk_int_t magic;
	duk_idx_t nargs;
	duk_uint_t mask;

	magic = duk_get_current_magic(ctx);
	switch (magic) {
	case 0:  /* Function.prototype.apply() */
		DUK_ASSERT_TOP(ctx, 2);  /* not a vararg function */
		duk_push_this(ctx);
		duk_insert(ctx, 0);
		/* Fall through intentionally for shared handling. */
	case 1:  /* Reflect.apply(); Function.prototype.apply() after 'this' fixup. */
		DUK_ASSERT_TOP(ctx, 3);  /* not a vararg function */
		idx_args = 2;
		duk_require_callable(ctx, 0);
		break;
	default:  /* Reflect.construct() */
		DUK_ASSERT(magic == 2);
		duk_require_constructable(ctx, 0);
		nargs = duk_get_top(ctx);
		if (nargs < 2) {
			DUK_DCERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
		}
		if (nargs >= 3 && !duk_strict_equals(ctx, 0, 2)) {
			/* XXX: [[Construct]] newTarget currently unsupported */
			DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
		}
		duk_set_top(ctx, 2);  /* chop off extra arguments: [ constructor argArray ] */
		idx_args = 1;
		break;
	}

	if (magic != 2) {
		DUK_DDD(DUK_DDDPRINT("func=%!iT, thisArg=%!iT, argArray=%!iT",
							 (duk_tval *) duk_get_tval(ctx, 0),
							 (duk_tval *) duk_get_tval(ctx, 1),
							 (duk_tval *) duk_get_tval(ctx, 2)));
	} else {
		/* thisArg is not applicable for Reflect.construct(). */
		DUK_DDD(DUK_DDDPRINT("func=%!iT, argArray=%!iT",
							 (duk_tval *) duk_get_tval(ctx, 0),
							 (duk_tval *) duk_get_tval(ctx, 1)));
	}

	/* [ func thisArg? argArray ] */

	mask = duk_get_type_mask(ctx, idx_args);
	if (mask & (DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_UNDEFINED)) {
		DUK_DDD(DUK_DDDPRINT("argArray is null/undefined, no args"));
		len = 0;
	} else if (mask & DUK_TYPE_MASK_OBJECT) {
		DUK_DDD(DUK_DDDPRINT("argArray is an object"));

		/* XXX: make this an internal helper */
		DUK_ASSERT(idx_args >= 0 && idx_args <= 0x7fffL);  /* short variants would work, but avoid shifting */
		duk_get_prop_stridx(ctx, idx_args, DUK_STRIDX_LENGTH);
		len = (duk_idx_t) duk_to_uint32(ctx, -1);  /* ToUint32() coercion required */
		duk_pop(ctx);

		duk_require_stack(ctx, len);

		DUK_DDD(DUK_DDDPRINT("argArray length is %ld", (long) len));
		for (i = 0; i < len; i++) {
			duk_get_prop_index(ctx, idx_args, i);
		}
	} else {
		goto type_error;
	}
	duk_remove(ctx, idx_args);
	DUK_ASSERT_TOP(ctx, idx_args + len);

	/* [ func thisArg? arg1 ... argN ] */

	if (magic != 2) {
		/* Function.prototype.apply() or Reflect.apply() */
		DUK_DDD(DUK_DDDPRINT("apply, func=%!iT, thisArg=%!iT, len=%ld",
							 (duk_tval *) duk_get_tval(ctx, 0),
							 (duk_tval *) duk_get_tval(ctx, 1),
							 (long) len));
		duk_call_method(ctx, len);
	} else {
		/* Reflect.construct() */
		DUK_DDD(DUK_DDDPRINT("construct, func=%!iT, len=%ld",
							 (duk_tval *) duk_get_tval(ctx, 0),
							 (long) len));
		duk_new(ctx, len);
	}
	return 1;

 type_error:
	DUK_DCERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
}
#endif

#if defined(DUK_USE_FUNCTION_BUILTIN)
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
#endif  /* DUK_USE_FUNCTION_BUILTIN */

#if defined(DUK_USE_FUNCTION_BUILTIN)
/* XXX: the implementation now assumes "chained" bound functions,
 * whereas "collapsed" bound functions (where there is ever only
 * one bound function which directly points to a non-bound, final
 * function) would require a "collapsing" implementation which
 * merges argument lists etc here.
 */
DUK_INTERNAL duk_ret_t duk_bi_function_prototype_bind(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
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
	duk_require_callable(ctx, -1);

	/* [ thisArg arg1 ... argN func ]  (thisArg+args == nargs total) */
	DUK_ASSERT_TOP(ctx, nargs + 1);

	/* create bound function object */
	h_bound = duk_push_object_helper(ctx,
	                                 DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                 DUK_HOBJECT_FLAG_FASTREFS |
	                                 DUK_HOBJECT_FLAG_BOUNDFUNC |
	                                 DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	                                 DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION),
	                                 DUK_BIDX_FUNCTION_PROTOTYPE);
	DUK_ASSERT(h_bound != NULL);

	/* [ thisArg arg1 ... argN func boundFunc ] */
	duk_dup_m2(ctx);  /* func */
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_INT_TARGET, DUK_PROPDESC_FLAGS_NONE);

	duk_dup_0(ctx);   /* thisArg */
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_INT_THIS, DUK_PROPDESC_FLAGS_NONE);

	duk_push_array(ctx);

	/* [ thisArg arg1 ... argN func boundFunc argArray ] */

	for (i = 0; i < nargs - 1; i++) {
		duk_dup(ctx, 1 + i);
		duk_put_prop_index(ctx, -2, i);
	}
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_INT_ARGS, DUK_PROPDESC_FLAGS_NONE);

	/* [ thisArg arg1 ... argN func boundFunc ] */

	h_target = duk_get_hobject(ctx, -2);

	/* internal prototype must be copied from the target */
	if (h_target != NULL) {
		/* For lightfuncs Function.prototype is used and is already in place. */
		DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h_bound, DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h_target));
	}

	/* bound function 'length' property is interesting */
	if (h_target == NULL ||  /* lightfunc */
	    DUK_HOBJECT_GET_CLASS_NUMBER(h_target) == DUK_HOBJECT_CLASS_FUNCTION) {
		/* For lightfuncs, simply read the virtual property. */
		duk_int_t tmp;
		duk_get_prop_stridx_short(ctx, -2, DUK_STRIDX_LENGTH);
		tmp = duk_to_int(ctx, -1) - (nargs - 1);  /* step 15.a */
		duk_pop(ctx);
		duk_push_int(ctx, (tmp < 0 ? 0 : tmp));
	} else {
		duk_push_int(ctx, 0);
	}
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_C);  /* attrs in E6 Section 9.2.4 */

	/* caller and arguments must use the same thrower, [[ThrowTypeError]] */
	duk_xdef_prop_stridx_thrower(ctx, -1, DUK_STRIDX_CALLER);
	duk_xdef_prop_stridx_thrower(ctx, -1, DUK_STRIDX_LC_ARGUMENTS);

	/* XXX: 'copy properties' API call? */
#if defined(DUK_USE_FUNC_NAME_PROPERTY)
	duk_push_string(ctx, "bound ");  /* ES2015 19.2.3.2. */
	duk_get_prop_stridx_short(ctx, -3, DUK_STRIDX_NAME);
	if (!duk_is_string_notsymbol(ctx, -1)) {
		/* ES2015 has requirement to check that .name of target is a string
		 * (also must check for Symbol); if not, targetName should be the
		 * empty string.  ES2015 19.2.3.2.
		 */
		duk_pop(ctx);
		duk_push_hstring_empty(ctx);
	}
	duk_concat(ctx, 2);
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_C);
#endif
#if defined(DUK_USE_FUNC_FILENAME_PROPERTY)
	duk_get_prop_stridx_short(ctx, -2, DUK_STRIDX_FILE_NAME);
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_FILE_NAME, DUK_PROPDESC_FLAGS_C);
#endif

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
}
#endif  /* DUK_USE_FUNCTION_BUILTIN */
