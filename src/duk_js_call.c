/*
 *  Call handling.
 *
 *  The main work horse functions are:
 *    - duk_handle_call(): call to a C/Ecmascript functions
 *    - duk_handle_safe_call(): make a protected C call within current activation
 *    - duk_handle_ecma_call_setup(): Ecmascript-to-Ecmascript calls, including
 *      tail calls and coroutine resume
 */

#include "duk_internal.h"

/*
 *  Arguments object creation.
 *
 *  Creating arguments objects is a bit finicky, see E5 Section 10.6 for the
 *  specific requirements.  Much of the arguments object exotic behavior is
 *  implemented in duk_hobject_props.c, and is enabled by the object flag
 *  DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS.
 */

DUK_LOCAL
void duk__create_arguments_object(duk_hthread *thr,
                                  duk_hobject *func,
                                  duk_hobject *varenv,
                                  duk_idx_t idx_argbase,        /* idx of first argument on stack */
                                  duk_idx_t num_stack_args) {   /* num args starting from idx_argbase */
	duk_context *ctx = (duk_context *) thr;
	duk_hobject *arg;          /* 'arguments' */
	duk_hobject *formals;      /* formals for 'func' (may be NULL if func is a C function) */
	duk_idx_t i_arg;
	duk_idx_t i_map;
	duk_idx_t i_mappednames;
	duk_idx_t i_formals;
	duk_idx_t i_argbase;
	duk_idx_t n_formals;
	duk_idx_t idx;
	duk_bool_t need_map;

	DUK_DDD(DUK_DDDPRINT("creating arguments object for func=%!iO, varenv=%!iO, "
	                     "idx_argbase=%ld, num_stack_args=%ld",
	                     (duk_heaphdr *) func, (duk_heaphdr *) varenv,
	                     (long) idx_argbase, (long) num_stack_args));

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_NONBOUND_FUNCTION(func));
	DUK_ASSERT(varenv != NULL);
	DUK_ASSERT(idx_argbase >= 0);  /* assumed to bottom relative */
	DUK_ASSERT(num_stack_args >= 0);

	need_map = 0;

	i_argbase = idx_argbase;
	DUK_ASSERT(i_argbase >= 0);

	duk_push_hobject(ctx, func);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_FORMALS);
	formals = duk_get_hobject(ctx, -1);
	n_formals = 0;
	if (formals) {
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_LENGTH);
		n_formals = (duk_idx_t) duk_require_int(ctx, -1);
		duk_pop(ctx);
	}
	duk_remove(ctx, -2);  /* leave formals on stack for later use */
	i_formals = duk_require_top_index(ctx);

	DUK_ASSERT(n_formals >= 0);
	DUK_ASSERT(formals != NULL || n_formals == 0);

	DUK_DDD(DUK_DDDPRINT("func=%!O, formals=%!O, n_formals=%ld",
	                     (duk_heaphdr *) func, (duk_heaphdr *) formals,
	                     (long) n_formals));

	/* [ ... formals ] */

	/*
	 *  Create required objects:
	 *    - 'arguments' object: array-like, but not an array
	 *    - 'map' object: internal object, tied to 'arguments'
	 *    - 'mappedNames' object: temporary value used during construction
	 */

	i_arg = duk_push_object_helper(ctx,
	                               DUK_HOBJECT_FLAG_EXTENSIBLE |
	                               DUK_HOBJECT_FLAG_ARRAY_PART |
	                               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ARGUMENTS),
	                               DUK_BIDX_OBJECT_PROTOTYPE);
	DUK_ASSERT(i_arg >= 0);
	arg = duk_require_hobject(ctx, -1);
	DUK_ASSERT(arg != NULL);

	i_map = duk_push_object_helper(ctx,
	                               DUK_HOBJECT_FLAG_EXTENSIBLE |
	                               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                               -1);  /* no prototype */
	DUK_ASSERT(i_map >= 0);

	i_mappednames = duk_push_object_helper(ctx,
	                                       DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                                       -1);  /* no prototype */
	DUK_ASSERT(i_mappednames >= 0);

	/* [... formals arguments map mappedNames] */

	DUK_DDD(DUK_DDDPRINT("created arguments related objects: "
	                     "arguments at index %ld -> %!O "
	                     "map at index %ld -> %!O "
	                     "mappednames at index %ld -> %!O",
	                     (long) i_arg, (duk_heaphdr *) duk_get_hobject(ctx, i_arg),
	                     (long) i_map, (duk_heaphdr *) duk_get_hobject(ctx, i_map),
	                     (long) i_mappednames, (duk_heaphdr *) duk_get_hobject(ctx, i_mappednames)));

	/*
	 *  Init arguments properties, map, etc.
	 */

	duk_push_int(ctx, num_stack_args);
	duk_xdef_prop_stridx(ctx, i_arg, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_WC);

	/*
	 *  Init argument related properties
	 */

	/* step 11 */
	idx = num_stack_args - 1;
	while (idx >= 0) {
		DUK_DDD(DUK_DDDPRINT("arg idx %ld, argbase=%ld, argidx=%ld",
		                     (long) idx, (long) i_argbase, (long) (i_argbase + idx)));

		DUK_DDD(DUK_DDDPRINT("define arguments[%ld]=arg", (long) idx));
		duk_dup(ctx, i_argbase + idx);
		duk_xdef_prop_index_wec(ctx, i_arg, (duk_uarridx_t) idx);
		DUK_DDD(DUK_DDDPRINT("defined arguments[%ld]=arg", (long) idx));

		/* step 11.c is relevant only if non-strict (checked in 11.c.ii) */
		if (!DUK_HOBJECT_HAS_STRICT(func) && idx < n_formals) {
			DUK_ASSERT(formals != NULL);

			DUK_DDD(DUK_DDDPRINT("strict function, index within formals (%ld < %ld)",
			                     (long) idx, (long) n_formals));

			duk_get_prop_index(ctx, i_formals, idx);
			DUK_ASSERT(duk_is_string(ctx, -1));

			duk_dup(ctx, -1);  /* [... name name] */

			if (!duk_has_prop(ctx, i_mappednames)) {
				/* steps 11.c.ii.1 - 11.c.ii.4, but our internal book-keeping
				 * differs from the reference model
				 */

				/* [... name] */

				need_map = 1;

				DUK_DDD(DUK_DDDPRINT("set mappednames[%s]=%ld",
				                     (const char *) duk_get_string(ctx, -1),
				                     (long) idx));
				duk_dup(ctx, -1);                      /* name */
				duk_push_uint(ctx, (duk_uint_t) idx);  /* index */
				duk_to_string(ctx, -1);
				duk_xdef_prop_wec(ctx, i_mappednames);  /* out of spec, must be configurable */

				DUK_DDD(DUK_DDDPRINT("set map[%ld]=%s",
				                     (long) idx,
				                     duk_get_string(ctx, -1)));
				duk_dup(ctx, -1);         /* name */
				duk_xdef_prop_index_wec(ctx, i_map, (duk_uarridx_t) idx);  /* out of spec, must be configurable */
			} else {
				/* duk_has_prop() popped the second 'name' */
			}

			/* [... name] */
			duk_pop(ctx);  /* pop 'name' */
		}

		idx--;
	}

	DUK_DDD(DUK_DDDPRINT("actual arguments processed"));

	/* step 12 */
	if (need_map) {
		DUK_DDD(DUK_DDDPRINT("adding 'map' and 'varenv' to arguments object"));

		/* should never happen for a strict callee */
		DUK_ASSERT(!DUK_HOBJECT_HAS_STRICT(func));

		duk_dup(ctx, i_map);
		duk_xdef_prop_stridx(ctx, i_arg, DUK_STRIDX_INT_MAP, DUK_PROPDESC_FLAGS_NONE);  /* out of spec, don't care */

		/* The variable environment for magic variable bindings needs to be
		 * given by the caller and recorded in the arguments object.
		 *
		 * See E5 Section 10.6, the creation of setters/getters.
		 *
		 * The variable environment also provides access to the callee, so
		 * an explicit (internal) callee property is not needed.
		 */

		duk_push_hobject(ctx, varenv);
		duk_xdef_prop_stridx(ctx, i_arg, DUK_STRIDX_INT_VARENV, DUK_PROPDESC_FLAGS_NONE);  /* out of spec, don't care */
	}

	/* steps 13-14 */
	if (DUK_HOBJECT_HAS_STRICT(func)) {
		/*
		 *  Note: callee/caller are throwers and are not deletable etc.
		 *  They could be implemented as virtual properties, but currently
		 *  there is no support for virtual properties which are accessors
		 *  (only plain virtual properties).  This would not be difficult
		 *  to change in duk_hobject_props, but we can make the throwers
		 *  normal, concrete properties just as easily.
		 *
		 *  Note that the specification requires that the *same* thrower
		 *  built-in object is used here!  See E5 Section 10.6 main
		 *  algoritm, step 14, and Section 13.2.3 which describes the
		 *  thrower.  See test case test-arguments-throwers.js.
		 */

		DUK_DDD(DUK_DDDPRINT("strict function, setting caller/callee to throwers"));

		duk_xdef_prop_stridx_thrower(ctx, i_arg, DUK_STRIDX_CALLER, DUK_PROPDESC_FLAGS_NONE);
		duk_xdef_prop_stridx_thrower(ctx, i_arg, DUK_STRIDX_CALLEE, DUK_PROPDESC_FLAGS_NONE);
	} else {
		DUK_DDD(DUK_DDDPRINT("non-strict function, setting callee to actual value"));
		duk_push_hobject(ctx, func);
		duk_xdef_prop_stridx(ctx, i_arg, DUK_STRIDX_CALLEE, DUK_PROPDESC_FLAGS_WC);
	}

	/* set exotic behavior only after we're done */
	if (need_map) {
		/*
		 *  Note: exotic behaviors are only enabled for arguments
		 *  objects which have a parameter map (see E5 Section 10.6
		 *  main algorithm, step 12).
		 *
		 *  In particular, a non-strict arguments object with no
		 *  mapped formals does *NOT* get exotic behavior, even
		 *  for e.g. "caller" property.  This seems counterintuitive
		 *  but seems to be the case.
		 */

		/* cannot be strict (never mapped variables) */
		DUK_ASSERT(!DUK_HOBJECT_HAS_STRICT(func));

		DUK_DDD(DUK_DDDPRINT("enabling exotic behavior for arguments object"));
		DUK_HOBJECT_SET_EXOTIC_ARGUMENTS(arg);
	} else {
		DUK_DDD(DUK_DDDPRINT("not enabling exotic behavior for arguments object"));
	}

	/* nice log */
	DUK_DDD(DUK_DDDPRINT("final arguments related objects: "
	                     "arguments at index %ld -> %!O "
	                     "map at index %ld -> %!O "
	                     "mappednames at index %ld -> %!O",
	                     (long) i_arg, (duk_heaphdr *) duk_get_hobject(ctx, i_arg),
	                     (long) i_map, (duk_heaphdr *) duk_get_hobject(ctx, i_map),
	                     (long) i_mappednames, (duk_heaphdr *) duk_get_hobject(ctx, i_mappednames)));

	/* [args(n) [crud] formals arguments map mappednames] -> [args [crud] arguments] */
	duk_pop_2(ctx);
	duk_remove(ctx, -2);
}

/* Helper for creating the arguments object and adding it to the env record
 * on top of the value stack.  This helper has a very strict dependency on
 * the shape of the input stack.
 */
DUK_LOCAL
void duk__handle_createargs_for_call(duk_hthread *thr,
                                     duk_hobject *func,
                                     duk_hobject *env,
                                     duk_idx_t num_stack_args) {
	duk_context *ctx = (duk_context *) thr;

	DUK_DDD(DUK_DDDPRINT("creating arguments object for function call"));

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(env != NULL);
	DUK_ASSERT(DUK_HOBJECT_HAS_CREATEARGS(func));
	DUK_ASSERT(duk_get_top(ctx) >= num_stack_args + 1);

	/* [... arg1 ... argN envobj] */

	duk__create_arguments_object(thr,
	                             func,
	                             env,
	                             duk_get_top(ctx) - num_stack_args - 1,    /* idx_argbase */
	                             num_stack_args);

	/* [... arg1 ... argN envobj argobj] */

	duk_xdef_prop_stridx(ctx,
	                     -2,
	                     DUK_STRIDX_LC_ARGUMENTS,
	                     DUK_HOBJECT_HAS_STRICT(func) ? DUK_PROPDESC_FLAGS_E :   /* strict: non-deletable, non-writable */
	                                                    DUK_PROPDESC_FLAGS_WE);  /* non-strict: non-deletable, writable */
	/* [... arg1 ... argN envobj] */
}

/*
 *  Helper for handling a "bound function" chain when a call is being made.
 *
 *  Follows the bound function chain until a non-bound function is found.
 *  Prepends the bound arguments to the value stack (at idx_func + 2),
 *  updating 'num_stack_args' in the process.  The 'this' binding is also
 *  updated if necessary (at idx_func + 1).  Note that for constructor calls
 *  the 'this' binding is never updated by [[BoundThis]].
 *
 *  XXX: bound function chains could be collapsed at bound function creation
 *  time so that each bound function would point directly to a non-bound
 *  function.  This would make call time handling much easier.
 */

DUK_LOCAL
void duk__handle_bound_chain_for_call(duk_hthread *thr,
                                      duk_idx_t idx_func,
                                      duk_idx_t *p_num_stack_args,   /* may be changed by call */
                                      duk_bool_t is_constructor_call) {
	duk_context *ctx = (duk_context *) thr;
	duk_idx_t num_stack_args;
	duk_tval *tv_func;
	duk_hobject *func;
	duk_uint_t sanity;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(p_num_stack_args != NULL);

	/* On entry, item at idx_func is a bound, non-lightweight function,
	 * but we don't rely on that below.
	 */

	num_stack_args = *p_num_stack_args;

	sanity = DUK_HOBJECT_BOUND_CHAIN_SANITY;
	do {
		duk_idx_t i, len;

		tv_func = duk_require_tval(ctx, idx_func);
		DUK_ASSERT(tv_func != NULL);

		if (DUK_TVAL_IS_LIGHTFUNC(tv_func)) {
			/* Lightweight function: never bound, so terminate. */
			break;
		} else if (DUK_TVAL_IS_OBJECT(tv_func)) {
			func = DUK_TVAL_GET_OBJECT(tv_func);
			if (!DUK_HOBJECT_HAS_BOUND(func)) {
				/* Normal non-bound function. */
				break;
			}
		} else {
			/* Function.prototype.bind() should never let this happen,
			 * ugly error message is enough.
			 */
			DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, DUK_STR_INTERNAL_ERROR);
		}
		DUK_ASSERT(DUK_TVAL_GET_OBJECT(tv_func) != NULL);

		/* XXX: this could be more compact by accessing the internal properties
		 * directly as own properties (they cannot be inherited, and are not
		 * externally visible).
		 */

		DUK_DDD(DUK_DDDPRINT("bound function encountered, ptr=%p, num_stack_args=%ld: %!T",
		                     (void *) DUK_TVAL_GET_OBJECT(tv_func), (long) num_stack_args, tv_func));

		/* [ ... func this arg1 ... argN ] */

		if (is_constructor_call) {
			/* See: ecmascript-testcases/test-spec-bound-constructor.js */
			DUK_DDD(DUK_DDDPRINT("constructor call: don't update this binding"));
		} else {
			duk_get_prop_stridx(ctx, idx_func, DUK_STRIDX_INT_THIS);
			duk_replace(ctx, idx_func + 1);  /* idx_this = idx_func + 1 */
		}

		/* [ ... func this arg1 ... argN ] */

		/* XXX: duk_get_length? */
		duk_get_prop_stridx(ctx, idx_func, DUK_STRIDX_INT_ARGS);  /* -> [ ... func this arg1 ... argN _Args ] */
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_LENGTH);          /* -> [ ... func this arg1 ... argN _Args length ] */
		len = (duk_idx_t) duk_require_int(ctx, -1);
		duk_pop(ctx);
		for (i = 0; i < len; i++) {
			/* XXX: very slow - better to bulk allocate a gap, and copy
			 * from args_array directly (we know it has a compact array
			 * part, etc).
			 */

			/* [ ... func this <some bound args> arg1 ... argN _Args ] */
			duk_get_prop_index(ctx, -1, i);
			duk_insert(ctx, idx_func + 2 + i);  /* idx_args = idx_func + 2 */
		}
		num_stack_args += len;  /* must be updated to work properly (e.g. creation of 'arguments') */
		duk_pop(ctx);

		/* [ ... func this <bound args> arg1 ... argN ] */

		duk_get_prop_stridx(ctx, idx_func, DUK_STRIDX_INT_TARGET);
		duk_replace(ctx, idx_func);  /* replace in stack */

		DUK_DDD(DUK_DDDPRINT("bound function handled, num_stack_args=%ld, idx_func=%ld, curr func=%!T",
		                     (long) num_stack_args, (long) idx_func, duk_get_tval(ctx, idx_func)));
	} while (--sanity > 0);

	if (sanity == 0) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, DUK_STR_BOUND_CHAIN_LIMIT);
	}

	DUK_DDD(DUK_DDDPRINT("final non-bound function is: %!T", duk_get_tval(ctx, idx_func)));

#ifdef DUK_USE_ASSERTIONS
	tv_func = duk_require_tval(ctx, idx_func);
	DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv_func) || DUK_TVAL_IS_OBJECT(tv_func));
	if (DUK_TVAL_IS_OBJECT(tv_func)) {
		func = DUK_TVAL_GET_OBJECT(tv_func);
		DUK_ASSERT(func != NULL);
		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(func));
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPILEDFUNCTION(func) ||
		           DUK_HOBJECT_HAS_NATIVEFUNCTION(func));
	}
#endif

	/* write back */
	*p_num_stack_args = num_stack_args;
}

/*
 *  Helper for setting up var_env and lex_env of an activation,
 *  assuming it does NOT have the DUK_HOBJECT_FLAG_NEWENV flag.
 */

DUK_LOCAL
void duk__handle_oldenv_for_call(duk_hthread *thr,
                                 duk_hobject *func,
                                 duk_activation *act) {
	duk_tval *tv;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(act != NULL);
	DUK_ASSERT(!DUK_HOBJECT_HAS_NEWENV(func));
	DUK_ASSERT(!DUK_HOBJECT_HAS_CREATEARGS(func));

	tv = duk_hobject_find_existing_entry_tval_ptr(thr->heap, func, DUK_HTHREAD_STRING_INT_LEXENV(thr));
	if (tv) {
		DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv));
		DUK_ASSERT(DUK_HOBJECT_IS_ENV(DUK_TVAL_GET_OBJECT(tv)));
		act->lex_env = DUK_TVAL_GET_OBJECT(tv);

		tv = duk_hobject_find_existing_entry_tval_ptr(thr->heap, func, DUK_HTHREAD_STRING_INT_VARENV(thr));
		if (tv) {
			DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv));
			DUK_ASSERT(DUK_HOBJECT_IS_ENV(DUK_TVAL_GET_OBJECT(tv)));
			act->var_env = DUK_TVAL_GET_OBJECT(tv);
		} else {
			act->var_env = act->lex_env;
		}
	} else {
		act->lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
		act->var_env = act->lex_env;
	}

	DUK_HOBJECT_INCREF_ALLOWNULL(thr, act->lex_env);
	DUK_HOBJECT_INCREF_ALLOWNULL(thr, act->var_env);
}

/*
 *  Helper for updating callee 'caller' property.
 */

#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
DUK_LOCAL void duk__update_func_caller_prop(duk_hthread *thr, duk_hobject *func) {
	duk_tval *tv_caller;
	duk_hobject *h_tmp;
	duk_activation *act_callee;
	duk_activation *act_caller;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(func));  /* bound chain resolved */
	DUK_ASSERT(thr->callstack_top >= 1);

	if (DUK_HOBJECT_HAS_STRICT(func)) {
		/* Strict functions don't get their 'caller' updated. */
		return;
	}

	act_callee = thr->callstack + thr->callstack_top - 1;
	act_caller = (thr->callstack_top >= 2 ? act_callee - 1 : NULL);

	/* Backup 'caller' property and update its value. */
	tv_caller = duk_hobject_find_existing_entry_tval_ptr(thr->heap, func, DUK_HTHREAD_STRING_CALLER(thr));
	if (tv_caller) {
		/* If caller is global/eval code, 'caller' should be set to
		 * 'null'.
		 *
		 * XXX: there is no exotic flag to infer this correctly now.
		 * The NEWENV flag is used now which works as intended for
		 * everything (global code, non-strict eval code, and functions)
		 * except strict eval code.  Bound functions are never an issue
		 * because 'func' has been resolved to a non-bound function.
		 */

		if (act_caller) {
			/* act_caller->func may be NULL in some finalization cases,
			 * just treat like we don't know the caller.
			 */
			if (act_caller->func && !DUK_HOBJECT_HAS_NEWENV(act_caller->func)) {
				/* Setting to NULL causes 'caller' to be set to
				 * 'null' as desired.
				 */
				act_caller = NULL;
			}
		}

		if (DUK_TVAL_IS_OBJECT(tv_caller)) {
			h_tmp = DUK_TVAL_GET_OBJECT(tv_caller);
			DUK_ASSERT(h_tmp != NULL);
			act_callee->prev_caller = h_tmp;

			/* Previous value doesn't need refcount changes because its ownership
			 * is transferred to prev_caller.
			 */

			if (act_caller) {
				DUK_ASSERT(act_caller->func != NULL);
				DUK_TVAL_SET_OBJECT(tv_caller, act_caller->func);
				DUK_TVAL_INCREF(thr, tv_caller);
			} else {
				DUK_TVAL_SET_NULL(tv_caller);  /* no incref */
			}
		} else {
			/* 'caller' must only take on 'null' or function value */
			DUK_ASSERT(!DUK_TVAL_IS_HEAP_ALLOCATED(tv_caller));
			DUK_ASSERT(act_callee->prev_caller == NULL);
			if (act_caller && act_caller->func) {
				/* Tolerate act_caller->func == NULL which happens in
				 * some finalization cases; treat like unknown caller.
				 */
				DUK_TVAL_SET_OBJECT(tv_caller, act_caller->func);
				DUK_TVAL_INCREF(thr, tv_caller);
			} else {
				DUK_TVAL_SET_NULL(tv_caller);  /* no incref */
			}
		}
	}
}
#endif  /* DUK_USE_NONSTD_FUNC_CALLER_PROPERTY */

/*
 *  Determine the effective 'this' binding and coerce the current value
 *  on the valstack to the effective one (in-place, at idx_this).
 *
 *  The current this value in the valstack (at idx_this) represents either:
 *    - the caller's requested 'this' binding; or
 *    - a 'this' binding accumulated from the bound function chain
 *
 *  The final 'this' binding for the target function may still be
 *  different, and is determined as described in E5 Section 10.4.3.
 *
 *  For global and eval code (E5 Sections 10.4.1 and 10.4.2), we assume
 *  that the caller has provided the correct 'this' binding explicitly
 *  when calling, i.e.:
 *
 *    - global code: this=global object
 *    - direct eval: this=copy from eval() caller's this binding
 *    - other eval:  this=global object
 *
 *  Note: this function may cause a recursive function call with arbitrary
 *  side effects, because ToObject() may be called.
 */

DUK_LOCAL
void duk__coerce_effective_this_binding(duk_hthread *thr,
                                        duk_hobject *func,
                                        duk_idx_t idx_this) {
	duk_context *ctx = (duk_context *) thr;
	duk_small_int_t strict;

	if (func) {
		strict = DUK_HOBJECT_HAS_STRICT(func);
	} else {
		/* Lightfuncs are always considered strict. */
		strict = 1;
	}

	if (strict) {
		DUK_DDD(DUK_DDDPRINT("this binding: strict -> use directly"));
	} else {
		duk_tval *tv_this = duk_require_tval(ctx, idx_this);
		duk_hobject *obj_global;

		if (DUK_TVAL_IS_OBJECT(tv_this)) {
			DUK_DDD(DUK_DDDPRINT("this binding: non-strict, object -> use directly"));
		} else if (DUK_TVAL_IS_LIGHTFUNC(tv_this)) {
			/* Lightfuncs are treated like objects and not coerced. */
			DUK_DDD(DUK_DDDPRINT("this binding: non-strict, lightfunc -> use directly"));
		} else if (DUK_TVAL_IS_UNDEFINED(tv_this) || DUK_TVAL_IS_NULL(tv_this)) {
			DUK_DDD(DUK_DDDPRINT("this binding: non-strict, undefined/null -> use global object"));
			obj_global = thr->builtins[DUK_BIDX_GLOBAL];
			if (obj_global) {
				duk_push_hobject(ctx, obj_global);
			} else {
				/*
				 *  This may only happen if built-ins are being "torn down".
				 *  This behavior is out of specification scope.
				 */
				DUK_D(DUK_DPRINT("this binding: wanted to use global object, but it is NULL -> using undefined instead"));
				duk_push_undefined(ctx);
			}
			duk_replace(ctx, idx_this);
		} else {
			DUK_DDD(DUK_DDDPRINT("this binding: non-strict, not object/undefined/null -> use ToObject(value)"));
			duk_to_object(ctx, idx_this);  /* may have side effects */
		}
	}
}

/*
 *  Shared helper for non-bound func lookup.
 *
 *  Returns duk_hobject * to the final non-bound function (NULL for lightfunc).
 */

DUK_LOCAL
duk_hobject *duk__nonbound_func_lookup(duk_context *ctx,
                                       duk_idx_t idx_func,
                                       duk_idx_t *out_num_stack_args,
                                       duk_tval **out_tv_func,
                                       duk_small_uint_t call_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_func;
	duk_hobject *func;

	for (;;) {
		/* Use loop to minimize code size of relookup after bound function case */
		tv_func = duk_get_tval(ctx, idx_func);
		DUK_ASSERT(tv_func != NULL);

		if (DUK_TVAL_IS_OBJECT(tv_func)) {
			func = DUK_TVAL_GET_OBJECT(tv_func);
			if (!DUK_HOBJECT_IS_CALLABLE(func)) {
				goto not_callable_error;
			}
			if (DUK_HOBJECT_HAS_BOUND(func)) {
				duk__handle_bound_chain_for_call(thr, idx_func, out_num_stack_args, call_flags & DUK_CALL_FLAG_CONSTRUCTOR_CALL);

				/* The final object may be a normal function or a lightfunc.
				 * We need to re-lookup tv_func because it may have changed
				 * (also value stack may have been resized).  Loop again to
				 * do that; we're guaranteed not to come here again.
				 */
				DUK_ASSERT(DUK_TVAL_IS_OBJECT(duk_require_tval(ctx, idx_func)) ||
				           DUK_TVAL_IS_LIGHTFUNC(duk_require_tval(ctx, idx_func)));
				continue;
			}
		} else if (DUK_TVAL_IS_LIGHTFUNC(tv_func)) {
			func = NULL;
		} else {
			goto not_callable_error;
		}
		break;
	}

	DUK_ASSERT((DUK_TVAL_IS_OBJECT(tv_func) && DUK_HOBJECT_IS_CALLABLE(DUK_TVAL_GET_OBJECT(tv_func))) ||
	           DUK_TVAL_IS_LIGHTFUNC(tv_func));
	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUND(func));
	DUK_ASSERT(func == NULL || (DUK_HOBJECT_IS_COMPILEDFUNCTION(func) ||
	                            DUK_HOBJECT_IS_NATIVEFUNCTION(func)));

	*out_tv_func = tv_func;
	return func;

 not_callable_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_CALLABLE);
	DUK_UNREACHABLE();
	return NULL;  /* never executed */
}

/*
 *  Value stack resize and stack top adjustment helper
 *
 *  XXX: This should all be merged to duk_valstack_resize_raw().
 */

DUK_LOCAL
void duk__adjust_valstack_and_top(duk_hthread *thr, duk_idx_t num_stack_args, duk_idx_t idx_args, duk_idx_t nregs, duk_idx_t nargs, duk_hobject *func) {
	duk_context *ctx = (duk_context *) thr;
	duk_size_t vs_min_size;
	duk_bool_t adjusted_top = 0;

	vs_min_size = (thr->valstack_bottom - thr->valstack) +         /* bottom of current func */
	              idx_args;                                        /* bottom of new func */

	if (nregs >= 0) {
		DUK_ASSERT(nargs >= 0);
		DUK_ASSERT(nregs >= nargs);
		vs_min_size += nregs;
	} else {
		/* 'func' wants stack "as is" */
		vs_min_size += num_stack_args;  /* num entries of new func at entry */
	}
	if (func == NULL || DUK_HOBJECT_IS_NATIVEFUNCTION(func)) {
		vs_min_size += DUK_VALSTACK_API_ENTRY_MINIMUM;         /* Duktape/C API guaranteed entries (on top of args) */
	}
	vs_min_size += DUK_VALSTACK_INTERNAL_EXTRA;                    /* + spare */

	/* XXX: Awkward fix for GH-107: we can't resize the value stack to
	 * a size smaller than the current top, so the order of the resize
	 * and adjusting the stack top depends on the current vs. final size
	 * of the value stack.  Ideally duk_valstack_resize_raw() would have
	 * a combined algorithm to avoid this.
	 */

	if (vs_min_size < (duk_size_t) (thr->valstack_top  - thr->valstack)) {
		DUK_DDD(DUK_DDDPRINT(("final size smaller, set top before resize")));

		DUK_ASSERT(nregs >= 0);  /* can't happen when keeping current stack size */
		duk_set_top(ctx, idx_args + nargs);  /* clamp anything above nargs */
		duk_set_top(ctx, idx_args + nregs);  /* extend with undefined */
		adjusted_top = 1;
	}

	(void) duk_valstack_resize_raw((duk_context *) thr,
	                               vs_min_size,
	                               DUK_VSRESIZE_FLAG_SHRINK |      /* flags */
	                               0 /* no compact */ |
	                               DUK_VSRESIZE_FLAG_THROW);

	if (!adjusted_top) {
		if (nregs >= 0) {
			DUK_ASSERT(nregs >= nargs);
			duk_set_top(ctx, idx_args + nargs);  /* clamp anything above nargs */
			duk_set_top(ctx, idx_args + nregs);  /* extend with undefined */
		}
	}
}

/*
 *  Helper for making various kinds of calls.
 *
 *  Call flags:
 *
 *    DUK_CALL_FLAG_PROTECTED        <-->  protected call
 *    DUK_CALL_FLAG_IGNORE_RECLIMIT  <-->  ignore C recursion limit,
 *                                         for errhandler calls
 *    DUK_CALL_FLAG_CONSTRUCTOR_CALL <-->  for 'new Foo()' calls
 *
 *  Input stack:
 *
 *    [ func this arg1 ... argN ]
 *
 *  Output stack:
 *
 *    [ retval ]         (DUK_EXEC_SUCCESS)
 *    [ errobj ]         (DUK_EXEC_ERROR (normal error), protected call)
 *
 *  Even when executing a protected call an error may be thrown in rare cases.
 *  For instance, if we run out of memory when setting up the return stack
 *  after a caught error, the out of memory is propagated to the caller.
 *  Similarly, API errors (such as invalid input stack shape and invalid
 *  indices) cause an error to propagate out of this function.  If there is
 *  no catchpoint for this error, the fatal error handler is called.
 *
 *  See 'execution.txt'.
 *
 *  The allowed thread states for making a call are:
 *    - thr matches heap->curr_thread, and thr is already RUNNING
 *    - thr does not match heap->curr_thread (may be NULL or other),
 *      and thr is INACTIVE (in this case, a setjmp() catchpoint is
 *      always used for thread book-keeping to work properly)
 *
 *  Like elsewhere, gotos are used to keep indent level minimal and
 *  avoiding a dozen helpers with awkward plumbing.
 *
 *  Note: setjmp() and local variables have a nasty interaction,
 *  see execution.txt; non-volatile locals modified after setjmp()
 *  call are not guaranteed to keep their value.
 */

DUK_INTERNAL
duk_int_t duk_handle_call(duk_hthread *thr,
                          duk_idx_t num_stack_args,
                          duk_small_uint_t call_flags) {
	duk_context *ctx = (duk_context *) thr;
	duk_size_t entry_valstack_bottom_index;
	duk_size_t entry_valstack_end;
	duk_size_t entry_callstack_top;
	duk_size_t entry_catchstack_top;
	duk_int_t entry_call_recursion_depth;
	duk_hthread *entry_curr_thread;
	duk_uint_fast8_t entry_thread_state;
	volatile duk_bool_t need_setjmp;
	duk_jmpbuf * volatile old_jmpbuf_ptr = NULL;    /* ptr is volatile (not the target) */
	duk_idx_t idx_func;         /* valstack index of 'func' and retval (relative to entry valstack_bottom) */
	duk_idx_t idx_args;         /* valstack index of start of args (arg1) (relative to entry valstack_bottom) */
	duk_idx_t nargs;            /* # argument registers target function wants (< 0 => "as is") */
	duk_idx_t nregs;            /* # total registers target function wants on entry (< 0 => "as is") */
	duk_hobject *func;          /* 'func' on stack (borrowed reference) */
	duk_tval *tv_func;          /* duk_tval ptr for 'func' on stack (borrowed reference) or tv_func_copy */
	duk_tval tv_func_copy;      /* to avoid relookups */
	duk_activation *act;
	duk_hobject *env;
	duk_jmpbuf our_jmpbuf;
	duk_tval tv_tmp;
	duk_int_t retval = DUK_EXEC_ERROR;
	duk_ret_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(num_stack_args >= 0);

	/* XXX: currently NULL allocations are not supported; remove if later allowed */
	DUK_ASSERT(thr->valstack != NULL);
	DUK_ASSERT(thr->callstack != NULL);
	DUK_ASSERT(thr->catchstack != NULL);

	/*
	 *  Preliminaries, required by setjmp() handler.
	 *
	 *  Must be careful not to throw an unintended error here.
	 *
	 *  Note: careful with indices like '-x'; if 'x' is zero, it
	 *  refers to valstack_bottom.
	 */

	entry_valstack_bottom_index = (duk_size_t) (thr->valstack_bottom - thr->valstack);
	entry_valstack_end = (duk_size_t) (thr->valstack_end - thr->valstack);
	entry_callstack_top = thr->callstack_top;
	entry_catchstack_top = thr->catchstack_top;
	entry_call_recursion_depth = thr->heap->call_recursion_depth;
	entry_curr_thread = thr->heap->curr_thread;  /* Note: may be NULL if first call */
	entry_thread_state = thr->state;
	idx_func = duk_normalize_index(ctx, -num_stack_args - 2);  /* idx_func must be valid, note: non-throwing! */
	idx_args = idx_func + 2;                                   /* idx_args is not necessarily valid if num_stack_args == 0 (idx_args then equals top) */

	/* Need a setjmp() catchpoint if a protected call OR if we need to
	 * do mandatory cleanup.
	 */
	need_setjmp = ((call_flags & DUK_CALL_FLAG_PROTECTED) != 0) || (thr->heap->curr_thread != thr);

	DUK_DD(DUK_DDPRINT("duk_handle_call: thr=%p, num_stack_args=%ld, "
	                   "call_flags=0x%08lx (protected=%ld, ignorerec=%ld, constructor=%ld), need_setjmp=%ld, "
	                   "valstack_top=%ld, idx_func=%ld, idx_args=%ld, rec_depth=%ld/%ld, "
	                   "entry_valstack_bottom_index=%ld, entry_callstack_top=%ld, entry_catchstack_top=%ld, "
	                   "entry_call_recursion_depth=%ld, entry_curr_thread=%p, entry_thread_state=%ld",
	                   (void *) thr,
	                   (long) num_stack_args,
	                   (unsigned long) call_flags,
	                   (long) ((call_flags & DUK_CALL_FLAG_PROTECTED) != 0 ? 1 : 0),
	                   (long) ((call_flags & DUK_CALL_FLAG_IGNORE_RECLIMIT) != 0 ? 1 : 0),
	                   (long) ((call_flags & DUK_CALL_FLAG_CONSTRUCTOR_CALL) != 0 ? 1 : 0),
	                   (long) need_setjmp,
	                   (long) duk_get_top(ctx),
	                   (long) idx_func,
	                   (long) idx_args,
	                   (long) thr->heap->call_recursion_depth,
	                   (long) thr->heap->call_recursion_limit,
	                   (long) entry_valstack_bottom_index,
	                   (long) entry_callstack_top,
	                   (long) entry_catchstack_top,
	                   (long) entry_call_recursion_depth,
	                   (void *) entry_curr_thread,
	                   (long) entry_thread_state));

	/* XXX: Multiple tv_func lookups are now avoided by making a local
	 * copy of tv_func.  Another approach would be to compute an offset
	 * for tv_func from valstack bottom and recomputing the tv_func
	 * pointer quickly as valstack + offset instead of calling duk_get_tval().
	 */

	if (idx_func < 0 || idx_args < 0) {
		/*
		 *  Since stack indices are not reliable, we can't do anything useful
		 *  here.  Invoke the existing setjmp catcher, or if it doesn't exist,
		 *  call the fatal error handler.
		 */

		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CALL_ARGS);
	}

	/*
	 *  Setup a setjmp() catchpoint first because even the call setup
	 *  may fail.
	 */

	if (!need_setjmp) {
		DUK_DDD(DUK_DDDPRINT("don't need a setjmp catchpoint"));
		goto handle_call;
	}

	old_jmpbuf_ptr = thr->heap->lj.jmpbuf_ptr;
	thr->heap->lj.jmpbuf_ptr = &our_jmpbuf;

	if (DUK_SETJMP(thr->heap->lj.jmpbuf_ptr->jb) == 0) {
		DUK_DDD(DUK_DDDPRINT("setjmp catchpoint setup complete"));
		goto handle_call;
	}

	/*
	 *  Error during setup, call, or postprocessing of the call.
	 *  The error value is in heap->lj.value1.
	 *
	 *  Note: any local variables accessed here must have their value
	 *  assigned *before* the setjmp() call, OR they must be declared
	 *  volatile.  Otherwise their value is not guaranteed to be correct.
	 *
	 *  The following are such variables:
	 *    - duk_handle_call() parameters
	 *    - entry_*
	 *    - idx_func
	 *    - idx_args
	 *
	 *  The very first thing we do is restore the previous setjmp catcher.
	 *  This means that any error in error handling will propagate outwards
	 *  instead of causing a setjmp() re-entry above.  The *only* actual
	 *  errors that should happen here are allocation errors.
	 */

	DUK_DDD(DUK_DDDPRINT("error caught during protected duk_handle_call(): %!T",
	                     (duk_tval *) &thr->heap->lj.value1));

	DUK_ASSERT(thr->heap->lj.type == DUK_LJ_TYPE_THROW);
	DUK_ASSERT(thr->callstack_top >= entry_callstack_top);
	DUK_ASSERT(thr->catchstack_top >= entry_catchstack_top);

	/*
	 *  Restore previous setjmp catchpoint
	 */

	/* Note: either pointer may be NULL (at entry), so don't assert */
	DUK_DDD(DUK_DDDPRINT("restore jmpbuf_ptr: %p -> %p",
	                     (void *) (thr && thr->heap ? thr->heap->lj.jmpbuf_ptr : NULL),
	                     (void *) old_jmpbuf_ptr));

	thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

	if (!(call_flags & DUK_CALL_FLAG_PROTECTED)) {
		/*
		 *  Caller did not request a protected call but a setjmp
		 *  catchpoint was set up to allow cleanup.  So, clean up
		 *  and rethrow.
		 *
		 *  We must restore curr_thread here to ensure that its
		 *  current value doesn't end up pointing to a thread object
		 *  which has been freed.  This is now a problem because some
		 *  call sites (namely duk_safe_call()) *first* unwind stacks
		 *  and only then deal with curr_thread.  If those call sites
		 *  were fixed, this wouldn't matter here.
		 *
		 *  Note: this case happens e.g. when heap->curr_thread is
		 *  NULL on entry.
		 */

		DUK_DDD(DUK_DDDPRINT("call is not protected -> clean up and rethrow"));

		DUK_HEAP_SWITCH_THREAD(thr->heap, entry_curr_thread);  /* may be NULL */
		thr->state = entry_thread_state;
		DUK_ASSERT((thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread == NULL) ||  /* first call */
		           (thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread != NULL) ||  /* other call */
		           (thr->state == DUK_HTHREAD_STATE_RUNNING && thr->heap->curr_thread == thr));     /* current thread */

		/* XXX: should setjmp catcher be responsible for this instead? */
		thr->heap->call_recursion_depth = entry_call_recursion_depth;
		duk_err_longjmp(thr);
		DUK_UNREACHABLE();
	}

	duk_hthread_catchstack_unwind(thr, entry_catchstack_top);
	duk_hthread_callstack_unwind(thr, entry_callstack_top);
	thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;

	/* [ ... func this (crud) errobj ] */

	/* XXX: is there space?  better implementation: write directly over
	 * 'func' slot to avoid valstack grow issues.
	 */
	duk_push_tval(ctx, &thr->heap->lj.value1);

	/* [ ... func this (crud) errobj ] */

	duk_replace(ctx, idx_func);
	duk_set_top(ctx, idx_func + 1);

	/* [ ... errobj ] */

	/* Ensure there is internal valstack spare before we exit; this may
	 * throw an alloc error.  The same guaranteed size must be available
	 * as before the call.  This is not optimal now: we store the valstack
	 * allocated size during entry; this value may be higher than the
	 * minimal guarantee for an application.
	 */

	(void) duk_valstack_resize_raw((duk_context *) thr,
	                               entry_valstack_end,                    /* same as during entry */
	                               DUK_VSRESIZE_FLAG_SHRINK |             /* flags */
	                               DUK_VSRESIZE_FLAG_COMPACT |
	                               DUK_VSRESIZE_FLAG_THROW);

	/* Note: currently a second setjmp restoration is done at the target;
	 * this is OK, but could be refactored away.
	 */
	retval = DUK_EXEC_ERROR;
	goto shrink_and_finished;

 handle_call:
	/*
	 *  Thread state check and book-keeping.
	 */

	if (thr == thr->heap->curr_thread) {
		/* same thread */
		if (thr->state != DUK_HTHREAD_STATE_RUNNING) {
			/* should actually never happen, but check anyway */
			goto thread_state_error;
		}
	} else {
		/* different thread */
		DUK_ASSERT(thr->heap->curr_thread == NULL ||
		           thr->heap->curr_thread->state == DUK_HTHREAD_STATE_RUNNING);
		if (thr->state != DUK_HTHREAD_STATE_INACTIVE) {
			goto thread_state_error;
		}
		DUK_HEAP_SWITCH_THREAD(thr->heap, thr);
		thr->state = DUK_HTHREAD_STATE_RUNNING;

		/* Note: multiple threads may be simultaneously in the RUNNING
		 * state, but not in the same "resume chain".
		 */
	}

	DUK_ASSERT(thr->heap->curr_thread == thr);
	DUK_ASSERT(thr->state == DUK_HTHREAD_STATE_RUNNING);

	/*
	 *  C call recursion depth check, which provides a reasonable upper
	 *  bound on maximum C stack size (arbitrary C stack growth is only
	 *  possible by recursive handle_call / handle_safe_call calls).
	 */

	DUK_ASSERT(thr->heap->call_recursion_depth >= 0);
	DUK_ASSERT(thr->heap->call_recursion_depth <= thr->heap->call_recursion_limit);

	if (call_flags & DUK_CALL_FLAG_IGNORE_RECLIMIT) {
		DUK_DD(DUK_DDPRINT("ignoring reclimit for this call (probably an errhandler call)"));
	} else {
		if (thr->heap->call_recursion_depth >= thr->heap->call_recursion_limit) {
			/* XXX: error message is a bit misleading: we reached a recursion
			 * limit which is also essentially the same as a C callstack limit
			 * (except perhaps with some relaxed threading assumptions).
			 */
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_C_CALLSTACK_LIMIT);
		}
		thr->heap->call_recursion_depth++;
	}

	/*
	 *  Check the function type, handle bound function chains, and prepare
	 *  parameters for the rest of the call handling.  Also figure out the
	 *  effective 'this' binding, which replaces the current value at
	 *  idx_func + 1.
	 *
	 *  If the target function is a 'bound' one, follow the chain of 'bound'
	 *  functions until a non-bound function is found.  During this process,
	 *  bound arguments are 'prepended' to existing ones, and the "this"
	 *  binding is overridden.  See E5 Section 15.3.4.5.1.
	 *
	 *  Lightfunc detection happens here too.  Note that lightweight functions
	 *  can be wrapped by (non-lightweight) bound functions so we must resolve
	 *  the bound function chain first.
	 */

	func = duk__nonbound_func_lookup(ctx, idx_func, &num_stack_args, &tv_func, call_flags);
	DUK_TVAL_SET_TVAL(&tv_func_copy, tv_func);
	tv_func = &tv_func_copy;  /* local copy to avoid relookups */

	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUND(func));
	DUK_ASSERT(func == NULL || (DUK_HOBJECT_IS_COMPILEDFUNCTION(func) ||
	                            DUK_HOBJECT_IS_NATIVEFUNCTION(func)));

	duk__coerce_effective_this_binding(thr, func, idx_func + 1);
	DUK_DDD(DUK_DDDPRINT("effective 'this' binding is: %!T",
	                     (duk_tval *) duk_get_tval(ctx, idx_func + 1)));

	/* These base values are never used, but if the compiler doesn't know
	 * that DUK_ERROR() won't return, these are needed to silence warnings.
	 * On the other hand, scan-build will warn about the values not being
	 * used, so add a DUK_UNREF.
	 */
	nargs = 0; DUK_UNREF(nargs);
	nregs = 0; DUK_UNREF(nregs);

	if (func == NULL) {
		duk_small_uint_t lf_flags;

		DUK_DDD(DUK_DDDPRINT("lightfunc call handling"));
		DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv_func));
		lf_flags = DUK_TVAL_GET_LIGHTFUNC_FLAGS(tv_func);
		nargs = DUK_LFUNC_FLAGS_GET_NARGS(lf_flags);
		if (nargs == DUK_LFUNC_NARGS_VARARGS) {
			nargs = -1;  /* vararg */
		}
		nregs = nargs;
	} else if (DUK_HOBJECT_IS_COMPILEDFUNCTION(func)) {
		nargs = ((duk_hcompiledfunction *) func)->nargs;
		nregs = ((duk_hcompiledfunction *) func)->nregs;
		DUK_ASSERT(nregs >= nargs);
	} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(func)) {
		/* Note: nargs (and nregs) may be negative for a native,
		 * function, which indicates that the function wants the
		 * input stack "as is" (i.e. handles "vararg" arguments).
		 */
		nargs = ((duk_hnativefunction *) func)->nargs;
		nregs = nargs;
	} else {
		/* XXX: this should be an assert */
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_CALLABLE);
	}

	/* [ ... func this arg1 ... argN ] */

	/*
	 *  Setup a preliminary activation.
	 *
	 *  Don't touch valstack_bottom or valstack_top yet so that Duktape API
	 *  calls work normally.
	 */

	duk_hthread_callstack_grow(thr);

	if (thr->callstack_top > 0) {
		/*
		 *  Update idx_retval of current activation.
		 *
		 *  Although it might seem this is not necessary (bytecode executor
		 *  does this for Ecmascript-to-Ecmascript calls; other calls are
		 *  handled here), this turns out to be necessary for handling yield
		 *  and resume.  For them, an Ecmascript-to-native call happens, and
		 *  the Ecmascript call's idx_retval must be set for things to work.
		 */

		(thr->callstack + thr->callstack_top - 1)->idx_retval = entry_valstack_bottom_index + idx_func;
	}

	DUK_ASSERT(thr->callstack_top < thr->callstack_size);
	act = thr->callstack + thr->callstack_top;
	thr->callstack_top++;
	DUK_ASSERT(thr->callstack_top <= thr->callstack_size);
	DUK_ASSERT(thr->valstack_top > thr->valstack_bottom);  /* at least effective 'this' */
	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUND(func));

	act->flags = 0;
	if (func == NULL || DUK_HOBJECT_HAS_STRICT(func)) {
		act->flags |= DUK_ACT_FLAG_STRICT;
	}
	if (call_flags & DUK_CALL_FLAG_CONSTRUCTOR_CALL) {
		act->flags |= DUK_ACT_FLAG_CONSTRUCT;
		/*act->flags |= DUK_ACT_FLAG_PREVENT_YIELD;*/
	}
	if (func == NULL || DUK_HOBJECT_IS_NATIVEFUNCTION(func)) {
		/*act->flags |= DUK_ACT_FLAG_PREVENT_YIELD;*/
	}
	if (call_flags & DUK_CALL_FLAG_DIRECT_EVAL) {
		act->flags |= DUK_ACT_FLAG_DIRECT_EVAL;
	}

	/* As a first approximation, all calls except Ecmascript-to-Ecmascript
	 * calls prevent a yield.
	 */
	act->flags |= DUK_ACT_FLAG_PREVENT_YIELD;

	act->func = func;  /* NULL for lightfunc */
	act->var_env = NULL;
	act->lex_env = NULL;
#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
	act->prev_caller = NULL;
#endif
	act->pc = 0;
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	act->prev_line = 0;
#endif
	act->idx_bottom = entry_valstack_bottom_index + idx_args;
#if 0  /* topmost activation idx_retval is considered garbage, no need to init */
	act->idx_retval = 0;
#endif
	DUK_TVAL_SET_TVAL(&act->tv_func, tv_func);  /* borrowed, no refcount */

	if (act->flags & DUK_ACT_FLAG_PREVENT_YIELD) {
		/* duk_hthread_callstack_unwind() will decrease this on unwind */
		thr->callstack_preventcount++;
	}

	/* XXX: Is this INCREF necessary? 'func' is always a borrowed
	 * reference reachable through the value stack?  If changed, stack
	 * unwind code also needs to be fixed to match.
	 */
	DUK_HOBJECT_INCREF_ALLOWNULL(thr, func);  /* act->func */

#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
	if (func) {
		duk__update_func_caller_prop(thr, func);
	}
	act = thr->callstack + thr->callstack_top - 1;
#endif

	/* [... func this arg1 ... argN] */

	/*
	 *  Environment record creation and 'arguments' object creation.
	 *  Named function expression name binding is handled by the
	 *  compiler; the compiled function's parent env will contain
	 *  the (immutable) binding already.
	 *
	 *  This handling is now identical for C and Ecmascript functions.
	 *  C functions always have the 'NEWENV' flag set, so their
	 *  environment record initialization is delayed (which is good).
	 *
	 *  Delayed creation (on demand) is handled in duk_js_var.c.
	 */

	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUND(func));  /* bound function chain has already been resolved */

	if (func != NULL && !DUK_HOBJECT_HAS_NEWENV(func)) {
		/* use existing env (e.g. for non-strict eval); cannot have
		 * an own 'arguments' object (but can refer to the existing one)
		 */

		DUK_ASSERT(!DUK_HOBJECT_HAS_CREATEARGS(func));

		duk__handle_oldenv_for_call(thr, func, act);

		DUK_ASSERT(act->lex_env != NULL);
		DUK_ASSERT(act->var_env != NULL);
		goto env_done;
	}

	DUK_ASSERT(func == NULL || DUK_HOBJECT_HAS_NEWENV(func));

	if (func == NULL || !DUK_HOBJECT_HAS_CREATEARGS(func)) {
		/* no need to create environment record now; leave as NULL */
		DUK_ASSERT(act->lex_env == NULL);
		DUK_ASSERT(act->var_env == NULL);
		goto env_done;
	}

	/* third arg: absolute index (to entire valstack) of idx_bottom of new activation */
	env = duk_create_activation_environment_record(thr, func, act->idx_bottom);
	DUK_ASSERT(env != NULL);

	/* [... func this arg1 ... argN envobj] */

	DUK_ASSERT(DUK_HOBJECT_HAS_CREATEARGS(func));
	duk__handle_createargs_for_call(thr, func, env, num_stack_args);

	/* [... func this arg1 ... argN envobj] */

	act->lex_env = env;
	act->var_env = env;
	DUK_HOBJECT_INCREF(thr, env);
	DUK_HOBJECT_INCREF(thr, env);  /* XXX: incref by count (2) directly */
	duk_pop(ctx);

 env_done:
	/* [... func this arg1 ... argN] */

	/*
	 *  Setup value stack: clamp to 'nargs', fill up to 'nregs'
	 *
	 *  Value stack may either grow or shrink, depending on the
	 *  number of func registers and the number of actual arguments.
	 *  If nregs >= 0, func wants args clamped to 'nargs'; else it
	 *  wants all args (= 'num_stack_args').
	 */

	duk__adjust_valstack_and_top(thr,
	                             num_stack_args,
	                             idx_args,
	                             nregs,
	                             nargs,
	                             func);

	/*
	 *  Determine call type; then setup activation and call
	 */

	if (func != NULL && DUK_HOBJECT_IS_COMPILEDFUNCTION(func)) {
		goto ecmascript_call;
	} else {
		goto native_call;
	}
	DUK_UNREACHABLE();

	/*
	 *  Native (C) call
	 */

 native_call:
	/*
	 *  Shift to new valstack_bottom.
	 */

	thr->valstack_bottom = thr->valstack_bottom + idx_args;
	/* keep current valstack_top */
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
	DUK_ASSERT(func == NULL || ((duk_hnativefunction *) func)->func != NULL);

	/* [... func this | arg1 ... argN] ('this' must precede new bottom) */

	/*
	 *  Actual function call and return value check.
	 *
	 *  Return values:
	 *    0    success, no return value (default to 'undefined')
	 *    1    success, one return value on top of stack
	 *  < 0    error, throw a "magic" error
	 *  other  invalid
	 */

	if (func) {
		rc = ((duk_hnativefunction *) func)->func((duk_context *) thr);
	} else {
		duk_c_function funcptr = DUK_TVAL_GET_LIGHTFUNC_FUNCPTR(tv_func);
		rc = funcptr((duk_context *) thr);
	}

	if (rc < 0) {
		duk_error_throw_from_negative_rc(thr, rc);
		DUK_UNREACHABLE();
	} else if (rc > 1) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "c function returned invalid rc");
	}
	DUK_ASSERT(rc == 0 || rc == 1);

	/*
	 *  Unwind stack(s) and shift back to old valstack_bottom.
	 */

	DUK_ASSERT(thr->catchstack_top == entry_catchstack_top);
	DUK_ASSERT(thr->callstack_top == entry_callstack_top + 1);

#if 0  /* should be no need to unwind */
	duk_hthread_catchstack_unwind(thr, entry_catchstack_top);
#endif
	duk_hthread_callstack_unwind(thr, entry_callstack_top);

	thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;
	/* keep current valstack_top */

	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
	DUK_ASSERT(thr->valstack_top - thr->valstack_bottom >= idx_func + 1);

	/*
	 *  Manipulate value stack so that return value is on top
	 *  (pushing an 'undefined' if necessary).
	 */

	/* XXX: should this happen in the callee's activation or after unwinding? */
	if (rc == 0) {
		duk_require_stack(ctx, 1);
		duk_push_undefined(ctx);
	}
	/* [... func this (crud) retval] */

	DUK_DDD(DUK_DDDPRINT("native call retval -> %!T (rc=%ld)",
	                     (duk_tval *) duk_get_tval(ctx, -1), (long) rc));

	duk_replace(ctx, idx_func);
	duk_set_top(ctx, idx_func + 1);

	/* [... retval] */

	/* Ensure there is internal valstack spare before we exit; this may
	 * throw an alloc error.  The same guaranteed size must be available
	 * as before the call.  This is not optimal now: we store the valstack
	 * allocated size during entry; this value may be higher than the
	 * minimal guarantee for an application.
	 */

	(void) duk_valstack_resize_raw((duk_context *) thr,
	                               entry_valstack_end,                    /* same as during entry */
	                               DUK_VSRESIZE_FLAG_SHRINK |             /* flags */
	                               DUK_VSRESIZE_FLAG_COMPACT |
	                               DUK_VSRESIZE_FLAG_THROW);


	/*
	 *  Shrink checks and return with success.
	 */

	retval = DUK_EXEC_SUCCESS;
	goto shrink_and_finished;

	/*
	 *  Ecmascript call
	 */

 ecmascript_call:

	/*
	 *  Shift to new valstack_bottom.
	 */

	thr->valstack_bottom = thr->valstack_bottom + idx_args;
	/* keep current valstack_top */
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	/* [... func this | arg1 ... argN] ('this' must precede new bottom) */

	/*
	 *  Bytecode executor call.
	 *
	 *  Execute bytecode, handling any recursive function calls and
	 *  thread resumptions.  Returns when execution would return from
	 *  the entry level activation.  When the executor returns, a
	 *  single return value is left on the stack top.
	 *
	 *  The only possible longjmp() is an error (DUK_LJ_TYPE_THROW),
	 *  other types are handled internally by the executor.
	 *
	 */

	DUK_DDD(DUK_DDDPRINT("entering bytecode execution"));
	duk_js_execute_bytecode(thr);
	DUK_DDD(DUK_DDDPRINT("returned from bytecode execution"));

	/*
	 *  Unwind stack(s) and shift back to old valstack_bottom.
	 */

	DUK_ASSERT(thr->callstack_top == entry_callstack_top + 1);

	duk_hthread_catchstack_unwind(thr, entry_catchstack_top);
	duk_hthread_callstack_unwind(thr, entry_callstack_top);

	thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;
	/* keep current valstack_top */

	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
	DUK_ASSERT(thr->valstack_top - thr->valstack_bottom >= idx_func + 1);

	/*
	 *  Manipulate value stack so that return value is on top.
	 */

	/* [... func this (crud) retval] */

	duk_replace(ctx, idx_func);
	duk_set_top(ctx, idx_func + 1);

	/* [... retval] */

	/* Ensure there is internal valstack spare before we exit; this may
	 * throw an alloc error.  The same guaranteed size must be available
	 * as before the call.  This is not optimal now: we store the valstack
	 * allocated size during entry; this value may be higher than the
	 * minimal guarantee for an application.
	 */

	(void) duk_valstack_resize_raw((duk_context *) thr,
	                               entry_valstack_end,                    /* same as during entry */
	                               DUK_VSRESIZE_FLAG_SHRINK |             /* flags */
	                               DUK_VSRESIZE_FLAG_COMPACT |
	                               DUK_VSRESIZE_FLAG_THROW);

	/*
	 *  Shrink checks and return with success.
	 */

	retval = DUK_EXEC_SUCCESS;
	goto shrink_and_finished;

 shrink_and_finished:
#if defined(DUK_OPT_FASTINT)
	/* Explicit check for fastint downgrade. */
	{
		duk_tval *tv_fi;
		tv_fi = duk_get_tval(ctx, -1);
		DUK_ASSERT(tv_fi != NULL);
		DUK_TVAL_CHKFAST_INPLACE(tv_fi);
	}
#endif

	/* these are "soft" shrink checks, whose failures are ignored */
	/* XXX: would be nice if fast path was inlined */
	duk_hthread_catchstack_shrink_check(thr);
	duk_hthread_callstack_shrink_check(thr);
	goto finished;

 finished:
	if (need_setjmp) {
		/* Note: either pointer may be NULL (at entry), so don't assert;
		 * this is now done potentially twice, which is OK
		 */
		DUK_DDD(DUK_DDDPRINT("restore jmpbuf_ptr: %p -> %p (possibly already done)",
		                     (void *) (thr && thr->heap ? thr->heap->lj.jmpbuf_ptr : NULL),
		                     (void *) old_jmpbuf_ptr));
		thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

		/* These are just convenience "wiping" of state */
		thr->heap->lj.type = DUK_LJ_TYPE_UNKNOWN;
		thr->heap->lj.iserror = 0;

		/* Side effects should not be an issue here: tv_tmp is local and
		 * thr->heap (and thr->heap->lj) have a stable pointer.  Finalizer
		 * runs etc capture even out-of-memory errors so nothing should
		 * throw here.
		 */
		DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value1);
		DUK_TVAL_SET_UNDEFINED_UNUSED(&thr->heap->lj.value1);
		DUK_TVAL_DECREF(thr, &tv_tmp);

		DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value2);
		DUK_TVAL_SET_UNDEFINED_UNUSED(&thr->heap->lj.value2);
		DUK_TVAL_DECREF(thr, &tv_tmp);

		DUK_DDD(DUK_DDDPRINT("setjmp catchpoint torn down"));
	}

	DUK_HEAP_SWITCH_THREAD(thr->heap, entry_curr_thread);  /* may be NULL */
	thr->state = (duk_uint8_t) entry_thread_state;

	DUK_ASSERT((thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread == NULL) ||  /* first call */
	           (thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread != NULL) ||  /* other call */
	           (thr->state == DUK_HTHREAD_STATE_RUNNING && thr->heap->curr_thread == thr));     /* current thread */

	thr->heap->call_recursion_depth = entry_call_recursion_depth;

	return retval;

 thread_state_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid thread state for call (%ld)", (long) thr->state);
	DUK_UNREACHABLE();
	return DUK_EXEC_ERROR;  /* never executed */
}

/*
 *  Manipulate value stack so that exactly 'num_stack_rets' return
 *  values are at 'idx_retbase' in every case, assuming there are
 *  'rc' return values on top of stack.
 *
 *  This is a bit tricky, because the called C function operates in
 *  the same activation record and may have e.g. popped the stack
 *  empty (below idx_retbase).
 */

DUK_LOCAL void duk__safe_call_adjust_valstack(duk_hthread *thr, duk_idx_t idx_retbase, duk_idx_t num_stack_rets, duk_idx_t num_actual_rets) {
	duk_context *ctx = (duk_context *) thr;
	duk_idx_t idx_rcbase;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(idx_retbase >= 0);
	DUK_ASSERT(num_stack_rets >= 0);
	DUK_ASSERT(num_actual_rets >= 0);

	idx_rcbase = duk_get_top(ctx) - num_actual_rets;  /* base of known return values */

	DUK_DDD(DUK_DDDPRINT("adjust valstack after func call: "
	                     "num_stack_rets=%ld, num_actual_rets=%ld, stack_top=%ld, idx_retbase=%ld, idx_rcbase=%ld",
	                     (long) num_stack_rets, (long) num_actual_rets, (long) duk_get_top(ctx),
	                     (long) idx_retbase, (long) idx_rcbase));

	DUK_ASSERT(idx_rcbase >= 0);  /* caller must check */

	/* ensure space for final configuration (idx_retbase + num_stack_rets) and
	 * intermediate configurations
	 */
	duk_require_stack_top(ctx,
	                      (idx_rcbase > idx_retbase ? idx_rcbase : idx_retbase) +
	                      num_stack_rets);

	/* chop extra retvals away / extend with undefined */
	duk_set_top(ctx, idx_rcbase + num_stack_rets);

	if (idx_rcbase >= idx_retbase) {
		duk_idx_t count = idx_rcbase - idx_retbase;
		duk_idx_t i;

		DUK_DDD(DUK_DDDPRINT("elements at/after idx_retbase have enough to cover func retvals "
		                     "(idx_retbase=%ld, idx_rcbase=%ld)", (long) idx_retbase, (long) idx_rcbase));

		/* nuke values at idx_retbase to get the first retval (initially
		 * at idx_rcbase) to idx_retbase
		 */

		DUK_ASSERT(count >= 0);

		for (i = 0; i < count; i++) {
			/* XXX: inefficient; block remove primitive */
			duk_remove(ctx, idx_retbase);
		}
	} else {
		duk_idx_t count = idx_retbase - idx_rcbase;
		duk_idx_t i;

		DUK_DDD(DUK_DDDPRINT("not enough elements at/after idx_retbase to cover func retvals "
		                     "(idx_retbase=%ld, idx_rcbase=%ld)", (long) idx_retbase, (long) idx_rcbase));

		/* insert 'undefined' values at idx_rcbase to get the
		 * return values to idx_retbase
		 */

		DUK_ASSERT(count > 0);

		for (i = 0; i < count; i++) {
			/* XXX: inefficient; block insert primitive */
			duk_push_undefined(ctx);
			duk_insert(ctx, idx_rcbase);
		}
	}
}

/*
 *  Make a "C protected call" within the current activation.
 *
 *  The allowed thread states for making a call are the same as for
 *  duk_handle_call().
 *
 *  Note that like duk_handle_call(), even if this call is protected,
 *  there are a few situations where the current (pre-entry) setjmp
 *  catcher (or a fatal error handler if no such catcher exists) is
 *  invoked:
 *
 *    - Blatant API argument errors (e.g. num_stack_args is invalid,
 *      so we can't form a reasonable return stack)
 *
 *    - Errors during error handling, e.g. failure to reallocate
 *      space in the value stack due to an alloc error
 *
 *  Such errors propagate outwards, ultimately to the fatal error
 *  handler if nothing else.
 */

/* XXX: bump preventcount by one for the duration of this call? */

DUK_INTERNAL
duk_int_t duk_handle_safe_call(duk_hthread *thr,
                               duk_safe_call_function func,
                               duk_idx_t num_stack_args,
                               duk_idx_t num_stack_rets) {
	duk_context *ctx = (duk_context *) thr;
	duk_size_t entry_valstack_bottom_index;
	duk_size_t entry_callstack_top;
	duk_size_t entry_catchstack_top;
	duk_int_t entry_call_recursion_depth;
	duk_hthread *entry_curr_thread;
	duk_uint_fast8_t entry_thread_state;
	duk_jmpbuf *old_jmpbuf_ptr = NULL;
	duk_jmpbuf our_jmpbuf;
	duk_tval tv_tmp;
	duk_idx_t idx_retbase;
	duk_int_t retval;
	duk_ret_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	/* Note: careful with indices like '-x'; if 'x' is zero, it refers to bottom */
	entry_valstack_bottom_index = (duk_size_t) (thr->valstack_bottom - thr->valstack);
	entry_callstack_top = thr->callstack_top;
	entry_catchstack_top = thr->catchstack_top;
	entry_call_recursion_depth = thr->heap->call_recursion_depth;
	entry_curr_thread = thr->heap->curr_thread;  /* Note: may be NULL if first call */
	entry_thread_state = thr->state;
	idx_retbase = duk_get_top(ctx) - num_stack_args;  /* Note: not a valid stack index if num_stack_args == 0 */

	/* Note: cannot portably debug print a function pointer, hence 'func' not printed! */
	DUK_DD(DUK_DDPRINT("duk_handle_safe_call: thr=%p, num_stack_args=%ld, num_stack_rets=%ld, "
	                   "valstack_top=%ld, idx_retbase=%ld, rec_depth=%ld/%ld, "
	                   "entry_valstack_bottom_index=%ld, entry_callstack_top=%ld, entry_catchstack_top=%ld, "
	                   "entry_call_recursion_depth=%ld, entry_curr_thread=%p, entry_thread_state=%ld",
	                   (void *) thr,
	                   (long) num_stack_args,
	                   (long) num_stack_rets,
	                   (long) duk_get_top(ctx),
	                   (long) idx_retbase,
	                   (long) thr->heap->call_recursion_depth,
	                   (long) thr->heap->call_recursion_limit,
	                   (long) entry_valstack_bottom_index,
	                   (long) entry_callstack_top,
	                   (long) entry_catchstack_top,
	                   (long) entry_call_recursion_depth,
	                   (void *) entry_curr_thread,
	                   (long) entry_thread_state));

	if (idx_retbase < 0) {
		/*
		 *  Since stack indices are not reliable, we can't do anything useful
		 *  here.  Invoke the existing setjmp catcher, or if it doesn't exist,
		 *  call the fatal error handler.
		 */

		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CALL_ARGS);
	}

	/* setjmp catchpoint setup */

	old_jmpbuf_ptr = thr->heap->lj.jmpbuf_ptr;
	thr->heap->lj.jmpbuf_ptr = &our_jmpbuf;

	if (DUK_SETJMP(thr->heap->lj.jmpbuf_ptr->jb) == 0) {
		goto handle_call;
	}

	/*
	 *  Error during call.  The error value is at heap->lj.value1.
	 *
	 *  Careful with variable accesses here; must be assigned to before
	 *  setjmp() or be declared volatile.  See duk_handle_call().
	 *
	 *  The following are such variables:
	 *    - duk_handle_safe_call() parameters
	 *    - entry_*
	 *    - idx_retbase
	 *
	 *  The very first thing we do is restore the previous setjmp catcher.
	 *  This means that any error in error handling will propagate outwards
	 *  instead of causing a setjmp() re-entry above.  The *only* actual
	 *  errors that should happen here are allocation errors.
	 */

	DUK_DDD(DUK_DDDPRINT("error caught during protected duk_handle_safe_call()"));

	DUK_ASSERT(thr->heap->lj.type == DUK_LJ_TYPE_THROW);
	DUK_ASSERT(thr->callstack_top >= entry_callstack_top);
	DUK_ASSERT(thr->catchstack_top >= entry_catchstack_top);

	/* Note: either pointer may be NULL (at entry), so don't assert;
	 * these are now restored twice which is OK.
	 */
	thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

	duk_hthread_catchstack_unwind(thr, entry_catchstack_top);
	duk_hthread_callstack_unwind(thr, entry_callstack_top);
	thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;

	/* [ ... | (crud) ] */

	/* XXX: space in valstack?  see discussion in duk_handle_call. */
	duk_push_tval(ctx, &thr->heap->lj.value1);

	/* [ ... | (crud) errobj ] */

	DUK_ASSERT(duk_get_top(ctx) >= 1);  /* at least errobj must be on stack */

	/* check that the valstack has space for the final amount and any
	 * intermediate space needed; this is unoptimal but should be safe
	 */
	duk_require_stack_top(ctx, idx_retbase + num_stack_rets);  /* final configuration */
	duk_require_stack(ctx, num_stack_rets);

	duk__safe_call_adjust_valstack(thr, idx_retbase, num_stack_rets, 1);  /* 1 = num actual 'return values' */

	/* [ ... | ] or [ ... | errobj (M * undefined)] where M = num_stack_rets - 1 */

	retval = DUK_EXEC_ERROR;
	goto shrink_and_finished;

	/*
	 *  Handle call (inside setjmp)
	 */

 handle_call:

	DUK_DDD(DUK_DDDPRINT("safe_call setjmp catchpoint setup complete"));

	/*
	 *  Thread state check and book-keeping.
	 */

	if (thr == thr->heap->curr_thread) {
		/* same thread */
		if (thr->state != DUK_HTHREAD_STATE_RUNNING) {
			/* should actually never happen, but check anyway */
			goto thread_state_error;
		}
	} else {
		/* different thread */
		DUK_ASSERT(thr->heap->curr_thread == NULL ||
		           thr->heap->curr_thread->state == DUK_HTHREAD_STATE_RUNNING);
		if (thr->state != DUK_HTHREAD_STATE_INACTIVE) {
			goto thread_state_error;
		}
		DUK_HEAP_SWITCH_THREAD(thr->heap, thr);
		thr->state = DUK_HTHREAD_STATE_RUNNING;

		/* Note: multiple threads may be simultaneously in the RUNNING
		 * state, but not in the same "resume chain".
		 */
	}

	DUK_ASSERT(thr->heap->curr_thread == thr);
	DUK_ASSERT(thr->state == DUK_HTHREAD_STATE_RUNNING);

	/*
	 *  Recursion limit check.
	 *
	 *  Note: there is no need for an "ignore recursion limit" flag
	 *  for duk_handle_safe_call now.
	 */

	DUK_ASSERT(thr->heap->call_recursion_depth >= 0);
	DUK_ASSERT(thr->heap->call_recursion_depth <= thr->heap->call_recursion_limit);
	if (thr->heap->call_recursion_depth >= thr->heap->call_recursion_limit) {
		/* XXX: error message is a bit misleading: we reached a recursion
		 * limit which is also essentially the same as a C callstack limit
		 * (except perhaps with some relaxed threading assumptions).
		 */
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_C_CALLSTACK_LIMIT);
	}
	thr->heap->call_recursion_depth++;

	/*
	 *  Valstack spare check
	 */

	duk_require_stack(ctx, 0);  /* internal spare */

	/*
	 *  Make the C call
	 */

	rc = func(ctx);

	DUK_DDD(DUK_DDDPRINT("safe_call, func rc=%ld", (long) rc));

	/*
	 *  Valstack manipulation for results
	 */

	/* we're running inside the caller's activation, so no change in call/catch stack or valstack bottom */
	DUK_ASSERT(thr->callstack_top == entry_callstack_top);
	DUK_ASSERT(thr->catchstack_top == entry_catchstack_top);
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT((duk_size_t) (thr->valstack_bottom - thr->valstack) == entry_valstack_bottom_index);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	if (rc < 0) {
		duk_error_throw_from_negative_rc(thr, rc);
	}
	DUK_ASSERT(rc >= 0);

	if (duk_get_top(ctx) < rc) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "not enough stack values for safe_call rc");
	}

	duk__safe_call_adjust_valstack(thr, idx_retbase, num_stack_rets, rc);

	/* Note: no need from callstack / catchstack shrink check */
	retval = DUK_EXEC_SUCCESS;
	goto finished;

 shrink_and_finished:
	/* these are "soft" shrink checks, whose failures are ignored */
	/* XXX: would be nice if fast path was inlined */
	duk_hthread_catchstack_shrink_check(thr);
	duk_hthread_callstack_shrink_check(thr);
	goto finished;

 finished:
	/* Note: either pointer may be NULL (at entry), so don't assert */
	thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

	/* These are just convenience "wiping" of state */
	thr->heap->lj.type = DUK_LJ_TYPE_UNKNOWN;
	thr->heap->lj.iserror = 0;

	/* Side effects should not be an issue here: tv_tmp is local and
	 * thr->heap (and thr->heap->lj) have a stable pointer.  Finalizer
	 * runs etc capture even out-of-memory errors so nothing should
	 * throw here.
	 */
	DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value1);
	DUK_TVAL_SET_UNDEFINED_UNUSED(&thr->heap->lj.value1);
	DUK_TVAL_DECREF(thr, &tv_tmp);

	DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value2);
	DUK_TVAL_SET_UNDEFINED_UNUSED(&thr->heap->lj.value2);
	DUK_TVAL_DECREF(thr, &tv_tmp);

	DUK_DDD(DUK_DDDPRINT("setjmp catchpoint torn down"));

	/* XXX: because we unwind stacks above, thr->heap->curr_thread is at
	 * risk of pointing to an already freed thread.  This was indeed the
	 * case in test-bug-multithread-valgrind.c, until duk_handle_call()
	 * was fixed to restore thr->heap->curr_thread before rethrowing an
	 * uncaught error.
	 */
	DUK_HEAP_SWITCH_THREAD(thr->heap, entry_curr_thread);  /* may be NULL */
	thr->state = (duk_uint8_t) entry_thread_state;

	DUK_ASSERT((thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread == NULL) ||  /* first call */
	           (thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread != NULL) ||  /* other call */
	           (thr->state == DUK_HTHREAD_STATE_RUNNING && thr->heap->curr_thread == thr));     /* current thread */

	thr->heap->call_recursion_depth = entry_call_recursion_depth;

	/* stack discipline consistency check */
	DUK_ASSERT(duk_get_top(ctx) == idx_retbase + num_stack_rets);

	return retval;

 thread_state_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid thread state for safe_call (%ld)", (long) thr->state);
	DUK_UNREACHABLE();
	return DUK_EXEC_ERROR;  /* never executed */
}

/*
 *  Helper for handling an Ecmascript-to-Ecmascript call or an Ecmascript
 *  function (initial) Duktape.Thread.resume().
 *
 *  Compared to normal calls handled by duk_handle_call(), there are a
 *  bunch of differences:
 *
 *    - the call is never protected
 *    - there is no C recursion depth increase (hence an "ignore recursion
 *      limit" flag is not applicable)
 *    - instead of making the call, this helper just performs the thread
 *      setup and returns; the bytecode executor then restarts execution
 *      internally
 *    - ecmascript functions are never 'vararg' functions (they access
 *      varargs through the 'arguments' object)
 *
 *  The callstack of the target contains an earlier Ecmascript call in case
 *  of an Ecmascript-to-Ecmascript call (whose idx_retval is updated), or
 *  is empty in case of an initial Duktape.Thread.resume().
 *
 *  The first thing to do here is to figure out whether an ecma-to-ecma
 *  call is actually possible.  It's not always the case if the target is
 *  a bound function; the final function may be native.  In that case,
 *  return an error so caller can fall back to a normal call path.
 */

DUK_INTERNAL
duk_bool_t duk_handle_ecma_call_setup(duk_hthread *thr,
                                      duk_idx_t num_stack_args,
                                      duk_small_uint_t call_flags) {
	duk_context *ctx = (duk_context *) thr;
	duk_size_t entry_valstack_bottom_index;
	duk_idx_t idx_func;     /* valstack index of 'func' and retval (relative to entry valstack_bottom) */
	duk_idx_t idx_args;     /* valstack index of start of args (arg1) (relative to entry valstack_bottom) */
	duk_idx_t nargs;        /* # argument registers target function wants (< 0 => never for ecma calls) */
	duk_idx_t nregs;        /* # total registers target function wants on entry (< 0 => never for ecma calls) */
	duk_hobject *func;      /* 'func' on stack (borrowed reference) */
	duk_tval *tv_func;      /* duk_tval ptr for 'func' on stack (borrowed reference) */
	duk_activation *act;
	duk_hobject *env;
	duk_bool_t use_tailcall;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(!((call_flags & DUK_CALL_FLAG_IS_RESUME) != 0 && (call_flags & DUK_CALL_FLAG_IS_TAILCALL) != 0));

	/* XXX: assume these? */
	DUK_ASSERT(thr->valstack != NULL);
	DUK_ASSERT(thr->callstack != NULL);
	DUK_ASSERT(thr->catchstack != NULL);

	/* no need to handle thread state book-keeping here */
	DUK_ASSERT((call_flags & DUK_CALL_FLAG_IS_RESUME) != 0 ||
	           (thr->state == DUK_HTHREAD_STATE_RUNNING &&
	            thr->heap->curr_thread == thr));

	/* if a tailcall:
	 *   - an Ecmascript activation must be on top of the callstack
	 *   - there cannot be any active catchstack entries
	 */
#ifdef DUK_USE_ASSERTIONS
	if (call_flags & DUK_CALL_FLAG_IS_TAILCALL) {
		duk_size_t our_callstack_index;
		duk_size_t i;

		DUK_ASSERT(thr->callstack_top >= 1);
		our_callstack_index = thr->callstack_top - 1;
		DUK_ASSERT_DISABLE(our_callstack_index >= 0);
		DUK_ASSERT(our_callstack_index < thr->callstack_size);
		DUK_ASSERT(DUK_ACT_GET_FUNC(thr->callstack + our_callstack_index) != NULL);
		DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION(DUK_ACT_GET_FUNC(thr->callstack + our_callstack_index)));

		/* No entry in the catchstack which would actually catch a
		 * throw can refer to the callstack entry being reused.
		 * There *can* be catchstack entries referring to the current
		 * callstack entry as long as they don't catch (e.g. label sites).
		 */

		for (i = 0; i < thr->catchstack_top; i++) {
			DUK_ASSERT(thr->catchstack[i].callstack_index < our_callstack_index ||  /* refer to callstack entries below current */
			           DUK_CAT_GET_TYPE(thr->catchstack + i) == DUK_CAT_TYPE_LABEL); /* or a non-catching entry */
		}
	}
#endif  /* DUK_USE_ASSERTIONS */

	entry_valstack_bottom_index = (duk_size_t) (thr->valstack_bottom - thr->valstack);
	idx_func = duk_normalize_index(thr, -num_stack_args - 2);
	idx_args = idx_func + 2;

	DUK_DD(DUK_DDPRINT("handle_ecma_call_setup: thr=%p, "
	                   "num_stack_args=%ld, call_flags=0x%08lx (resume=%ld, tailcall=%ld), "
	                   "idx_func=%ld, idx_args=%ld, entry_valstack_bottom_index=%ld",
	                   (void *) thr,
	                   (long) num_stack_args,
	                   (unsigned long) call_flags,
	                   (long) ((call_flags & DUK_CALL_FLAG_IS_RESUME) != 0 ? 1 : 0),
	                   (long) ((call_flags & DUK_CALL_FLAG_IS_TAILCALL) != 0 ? 1 : 0),
	                   (long) idx_func,
	                   (long) idx_args,
	                   (long) entry_valstack_bottom_index));

	if (idx_func < 0 || idx_args < 0) {
		/* XXX: assert? compiler is responsible for this never happening */
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CALL_ARGS);
	}

	/*
	 *  Check the function type, handle bound function chains, and prepare
	 *  parameters for the rest of the call handling.  Also figure out the
	 *  effective 'this' binding, which replaces the current value at
	 *  idx_func + 1.
	 *
	 *  If the target function is a 'bound' one, follow the chain of 'bound'
	 *  functions until a non-bound function is found.  During this process,
	 *  bound arguments are 'prepended' to existing ones, and the "this"
	 *  binding is overridden.  See E5 Section 15.3.4.5.1.
	 *
	 *  If the final target function cannot be handled by an ecma-to-ecma
	 *  call, return to the caller with a return value indicating this case.
	 *  The bound chain is resolved and the caller can resume with a plain
	 *  function call.
	 */

	func = duk__nonbound_func_lookup(ctx, idx_func, &num_stack_args, &tv_func, call_flags);
	if (func == NULL || !DUK_HOBJECT_IS_COMPILEDFUNCTION(func)) {
		DUK_DDD(DUK_DDDPRINT("final target is a lightfunc/nativefunc, cannot do ecma-to-ecma call"));
		return 0;
	}
	/* XXX: tv_func is not actually needed */

	DUK_ASSERT(func != NULL);
	DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(func));
	DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION(func));

	duk__coerce_effective_this_binding(thr, func, idx_func + 1);
	DUK_DDD(DUK_DDDPRINT("effective 'this' binding is: %!T",
	                     duk_get_tval(ctx, idx_func + 1)));

	nargs = ((duk_hcompiledfunction *) func)->nargs;
	nregs = ((duk_hcompiledfunction *) func)->nregs;
	DUK_ASSERT(nregs >= nargs);

	/* [ ... func this arg1 ... argN ] */

	/*
	 *  Preliminary activation record and valstack manipulation.
	 *  The concrete actions depend on whether the we're dealing
	 *  with a tailcall (reuse an existing activation), a resume,
	 *  or a normal call.
	 *
	 *  The basic actions, in varying order, are:
	 *
	 *    - Check stack size for call handling
	 *    - Grow call stack if necessary (non-tail-calls)
	 *    - Update current activation (idx_retval) if necessary
	 *      (non-tail, non-resume calls)
	 *    - Move start of args (idx_args) to valstack bottom
	 *      (tail calls)
	 *
	 *  Don't touch valstack_bottom or valstack_top yet so that Duktape API
	 *  calls work normally.
	 */

	/* XXX: some overlapping code; cleanup */
	use_tailcall = call_flags & DUK_CALL_FLAG_IS_TAILCALL;
#if !defined(DUK_USE_TAILCALL)
	DUK_ASSERT(use_tailcall == 0);  /* compiler ensures this */
#endif
	if (use_tailcall) {
		/* tailcall cannot be flagged to resume calls, and a
		 * previous frame must exist
		 */
		DUK_ASSERT(thr->callstack_top >= 1);
		DUK_ASSERT((call_flags & DUK_CALL_FLAG_IS_RESUME) == 0);

		act = thr->callstack + thr->callstack_top - 1;
		if (act->flags & DUK_ACT_FLAG_PREVENT_YIELD) {
			/* See: test-bug-tailcall-preventyield-assert.c. */
			DUK_DDD(DUK_DDDPRINT("tailcall prevented by current activation having DUK_ACT_FLAG_PREVENTYIELD"));
			use_tailcall = 0;
		} else if (DUK_HOBJECT_HAS_NOTAIL(func)) {
			DUK_D(DUK_DPRINT("tailcall prevented by function having a notail flag"));
			use_tailcall = 0;
		}
	}

	if (use_tailcall) {
		duk_tval *tv1, *tv2;
		duk_tval tv_tmp;
		duk_size_t cs_index;
		duk_int_t i_stk;  /* must be signed for loop structure */
		duk_idx_t i_arg;

		/*
		 *  Tailcall handling
		 *
		 *  Although the callstack entry is reused, we need to explicitly unwind
		 *  the current activation (or simulate an unwind).  In particular, the
		 *  current activation must be closed, otherwise something like
		 *  test-bug-reduce-judofyr.js results.  Also catchstack needs be unwound
		 *  because there may be non-error-catching label entries in valid tailcalls.
		 */

		DUK_DDD(DUK_DDDPRINT("is tailcall, reusing activation at callstack top, at index %ld",
		                     (long) (thr->callstack_top - 1)));

		/* 'act' already set above */

		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(func));
		DUK_ASSERT(!DUK_HOBJECT_HAS_NATIVEFUNCTION(func));
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPILEDFUNCTION(func));
		DUK_ASSERT((act->flags & DUK_ACT_FLAG_PREVENT_YIELD) == 0);

		/* Unwind catchstack entries referring to the callstack entry we're reusing */
		cs_index = thr->callstack_top - 1;
		DUK_ASSERT(thr->catchstack_top <= DUK_INT_MAX);  /* catchstack limits */
		for (i_stk = (duk_int_t) (thr->catchstack_top - 1); i_stk >= 0; i_stk--) {
			duk_catcher *cat = thr->catchstack + i_stk;
			if (cat->callstack_index != cs_index) {
				/* 'i' is the first entry we'll keep */
				break;
			}
		}
		duk_hthread_catchstack_unwind(thr, i_stk + 1);

		/* Unwind the topmost callstack entry before reusing it */
		DUK_ASSERT(thr->callstack_top > 0);
		duk_hthread_callstack_unwind(thr, thr->callstack_top - 1);

		/* Then reuse the unwound activation; callstack was not shrunk so there is always space */
		thr->callstack_top++;
		DUK_ASSERT(thr->callstack_top <= thr->callstack_size);
		act = thr->callstack + thr->callstack_top - 1;

		/* Start filling in the activation */
		act->func = func;  /* don't want an intermediate exposed state with func == NULL */
#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
		act->prev_caller = NULL;
#endif
		act->pc = 0;       /* don't want an intermediate exposed state with invalid pc */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
		act->prev_line = 0;
#endif
		DUK_TVAL_SET_OBJECT(&act->tv_func, func);  /* borrowed, no refcount */
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_INCREF(thr, func);
		act = thr->callstack + thr->callstack_top - 1;  /* side effects (currently none though) */
#endif

#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
#ifdef DUK_USE_TAILCALL
#error incorrect options: tailcalls enabled with function caller property
#endif
		/* XXX: this doesn't actually work properly for tail calls, so
		 * tail calls are disabled when DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
		 * is in use.
		 */
		duk__update_func_caller_prop(thr, func);
		act = thr->callstack + thr->callstack_top - 1;
#endif

		act->flags = (DUK_HOBJECT_HAS_STRICT(func) ?
		              DUK_ACT_FLAG_STRICT | DUK_ACT_FLAG_TAILCALLED :
		              DUK_ACT_FLAG_TAILCALLED);

		DUK_ASSERT(DUK_ACT_GET_FUNC(act) == func);      /* already updated */
		DUK_ASSERT(act->var_env == NULL);   /* already NULLed (by unwind) */
		DUK_ASSERT(act->lex_env == NULL);   /* already NULLed (by unwind) */
		DUK_ASSERT(act->pc == 0);           /* already zeroed */
		act->idx_bottom = entry_valstack_bottom_index;  /* tail call -> reuse current "frame" */
		DUK_ASSERT(nregs >= 0);
#if 0  /* topmost activation idx_retval is considered garbage, no need to init */
		act->idx_retval = 0;
#endif

		/*
		 *  Manipulate valstack so that args are on the current bottom and the
		 *  previous caller's 'this' binding (which is the value preceding the
		 *  current bottom) is replaced with the new 'this' binding:
		 *
		 *       [ ... this_old | (crud) func this_new arg1 ... argN ]
		 *  -->  [ ... this_new | arg1 ... argN ]
		 *
		 *  For tailcalling to work properly, the valstack bottom must not grow
		 *  here; otherwise crud would accumulate on the valstack.
		 */

		tv1 = thr->valstack_bottom - 1;
		tv2 = thr->valstack_bottom + idx_func + 1;
		DUK_ASSERT(tv1 >= thr->valstack && tv1 < thr->valstack_top);  /* tv1 is -below- valstack_bottom */
		DUK_ASSERT(tv2 >= thr->valstack_bottom && tv2 < thr->valstack_top);
		DUK_TVAL_SET_TVAL(&tv_tmp, tv1);
		DUK_TVAL_SET_TVAL(tv1, tv2);
		DUK_TVAL_INCREF(thr, tv1);
		DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */

		for (i_arg = 0; i_arg < idx_args; i_arg++) {
			/* XXX: block removal API primitive */
			/* Note: 'func' is popped from valstack here, but it is
			 * already reachable from the activation.
			 */
			duk_remove(ctx, 0);
		}
		idx_func = 0; DUK_UNREF(idx_func);  /* really 'not applicable' anymore, should not be referenced after this */
		idx_args = 0;

		/* [ ... this_new | arg1 ... argN ] */
	} else {
		DUK_DDD(DUK_DDDPRINT("not a tailcall, pushing a new activation to callstack, to index %ld",
		                     (long) (thr->callstack_top)));

		duk_hthread_callstack_grow(thr);

		if (call_flags & DUK_CALL_FLAG_IS_RESUME) {
			DUK_DDD(DUK_DDDPRINT("is resume -> no update to current activation (may not even exist)"));
		} else {
			DUK_DDD(DUK_DDDPRINT("update to current activation idx_retval"));
			DUK_ASSERT(thr->callstack_top < thr->callstack_size);
			DUK_ASSERT(thr->callstack_top >= 1);
			act = thr->callstack + thr->callstack_top - 1;
			DUK_ASSERT(DUK_ACT_GET_FUNC(act) != NULL);
			DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION(DUK_ACT_GET_FUNC(act)));
			act->idx_retval = entry_valstack_bottom_index + idx_func;
		}

		DUK_ASSERT(thr->callstack_top < thr->callstack_size);
		act = thr->callstack + thr->callstack_top;
		thr->callstack_top++;
		DUK_ASSERT(thr->callstack_top <= thr->callstack_size);

		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(func));
		DUK_ASSERT(!DUK_HOBJECT_HAS_NATIVEFUNCTION(func));
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPILEDFUNCTION(func));

		act->flags = (DUK_HOBJECT_HAS_STRICT(func) ?
		              DUK_ACT_FLAG_STRICT :
		              0);
		act->func = func;
		act->var_env = NULL;
		act->lex_env = NULL;
#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
		act->prev_caller = NULL;
#endif
		act->pc = 0;
#if defined(DUK_USE_DEBUGGER_SUPPORT)
		act->prev_line = 0;
#endif
		act->idx_bottom = entry_valstack_bottom_index + idx_args;
		DUK_ASSERT(nregs >= 0);
#if 0  /* topmost activation idx_retval is considered garbage, no need to init */
		act->idx_retval = 0;
#endif
		DUK_TVAL_SET_OBJECT(&act->tv_func, func);  /* borrowed, no refcount */

		DUK_HOBJECT_INCREF(thr, func);  /* act->func */

#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
		duk__update_func_caller_prop(thr, func);
		act = thr->callstack + thr->callstack_top - 1;
#endif
	}

	/* [... func this arg1 ... argN]  (not tail call)
	 * [this | arg1 ... argN]         (tail call)
	 *
	 * idx_args updated to match
	 */

	/*
	 *  Environment record creation and 'arguments' object creation.
	 *  Named function expression name binding is handled by the
	 *  compiler; the compiled function's parent env will contain
	 *  the (immutable) binding already.
	 *
	 *  Delayed creation (on demand) is handled in duk_js_var.c.
	 */

	DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(func));  /* bound function chain has already been resolved */

	if (!DUK_HOBJECT_HAS_NEWENV(func)) {
		/* use existing env (e.g. for non-strict eval); cannot have
		 * an own 'arguments' object (but can refer to the existing one)
		 */

		duk__handle_oldenv_for_call(thr, func, act);

		DUK_ASSERT(act->lex_env != NULL);
		DUK_ASSERT(act->var_env != NULL);
		goto env_done;
	}

	DUK_ASSERT(DUK_HOBJECT_HAS_NEWENV(func));

	if (!DUK_HOBJECT_HAS_CREATEARGS(func)) {
		/* no need to create environment record now; leave as NULL */
		DUK_ASSERT(act->lex_env == NULL);
		DUK_ASSERT(act->var_env == NULL);
		goto env_done;
	}

	/* third arg: absolute index (to entire valstack) of idx_bottom of new activation */
	env = duk_create_activation_environment_record(thr, func, act->idx_bottom);
	DUK_ASSERT(env != NULL);

	/* [... arg1 ... argN envobj] */

	/* original input stack before nargs/nregs handling must be
	 * intact for 'arguments' object
	 */
	DUK_ASSERT(DUK_HOBJECT_HAS_CREATEARGS(func));
	duk__handle_createargs_for_call(thr, func, env, num_stack_args);

	/* [... arg1 ... argN envobj] */

	act->lex_env = env;
	act->var_env = env;
	DUK_HOBJECT_INCREF(thr, act->lex_env);
	DUK_HOBJECT_INCREF(thr, act->var_env);
	duk_pop(ctx);

 env_done:
	/* [... arg1 ... argN] */

	/*
	 *  Setup value stack: clamp to 'nargs', fill up to 'nregs'
	 */

	duk__adjust_valstack_and_top(thr,
	                             num_stack_args,
	                             idx_args,
	                             nregs,
	                             nargs,
	                             func);

	/*
	 *  Shift to new valstack_bottom.
	 */

	thr->valstack_bottom = thr->valstack_bottom + idx_args;
	/* keep current valstack_top */
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	/*
	 *  Return to bytecode executor, which will resume execution from
	 *  the topmost activation.
	 */

	return 1;
}
