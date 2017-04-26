/*
 *  Call handling.
 *
 *  Main functions are:
 *
 *    - duk_handle_call_unprotected(): unprotected call to Ecmascript or
 *      Duktape/C function
 *    - duk_handle_call_protected(): protected call to Ecmascript or
 *      Duktape/C function
 *    - duk_handle_safe_call(): make a protected C call within current
 *      activation
 *    - duk_handle_ecma_call_setup(): Ecmascript-to-Ecmascript calls
 *      (not always possible), including tail calls and coroutine resume
 *
 *  See 'execution.rst'.
 *
 *  Note: setjmp() and local variables have a nasty interaction,
 *  see execution.rst; non-volatile locals modified after setjmp()
 *  call are not guaranteed to keep their value.
 */

#include "duk_internal.h"

/* XXX: heap->error_not_allowed for success path too? */

/*
 *  Forward declarations.
 */

DUK_LOCAL void duk__handle_call_inner(duk_hthread *thr,
                                      duk_idx_t num_stack_args,
                                      duk_small_uint_t call_flags,
                                      duk_idx_t idx_func);
DUK_LOCAL void duk__handle_call_error(duk_hthread *thr,
                                      duk_size_t entry_valstack_bottom_index,
                                      duk_size_t entry_valstack_end,
                                      duk_size_t entry_catchstack_top,
                                      duk_size_t entry_callstack_top,
                                      duk_int_t entry_call_recursion_depth,
                                      duk_hthread *entry_curr_thread,
                                      duk_uint_fast8_t entry_thread_state,
                                      duk_instr_t **entry_ptr_curr_pc,
                                      duk_idx_t idx_func,
                                      duk_jmpbuf *old_jmpbuf_ptr);
DUK_LOCAL void duk__handle_safe_call_inner(duk_hthread *thr,
                                           duk_safe_call_function func,
                                           void *udata,
                                           duk_idx_t idx_retbase,
                                           duk_idx_t num_stack_rets,
                                           duk_size_t entry_valstack_bottom_index,
                                           duk_size_t entry_callstack_top,
                                           duk_size_t entry_catchstack_top);
DUK_LOCAL void duk__handle_safe_call_error(duk_hthread *thr,
                                           duk_idx_t idx_retbase,
                                           duk_idx_t num_stack_rets,
                                           duk_size_t entry_valstack_bottom_index,
                                           duk_size_t entry_callstack_top,
                                           duk_size_t entry_catchstack_top,
                                           duk_jmpbuf *old_jmpbuf_ptr);
DUK_LOCAL void duk__handle_safe_call_shared(duk_hthread *thr,
                                            duk_idx_t idx_retbase,
                                            duk_idx_t num_stack_rets,
                                            duk_int_t entry_call_recursion_depth,
                                            duk_hthread *entry_curr_thread,
                                            duk_uint_fast8_t entry_thread_state,
                                            duk_instr_t **entry_ptr_curr_pc);

/*
 *  Interrupt counter fixup (for development only).
 */

#if defined(DUK_USE_INTERRUPT_COUNTER) && defined(DUK_USE_DEBUG)
DUK_LOCAL void duk__interrupt_fixup(duk_hthread *thr, duk_hthread *entry_curr_thread) {
	/* Currently the bytecode executor and executor interrupt
	 * instruction counts are off because we don't execute the
	 * interrupt handler when we're about to exit from the initial
	 * user call into Duktape.
	 *
	 * If we were to execute the interrupt handler here, the counts
	 * would match.  You can enable this block manually to check
	 * that this is the case.
	 */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

#if defined(DUK_USE_INTERRUPT_DEBUG_FIXUP)
	if (entry_curr_thread == NULL) {
		thr->interrupt_init = thr->interrupt_init - thr->interrupt_counter;
		thr->heap->inst_count_interrupt += thr->interrupt_init;
		DUK_DD(DUK_DDPRINT("debug test: updated interrupt count on exit to "
		                   "user code, instruction counts: executor=%ld, interrupt=%ld",
		                   (long) thr->heap->inst_count_exec, (long) thr->heap->inst_count_interrupt));
		DUK_ASSERT(thr->heap->inst_count_exec == thr->heap->inst_count_interrupt);
	}
#else
	DUK_UNREF(thr);
	DUK_UNREF(entry_curr_thread);
#endif
}
#endif

/*
 *  Arguments object creation.
 *
 *  Creating arguments objects involves many small details, see E5 Section
 *  10.6 for the specific requirements.  Much of the arguments object exotic
 *  behavior is implemented in duk_hobject_props.c, and is enabled by the
 *  object flag DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS.
 */

DUK_LOCAL void duk__create_arguments_object(duk_hthread *thr,
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
	duk_get_prop_stridx_short(ctx, -1, DUK_STRIDX_INT_FORMALS);
	formals = duk_get_hobject(ctx, -1);
	if (formals) {
		n_formals = (duk_idx_t) duk_get_length(ctx, -1);
	} else {
		/* This shouldn't happen without tampering of internal
		 * properties: if a function accesses 'arguments', _Formals
		 * is kept.  Check for the case anyway in case internal
		 * properties have been modified manually.
		 */
		DUK_D(DUK_DPRINT("_Formals is undefined when creating arguments, use n_formals == 0"));
		n_formals = 0;
	}
	duk_remove_m2(ctx);  /* leave formals on stack for later use */
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

	arg = duk_push_object_helper(ctx,
	                             DUK_HOBJECT_FLAG_EXTENSIBLE |
	                             DUK_HOBJECT_FLAG_FASTREFS |
	                             DUK_HOBJECT_FLAG_ARRAY_PART |
	                             DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ARGUMENTS),
	                             DUK_BIDX_OBJECT_PROTOTYPE);
	DUK_ASSERT(arg != NULL);
	(void) duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_FLAG_FASTREFS |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              -1);  /* no prototype */
	(void) duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_FLAG_FASTREFS |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              -1);  /* no prototype */
	i_arg = duk_get_top(ctx) - 3;
	i_map = i_arg + 1;
	i_mappednames = i_arg + 2;

	/* [ ... formals arguments map mappedNames ] */

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

			duk_dup_top(ctx);  /* [ ... name name ] */

			if (!duk_has_prop(ctx, i_mappednames)) {
				/* steps 11.c.ii.1 - 11.c.ii.4, but our internal book-keeping
				 * differs from the reference model
				 */

				/* [ ... name ] */

				need_map = 1;

				DUK_DDD(DUK_DDDPRINT("set mappednames[%s]=%ld",
				                     (const char *) duk_get_string(ctx, -1),
				                     (long) idx));
				duk_dup_top(ctx);                      /* name */
				(void) duk_push_uint_to_hstring(ctx, (duk_uint_t) idx);  /* index */
				duk_xdef_prop_wec(ctx, i_mappednames);  /* out of spec, must be configurable */

				DUK_DDD(DUK_DDDPRINT("set map[%ld]=%s",
				                     (long) idx,
				                     duk_get_string(ctx, -1)));
				duk_dup_top(ctx);         /* name */
				duk_xdef_prop_index_wec(ctx, i_map, (duk_uarridx_t) idx);  /* out of spec, must be configurable */
			} else {
				/* duk_has_prop() popped the second 'name' */
			}

			/* [ ... name ] */
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
		/* Callee/caller are throwers and are not deletable etc.  They
		 * could be implemented as virtual properties, but currently
		 * there is no support for virtual properties which are accessors
		 * (only plain virtual properties).  This would not be difficult
		 * to change in duk_hobject_props, but we can make the throwers
		 * normal, concrete properties just as easily.
		 *
		 * Note that the specification requires that the *same* thrower
		 * built-in object is used here!  See E5 Section 10.6 main
		 * algoritm, step 14, and Section 13.2.3 which describes the
		 * thrower.  See test case test-arguments-throwers.js.
		 */

		DUK_DDD(DUK_DDDPRINT("strict function, setting caller/callee to throwers"));

		duk_xdef_prop_stridx_thrower(ctx, i_arg, DUK_STRIDX_CALLER);
		duk_xdef_prop_stridx_thrower(ctx, i_arg, DUK_STRIDX_CALLEE);
	} else {
		DUK_DDD(DUK_DDDPRINT("non-strict function, setting callee to actual value"));
		duk_push_hobject(ctx, func);
		duk_xdef_prop_stridx(ctx, i_arg, DUK_STRIDX_CALLEE, DUK_PROPDESC_FLAGS_WC);
	}

	/* set exotic behavior only after we're done */
	if (need_map) {
		/* Exotic behaviors are only enabled for arguments objects
		 * which have a parameter map (see E5 Section 10.6 main
		 * algorithm, step 12).
		 *
		 * In particular, a non-strict arguments object with no
		 * mapped formals does *NOT* get exotic behavior, even
		 * for e.g. "caller" property.  This seems counterintuitive
		 * but seems to be the case.
		 */

		/* cannot be strict (never mapped variables) */
		DUK_ASSERT(!DUK_HOBJECT_HAS_STRICT(func));

		DUK_DDD(DUK_DDDPRINT("enabling exotic behavior for arguments object"));
		DUK_HOBJECT_SET_EXOTIC_ARGUMENTS(arg);
	} else {
		DUK_DDD(DUK_DDDPRINT("not enabling exotic behavior for arguments object"));
	}

	DUK_DDD(DUK_DDDPRINT("final arguments related objects: "
	                     "arguments at index %ld -> %!O "
	                     "map at index %ld -> %!O "
	                     "mappednames at index %ld -> %!O",
	                     (long) i_arg, (duk_heaphdr *) duk_get_hobject(ctx, i_arg),
	                     (long) i_map, (duk_heaphdr *) duk_get_hobject(ctx, i_map),
	                     (long) i_mappednames, (duk_heaphdr *) duk_get_hobject(ctx, i_mappednames)));

	/* [ args(n) [crud] formals arguments map mappednames ] */

	duk_pop_2(ctx);
	duk_remove_m2(ctx);

	/* [ args [crud] arguments ] */
}

/* Helper for creating the arguments object and adding it to the env record
 * on top of the value stack.  This helper has a very strict dependency on
 * the shape of the input stack.
 */
DUK_LOCAL void duk__handle_createargs_for_call(duk_hthread *thr,
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

	/* [ ... arg1 ... argN envobj ] */

	duk__create_arguments_object(thr,
	                             func,
	                             env,
	                             duk_get_top(ctx) - num_stack_args - 1,    /* idx_argbase */
	                             num_stack_args);

	/* [ ... arg1 ... argN envobj argobj ] */

	duk_xdef_prop_stridx_short(ctx,
	                           -2,
	                           DUK_STRIDX_LC_ARGUMENTS,
	                           DUK_HOBJECT_HAS_STRICT(func) ? DUK_PROPDESC_FLAGS_E :   /* strict: non-deletable, non-writable */
	                                                          DUK_PROPDESC_FLAGS_WE);  /* non-strict: non-deletable, writable */
	/* [ ... arg1 ... argN envobj ] */
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

DUK_LOCAL void duk__handle_bound_chain_for_call(duk_hthread *thr,
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
			if (!DUK_HOBJECT_HAS_BOUNDFUNC(func)) {
				/* Normal non-bound function. */
				break;
			}
		} else {
			/* Function.prototype.bind() should never let this happen,
			 * ugly error message is enough.
			 */
			DUK_ERROR_INTERNAL(thr);
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
			/* See: tests/ecmascript/test-spec-bound-constructor.js */
			DUK_DDD(DUK_DDDPRINT("constructor call: don't update this binding"));
		} else {
			duk_get_prop_stridx(ctx, idx_func, DUK_STRIDX_INT_THIS);
			duk_replace(ctx, idx_func + 1);  /* idx_this = idx_func + 1 */
		}

		/* [ ... func this arg1 ... argN ] */

		/* XXX: duk_get_length? */
		duk_get_prop_stridx(ctx, idx_func, DUK_STRIDX_INT_ARGS);  /* -> [ ... func this arg1 ... argN _Args ] */
		duk_get_prop_stridx_short(ctx, -1, DUK_STRIDX_LENGTH);          /* -> [ ... func this arg1 ... argN _Args length ] */
		len = (duk_idx_t) duk_require_int(ctx, -1);
		duk_pop(ctx);

		duk_require_stack(ctx, len);
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

	if (DUK_UNLIKELY(sanity == 0)) {
		DUK_ERROR_RANGE(thr, DUK_STR_BOUND_CHAIN_LIMIT);
	}

	DUK_DDD(DUK_DDDPRINT("final non-bound function is: %!T", duk_get_tval(ctx, idx_func)));

#if defined(DUK_USE_ASSERTIONS)
	tv_func = duk_require_tval(ctx, idx_func);
	DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv_func) || DUK_TVAL_IS_OBJECT(tv_func));
	if (DUK_TVAL_IS_OBJECT(tv_func)) {
		func = DUK_TVAL_GET_OBJECT(tv_func);
		DUK_ASSERT(func != NULL);
		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUNDFUNC(func));
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPFUNC(func) ||
		           DUK_HOBJECT_HAS_NATFUNC(func));
	}
#endif

	/* write back */
	*p_num_stack_args = num_stack_args;
}

/*
 *  Helper for setting up var_env and lex_env of an activation,
 *  assuming it does NOT have the DUK_HOBJECT_FLAG_NEWENV flag.
 */

DUK_LOCAL void duk__handle_oldenv_for_call(duk_hthread *thr,
                                           duk_hobject *func,
                                           duk_activation *act) {
	duk_hcompfunc *f;
	duk_hobject *h_lex;
	duk_hobject *h_var;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(act != NULL);
	DUK_ASSERT(!DUK_HOBJECT_HAS_NEWENV(func));
	DUK_ASSERT(!DUK_HOBJECT_HAS_CREATEARGS(func));
	DUK_ASSERT(DUK_HOBJECT_IS_COMPFUNC(func));
	DUK_UNREF(thr);

	f = (duk_hcompfunc *) func;
	h_lex = DUK_HCOMPFUNC_GET_LEXENV(thr->heap, f);
	h_var = DUK_HCOMPFUNC_GET_VARENV(thr->heap, f);
	DUK_ASSERT(h_lex != NULL);  /* Always true for closures (not for templates) */
	DUK_ASSERT(h_var != NULL);
	act->lex_env = h_lex;
	act->var_env = h_var;
	DUK_HOBJECT_INCREF(thr, h_lex);
	DUK_HOBJECT_INCREF(thr, h_var);
}

/*
 *  Helper for updating callee 'caller' property.
 */

#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
DUK_LOCAL void duk__update_func_caller_prop(duk_hthread *thr, duk_hobject *func) {
	duk_tval *tv_caller;
	duk_hobject *h_tmp;
	duk_activation *act_callee;
	duk_activation *act_caller;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(!DUK_HOBJECT_HAS_BOUNDFUNC(func));  /* bound chain resolved */
	DUK_ASSERT(thr->callstack_top >= 1);

	if (DUK_HOBJECT_HAS_STRICT(func)) {
		/* Strict functions don't get their 'caller' updated. */
		return;
	}

	DUK_ASSERT(thr->callstack_top > 0);
	act_callee = thr->callstack_curr;
	DUK_ASSERT(act_callee != NULL);
	act_caller = (thr->callstack_top >= 2 ? act_callee - 1 : NULL);

	/* XXX: check .caller writability? */

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

DUK_LOCAL void duk__coerce_effective_this_binding(duk_hthread *thr,
                                                  duk_hobject *func,
                                                  duk_idx_t idx_this) {
	duk_context *ctx = (duk_context *) thr;
	duk_tval *tv_this;
	duk_hobject *obj_global;

	if (func == NULL || DUK_HOBJECT_HAS_STRICT(func)) {
		/* Lightfuncs are always considered strict. */
		DUK_DDD(DUK_DDDPRINT("this binding: strict -> use directly"));
		return;
	}

	/* XXX: byte offset */
	tv_this = thr->valstack_bottom + idx_this;
	switch (DUK_TVAL_GET_TAG(tv_this)) {
	case DUK_TAG_OBJECT:
		DUK_DDD(DUK_DDDPRINT("this binding: non-strict, object -> use directly"));
		break;
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
		DUK_DDD(DUK_DDDPRINT("this binding: non-strict, undefined/null -> use global object"));
		obj_global = thr->builtins[DUK_BIDX_GLOBAL];
		/* XXX: avoid this check somehow */
		if (DUK_LIKELY(obj_global != NULL)) {
			DUK_ASSERT(!DUK_TVAL_IS_HEAP_ALLOCATED(tv_this));  /* no need to decref previous value */
			DUK_TVAL_SET_OBJECT(tv_this, obj_global);
			DUK_HOBJECT_INCREF(thr, obj_global);
		} else {
			/* This may only happen if built-ins are being "torn down".
			 * This behavior is out of specification scope.
			 */
			DUK_D(DUK_DPRINT("this binding: wanted to use global object, but it is NULL -> using undefined instead"));
			DUK_ASSERT(!DUK_TVAL_IS_HEAP_ALLOCATED(tv_this));  /* no need to decref previous value */
			DUK_TVAL_SET_UNDEFINED(tv_this);  /* nothing to incref */
		}
		break;
	default:
		/* Plain buffers and lightfuncs are object coerced.  Lightfuncs
		 * very rarely come here however, because the call target would
		 * need to be a strict non-lightfunc (lightfuncs are considered
		 * strict) with an explicit lightfunc 'this' binding.
		 */
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv_this));
		DUK_DDD(DUK_DDDPRINT("this binding: non-strict, not object/undefined/null -> use ToObject(value)"));
		duk_to_object(ctx, idx_this);  /* may have side effects */
		break;
	}
}

/*
 *  Shared helper for non-bound func lookup.
 *
 *  Returns duk_hobject * to the final non-bound function (NULL for lightfunc).
 */

DUK_LOCAL duk_hobject *duk__nonbound_func_lookup(duk_context *ctx,
                                                 duk_idx_t idx_func,
                                                 duk_idx_t *out_num_stack_args,
                                                 duk_tval **out_tv_func,
                                                 duk_small_uint_t call_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_func;
	duk_hobject *func;

	for (;;) {
		/* Use loop to minimize code size of relookup after bound function case */
		tv_func = DUK_GET_TVAL_POSIDX(ctx, idx_func);
		DUK_ASSERT(tv_func != NULL);

		if (DUK_TVAL_IS_OBJECT(tv_func)) {
			func = DUK_TVAL_GET_OBJECT(tv_func);
			if (!DUK_HOBJECT_IS_CALLABLE(func)) {
				goto not_callable_error;
			}
			if (DUK_HOBJECT_HAS_BOUNDFUNC(func)) {
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
	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUNDFUNC(func));
	DUK_ASSERT(func == NULL || (DUK_HOBJECT_IS_COMPFUNC(func) ||
	                            DUK_HOBJECT_IS_NATFUNC(func)));

	*out_tv_func = tv_func;
	return func;

 not_callable_error:
	DUK_ASSERT(tv_func != NULL);
#if defined(DUK_USE_PARANOID_ERRORS)
	DUK_ERROR_TYPE(thr, DUK_STR_NOT_CALLABLE);
#else
	DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "%s not callable", duk_push_string_tval_readable(ctx, tv_func));
#endif
	DUK_UNREACHABLE();
	return NULL;  /* never executed */
}

/*
 *  Value stack resize and stack top adjustment helper.
 *
 *  XXX: This should all be merged to duk_valstack_resize_raw().
 */

DUK_LOCAL void duk__adjust_valstack_and_top(duk_hthread *thr,
                                            duk_idx_t num_stack_args,
                                            duk_idx_t idx_args,
                                            duk_idx_t nregs,
                                            duk_idx_t nargs,
                                            duk_hobject *func) {
	duk_context *ctx = (duk_context *) thr;
	duk_size_t vs_min_size;
	duk_bool_t adjusted_top = 0;

	vs_min_size = (thr->valstack_bottom - thr->valstack) +  /* bottom of current func */
	              idx_args;                                 /* bottom of new func */

	if (nregs >= 0) {
		DUK_ASSERT(nargs >= 0);
		DUK_ASSERT(nregs >= nargs);
		vs_min_size += nregs;
	} else {
		/* 'func' wants stack "as is" */
		vs_min_size += num_stack_args;  /* num entries of new func at entry */
	}
	if (func == NULL || DUK_HOBJECT_IS_NATFUNC(func)) {
		vs_min_size += DUK_VALSTACK_API_ENTRY_MINIMUM;  /* Duktape/C API guaranteed entries (on top of args) */
	}
	vs_min_size += DUK_VALSTACK_INTERNAL_EXTRA;             /* + spare */

	/* XXX: We can't resize the value stack to a size smaller than the
	 * current top, so the order of the resize and adjusting the stack
	 * top depends on the current vs. final size of the value stack.
	 * The operations could be combined to avoid this, but the proper
	 * fix is to only grow the value stack on a function call, and only
	 * shrink it (without throwing if the shrink fails) on function
	 * return.
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

	/* Ensure space for final configuration (idx_retbase + num_stack_rets)
	 * and intermediate configurations.
	 */
	duk_require_stack_top(ctx,
	                      (idx_rcbase > idx_retbase ? idx_rcbase : idx_retbase) +
	                      num_stack_rets);

	/* Chop extra retvals away / extend with undefined. */
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
 *  Misc shared helpers.
 */

/* Get valstack index for the func argument or throw if insane stack. */
DUK_LOCAL duk_idx_t duk__get_idx_func(duk_hthread *thr, duk_idx_t num_stack_args) {
	duk_size_t off_stack_top;
	duk_size_t off_stack_args;
	duk_size_t off_stack_all;
	duk_idx_t idx_func;         /* valstack index of 'func' and retval (relative to entry valstack_bottom) */

	/* Argument validation and func/args offset. */
	off_stack_top = (duk_size_t) ((duk_uint8_t *) thr->valstack_top - (duk_uint8_t *) thr->valstack_bottom);
	off_stack_args = (duk_size_t) ((duk_size_t) num_stack_args * sizeof(duk_tval));
	off_stack_all = off_stack_args + 2 * sizeof(duk_tval);
	if (DUK_UNLIKELY(off_stack_all > off_stack_top)) {
		/* Since stack indices are not reliable, we can't do anything useful
		 * here.  Invoke the existing setjmp catcher, or if it doesn't exist,
		 * call the fatal error handler.
		 */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
		return 0;
	}
	idx_func = (duk_idx_t) ((off_stack_top - off_stack_all) / sizeof(duk_tval));
	return idx_func;
}

/*
 *  duk_handle_call_protected() and duk_handle_call_unprotected():
 *  call into a Duktape/C or an Ecmascript function from any state.
 *
 *  Input stack (thr):
 *
 *    [ func this arg1 ... argN ]
 *
 *  Output stack (thr):
 *
 *    [ retval ]         (DUK_EXEC_SUCCESS)
 *    [ errobj ]         (DUK_EXEC_ERROR (normal error), protected call)
 *
 *  Even when executing a protected call an error may be thrown in rare cases
 *  such as an insane num_stack_args argument.  If there is no catchpoint for
 *  such errors, the fatal error handler is called.
 *
 *  The error handling path should be error free, even for out-of-memory
 *  errors, to ensure safe sandboxing.  (As of Duktape 1.4.0 this is not
 *  yet the case, see XXX notes below.)
 */

DUK_INTERNAL duk_int_t duk_handle_call_protected(duk_hthread *thr,
                                                 duk_idx_t num_stack_args,
                                                 duk_small_uint_t call_flags) {
	duk_context *ctx;
	duk_size_t entry_valstack_bottom_index;
	duk_size_t entry_valstack_end;
	duk_size_t entry_callstack_top;
	duk_size_t entry_catchstack_top;
	duk_int_t entry_call_recursion_depth;
	duk_hthread *entry_curr_thread;
	duk_uint_fast8_t entry_thread_state;
	duk_instr_t **entry_ptr_curr_pc;
	duk_jmpbuf *old_jmpbuf_ptr = NULL;
	duk_jmpbuf our_jmpbuf;
	duk_idx_t idx_func;  /* valstack index of 'func' and retval (relative to entry valstack_bottom) */

	/* XXX: Multiple tv_func lookups are now avoided by making a local
	 * copy of tv_func.  Another approach would be to compute an offset
	 * for tv_func from valstack bottom and recomputing the tv_func
	 * pointer quickly as valstack + offset instead of calling duk_get_tval().
	 */

	ctx = (duk_context *) thr;
	DUK_UNREF(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(num_stack_args >= 0);
	/* XXX: currently NULL allocations are not supported; remove if later allowed */
	DUK_ASSERT(thr->valstack != NULL);
	DUK_ASSERT(thr->callstack != NULL);
	DUK_ASSERT(thr->catchstack != NULL);

	/* Argument validation and func/args offset. */
	idx_func = duk__get_idx_func(thr, num_stack_args);

	/* Preliminaries, required by setjmp() handler.  Must be careful not
	 * to throw an unintended error here.
	 */

	entry_valstack_bottom_index = (duk_size_t) (thr->valstack_bottom - thr->valstack);
#if defined(DUK_USE_PREFER_SIZE)
	entry_valstack_end = (duk_size_t) (thr->valstack_end - thr->valstack);
#else
	DUK_ASSERT((duk_size_t) (thr->valstack_end - thr->valstack) == thr->valstack_size);
	entry_valstack_end = thr->valstack_size;
#endif
	entry_callstack_top = thr->callstack_top;
	entry_catchstack_top = thr->catchstack_top;
	entry_call_recursion_depth = thr->heap->call_recursion_depth;
	entry_curr_thread = thr->heap->curr_thread;  /* Note: may be NULL if first call */
	entry_thread_state = thr->state;
	entry_ptr_curr_pc = thr->ptr_curr_pc;  /* may be NULL */

	DUK_DD(DUK_DDPRINT("duk_handle_call_protected: thr=%p, num_stack_args=%ld, "
	                   "call_flags=0x%08lx (ignorerec=%ld, constructor=%ld), "
	                   "valstack_top=%ld, idx_func=%ld, idx_args=%ld, rec_depth=%ld/%ld, "
	                   "entry_valstack_bottom_index=%ld, entry_callstack_top=%ld, entry_catchstack_top=%ld, "
	                   "entry_call_recursion_depth=%ld, entry_curr_thread=%p, entry_thread_state=%ld",
	                   (void *) thr,
	                   (long) num_stack_args,
	                   (unsigned long) call_flags,
	                   (long) ((call_flags & DUK_CALL_FLAG_IGNORE_RECLIMIT) != 0 ? 1 : 0),
	                   (long) ((call_flags & DUK_CALL_FLAG_CONSTRUCTOR_CALL) != 0 ? 1 : 0),
	                   (long) duk_get_top(ctx),
	                   (long) idx_func,
	                   (long) (idx_func + 2),
	                   (long) thr->heap->call_recursion_depth,
	                   (long) thr->heap->call_recursion_limit,
	                   (long) entry_valstack_bottom_index,
	                   (long) entry_callstack_top,
	                   (long) entry_catchstack_top,
	                   (long) entry_call_recursion_depth,
	                   (void *) entry_curr_thread,
	                   (long) entry_thread_state));

	old_jmpbuf_ptr = thr->heap->lj.jmpbuf_ptr;
	thr->heap->lj.jmpbuf_ptr = &our_jmpbuf;

#if defined(DUK_USE_CPP_EXCEPTIONS)
	try {
#else
	DUK_ASSERT(thr->heap->lj.jmpbuf_ptr == &our_jmpbuf);
	if (DUK_SETJMP(our_jmpbuf.jb) == 0) {
#endif
		/* Call handling and success path.  Success path exit cleans
		 * up almost all state.
		 */
		duk__handle_call_inner(thr, num_stack_args, call_flags, idx_func);

		thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

		return DUK_EXEC_SUCCESS;
#if defined(DUK_USE_CPP_EXCEPTIONS)
	} catch (duk_internal_exception &exc) {
#else
	} else {
#endif
		/* Error; error value is in heap->lj.value1. */

#if defined(DUK_USE_CPP_EXCEPTIONS)
		DUK_UNREF(exc);
#endif

		duk__handle_call_error(thr,
		                       entry_valstack_bottom_index,
		                       entry_valstack_end,
		                       entry_catchstack_top,
		                       entry_callstack_top,
		                       entry_call_recursion_depth,
		                       entry_curr_thread,
		                       entry_thread_state,
		                       entry_ptr_curr_pc,
		                       idx_func,
		                       old_jmpbuf_ptr);

		return DUK_EXEC_ERROR;
	}
#if defined(DUK_USE_CPP_EXCEPTIONS)
	catch (std::exception &exc) {
		const char *what = exc.what();
		if (!what) {
			what = "unknown";
		}
		DUK_D(DUK_DPRINT("unexpected c++ std::exception (perhaps thrown by user code)"));
		try {
			DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "caught invalid c++ std::exception '%s' (perhaps thrown by user code)", what);
		} catch (duk_internal_exception exc) {
			DUK_D(DUK_DPRINT("caught api error thrown from unexpected c++ std::exception"));
			DUK_UNREF(exc);
			duk__handle_call_error(thr,
			                       entry_valstack_bottom_index,
			                       entry_valstack_end,
			                       entry_catchstack_top,
			                       entry_callstack_top,
			                       entry_call_recursion_depth,
			                       entry_curr_thread,
			                       entry_thread_state,
			                       entry_ptr_curr_pc,
			                       idx_func,
			                       old_jmpbuf_ptr);

			return DUK_EXEC_ERROR;
		}
	} catch (...) {
		DUK_D(DUK_DPRINT("unexpected c++ exception (perhaps thrown by user code)"));
		try {
			DUK_ERROR_TYPE(thr, "caught invalid c++ exception (perhaps thrown by user code)");
		} catch (duk_internal_exception exc) {
			DUK_D(DUK_DPRINT("caught api error thrown from unexpected c++ exception"));
			DUK_UNREF(exc);
			duk__handle_call_error(thr,
			                       entry_valstack_bottom_index,
			                       entry_valstack_end,
			                       entry_catchstack_top,
			                       entry_callstack_top,
			                       entry_call_recursion_depth,
			                       entry_curr_thread,
			                       entry_thread_state,
			                       entry_ptr_curr_pc,
			                       idx_func,
			                       old_jmpbuf_ptr);

			return DUK_EXEC_ERROR;
		}
	}
#endif
}

DUK_INTERNAL void duk_handle_call_unprotected(duk_hthread *thr,
                                              duk_idx_t num_stack_args,
                                              duk_small_uint_t call_flags) {
	duk_idx_t idx_func;         /* valstack index of 'func' and retval (relative to entry valstack_bottom) */

	/* Argument validation and func/args offset. */
	idx_func = duk__get_idx_func(thr, num_stack_args);

	duk__handle_call_inner(thr, num_stack_args, call_flags, idx_func);
}

DUK_LOCAL void duk__handle_call_inner(duk_hthread *thr,
                                      duk_idx_t num_stack_args,
                                      duk_small_uint_t call_flags,
                                      duk_idx_t idx_func) {
	duk_context *ctx;
	duk_size_t entry_valstack_bottom_index;
	duk_size_t entry_valstack_end;
	duk_size_t entry_callstack_top;
	duk_size_t entry_catchstack_top;
	duk_int_t entry_call_recursion_depth;
	duk_hthread *entry_curr_thread;
	duk_uint_fast8_t entry_thread_state;
	duk_instr_t **entry_ptr_curr_pc;
	duk_idx_t nargs;            /* # argument registers target function wants (< 0 => "as is") */
	duk_idx_t nregs;            /* # total registers target function wants on entry (< 0 => "as is") */
	duk_hobject *func;          /* 'func' on stack (borrowed reference) */
	duk_tval *tv_func;          /* duk_tval ptr for 'func' on stack (borrowed reference) or tv_func_copy */
	duk_tval tv_func_copy;      /* to avoid relookups */
	duk_activation *act;
	duk_hobject *env;
	duk_ret_t rc;

	ctx = (duk_context *) thr;
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(num_stack_args >= 0);
	/* XXX: currently NULL allocations are not supported; remove if later allowed */
	DUK_ASSERT(thr->valstack != NULL);
	DUK_ASSERT(thr->callstack != NULL);
	DUK_ASSERT(thr->catchstack != NULL);

	DUK_DD(DUK_DDPRINT("duk__handle_call_inner: num_stack_args=%ld, call_flags=0x%08lx, top=%ld",
	                   (long) num_stack_args, (long) call_flags, (long) duk_get_top(ctx)));

	/*
	 *  Store entry state.
	 */

	entry_valstack_bottom_index = (duk_size_t) (thr->valstack_bottom - thr->valstack);
#if defined(DUK_USE_PREFER_SIZE)
	entry_valstack_end = (duk_size_t) (thr->valstack_end - thr->valstack);
#else
	DUK_ASSERT((duk_size_t) (thr->valstack_end - thr->valstack) == thr->valstack_size);
	entry_valstack_end = thr->valstack_size;
#endif
	entry_callstack_top = thr->callstack_top;
	entry_catchstack_top = thr->catchstack_top;
	entry_call_recursion_depth = thr->heap->call_recursion_depth;
	entry_curr_thread = thr->heap->curr_thread;  /* Note: may be NULL if first call */
	entry_thread_state = thr->state;
	entry_ptr_curr_pc = thr->ptr_curr_pc;  /* may be NULL */

	/* If thr->ptr_curr_pc is set, sync curr_pc to act->pc.  Then NULL
	 * thr->ptr_curr_pc so that it's not accidentally used with an incorrect
	 * activation when side effects occur.
	 */
	duk_hthread_sync_and_null_currpc(thr);

	DUK_DD(DUK_DDPRINT("duk__handle_call_inner: thr=%p, num_stack_args=%ld, "
	                   "call_flags=0x%08lx (ignorerec=%ld, constructor=%ld), "
	                   "valstack_top=%ld, idx_func=%ld, idx_args=%ld, rec_depth=%ld/%ld, "
	                   "entry_valstack_bottom_index=%ld, entry_callstack_top=%ld, entry_catchstack_top=%ld, "
	                   "entry_call_recursion_depth=%ld, entry_curr_thread=%p, entry_thread_state=%ld",
	                   (void *) thr,
	                   (long) num_stack_args,
	                   (unsigned long) call_flags,
	                   (long) ((call_flags & DUK_CALL_FLAG_IGNORE_RECLIMIT) != 0 ? 1 : 0),
	                   (long) ((call_flags & DUK_CALL_FLAG_CONSTRUCTOR_CALL) != 0 ? 1 : 0),
	                   (long) duk_get_top(ctx),
	                   (long) idx_func,
	                   (long) (idx_func + 2),
	                   (long) thr->heap->call_recursion_depth,
	                   (long) thr->heap->call_recursion_limit,
	                   (long) entry_valstack_bottom_index,
	                   (long) entry_callstack_top,
	                   (long) entry_catchstack_top,
	                   (long) entry_call_recursion_depth,
	                   (void *) entry_curr_thread,
	                   (long) entry_thread_state));


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

	/* XXX: remove DUK_CALL_FLAG_IGNORE_RECLIMIT flag: there's now the
	 * reclimit bump?
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
			DUK_ERROR_RANGE(thr, DUK_STR_C_CALLSTACK_LIMIT);
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

	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUNDFUNC(func));
	DUK_ASSERT(func == NULL || (DUK_HOBJECT_IS_COMPFUNC(func) ||
	                            DUK_HOBJECT_IS_NATFUNC(func)));

	duk__coerce_effective_this_binding(thr, func, idx_func + 1);
	DUK_DDD(DUK_DDDPRINT("effective 'this' binding is: %!T",
	                     (duk_tval *) duk_get_tval(ctx, idx_func + 1)));

	/* [ ... func this arg1 ... argN ] */

	/*
	 *  Setup a preliminary activation and figure out nargs/nregs.
	 *
	 *  Don't touch valstack_bottom or valstack_top yet so that Duktape API
	 *  calls work normally.
	 */

	duk_hthread_callstack_grow(thr);

	act = thr->callstack_curr;
	if (act != NULL) {
		/*
		 *  Update idx_retval of current activation.
		 *
		 *  Although it might seem this is not necessary (bytecode executor
		 *  does this for Ecmascript-to-Ecmascript calls; other calls are
		 *  handled here), this turns out to be necessary for handling yield
		 *  and resume.  For them, an Ecmascript-to-native call happens, and
		 *  the Ecmascript call's idx_retval must be set for things to work.
		 */

		act->idx_retval = entry_valstack_bottom_index + idx_func;
	}

	DUK_ASSERT(thr->callstack_top < thr->callstack_size);
	act = thr->callstack + thr->callstack_top;
	thr->callstack_top++;
	thr->callstack_curr = act;
	DUK_ASSERT(thr->callstack_top <= thr->callstack_size);
	DUK_ASSERT(thr->valstack_top > thr->valstack_bottom);  /* at least effective 'this' */
	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUNDFUNC(func));

	act->flags = 0;

	/* For now all calls except Ecma-to-Ecma calls prevent a yield. */
	act->flags |= DUK_ACT_FLAG_PREVENT_YIELD;
	if (call_flags & DUK_CALL_FLAG_CONSTRUCTOR_CALL) {
		act->flags |= DUK_ACT_FLAG_CONSTRUCT;
	}
	if (call_flags & DUK_CALL_FLAG_DIRECT_EVAL) {
		act->flags |= DUK_ACT_FLAG_DIRECT_EVAL;
	}

	/* These base values are never used, but if the compiler doesn't know
	 * that DUK_ERROR() won't return, these are needed to silence warnings.
	 * On the other hand, scan-build will warn about the values not being
	 * used, so add a DUK_UNREF.
	 */
	nargs = 0; DUK_UNREF(nargs);
	nregs = 0; DUK_UNREF(nregs);

	if (DUK_LIKELY(func != NULL)) {
		if (DUK_HOBJECT_HAS_STRICT(func)) {
			act->flags |= DUK_ACT_FLAG_STRICT;
		}
		if (DUK_HOBJECT_IS_COMPFUNC(func)) {
			nargs = ((duk_hcompfunc *) func)->nargs;
			nregs = ((duk_hcompfunc *) func)->nregs;
			DUK_ASSERT(nregs >= nargs);
		} else {
			/* True because of call target lookup checks. */
			DUK_ASSERT(DUK_HOBJECT_IS_NATFUNC(func));

			/* Note: nargs (and nregs) may be negative for a native,
			 * function, which indicates that the function wants the
			 * input stack "as is" (i.e. handles "vararg" arguments).
			 */
			nargs = ((duk_hnatfunc *) func)->nargs;
			nregs = nargs;
		}
	} else {
		duk_small_uint_t lf_flags;

		DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv_func));
		lf_flags = DUK_TVAL_GET_LIGHTFUNC_FLAGS(tv_func);
		nargs = DUK_LFUNC_FLAGS_GET_NARGS(lf_flags);
		if (nargs == DUK_LFUNC_NARGS_VARARGS) {
			nargs = -1;  /* vararg */
		}
		nregs = nargs;

		act->flags |= DUK_ACT_FLAG_STRICT;
	}

	act->func = func;  /* NULL for lightfunc */
	act->var_env = NULL;
	act->lex_env = NULL;
#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
	act->prev_caller = NULL;
#endif
	act->curr_pc = NULL;
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	act->prev_line = 0;
#endif
	act->idx_bottom = entry_valstack_bottom_index + idx_func + 2;
#if 0  /* topmost activation idx_retval is considered garbage, no need to init */
	act->idx_retval = 0;
#endif
	DUK_TVAL_SET_TVAL(&act->tv_func, tv_func);  /* borrowed, no refcount */

	/* XXX: remove the preventcount and make yield walk the callstack?
	 * Or perhaps just use a single flag, not a counter, faster to just
	 * set and restore?
	 */
	if (act->flags & DUK_ACT_FLAG_PREVENT_YIELD) {
		/* duk_hthread_callstack_unwind() will decrease this on unwind */
		thr->callstack_preventcount++;
	}

	/* XXX: Is this INCREF necessary? 'func' is always a borrowed
	 * reference reachable through the value stack?  If changed, stack
	 * unwind code also needs to be fixed to match.
	 */
	DUK_HOBJECT_INCREF_ALLOWNULL(thr, func);  /* act->func */

#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
	if (func) {
		duk__update_func_caller_prop(thr, func);
		act = thr->callstack_curr;
	}
#endif

	/* [ ... func this arg1 ... argN ] */

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

	DUK_ASSERT(func == NULL || !DUK_HOBJECT_HAS_BOUNDFUNC(func));  /* bound function chain has already been resolved */

	if (DUK_LIKELY(func != NULL)) {
		if (DUK_LIKELY(DUK_HOBJECT_HAS_NEWENV(func))) {
			if (DUK_LIKELY(!DUK_HOBJECT_HAS_CREATEARGS(func))) {
				/* Use a new environment but there's no 'arguments' object;
				 * delayed environment initialization.  This is the most
				 * common case.
				 */
				DUK_ASSERT(act->lex_env == NULL);
				DUK_ASSERT(act->var_env == NULL);
			} else {
				/* Use a new environment and there's an 'arguments' object.
				 * We need to initialize it right now.
				 */

				/* third arg: absolute index (to entire valstack) of idx_bottom of new activation */
				env = duk_create_activation_environment_record(thr, func, act->idx_bottom);
				DUK_ASSERT(env != NULL);

				/* [ ... func this arg1 ... argN envobj ] */

				DUK_ASSERT(DUK_HOBJECT_HAS_CREATEARGS(func));
				duk__handle_createargs_for_call(thr, func, env, num_stack_args);

				/* [ ... func this arg1 ... argN envobj ] */

				act = thr->callstack_curr;
				act->lex_env = env;
				act->var_env = env;
				DUK_HOBJECT_INCREF(thr, env);
				DUK_HOBJECT_INCREF(thr, env);  /* XXX: incref by count (2) directly */
				duk_pop(ctx);
			}
		} else {
			/* Use existing env (e.g. for non-strict eval); cannot have
			 * an own 'arguments' object (but can refer to an existing one).
			 */

			DUK_ASSERT(!DUK_HOBJECT_HAS_CREATEARGS(func));

			duk__handle_oldenv_for_call(thr, func, act);
			/* No need to re-lookup 'act' at present: no side effects. */

			DUK_ASSERT(act->lex_env != NULL);
			DUK_ASSERT(act->var_env != NULL);
		}
	} else {
		/* Lightfuncs are always native functions and have "newenv". */
		DUK_ASSERT(act->lex_env == NULL);
		DUK_ASSERT(act->var_env == NULL);
	}

	/* [ ... func this arg1 ... argN ] */

	/*
	 *  Setup value stack: clamp to 'nargs', fill up to 'nregs'
	 *
	 *  Value stack may either grow or shrink, depending on the
	 *  number of func registers and the number of actual arguments.
	 *  If nregs >= 0, func wants args clamped to 'nargs'; else it
	 *  wants all args (= 'num_stack_args').
	 */

	/* XXX: optimize value stack operation */
	/* XXX: don't want to shrink allocation here */

	duk__adjust_valstack_and_top(thr,
	                             num_stack_args,
	                             idx_func + 2,
	                             nregs,
	                             nargs,
	                             func);

	/*
	 *  Determine call type, then finalize activation, shift to
	 *  new value stack bottom, and call the target.
	 */

	act = thr->callstack_curr;
	if (func != NULL && DUK_HOBJECT_IS_COMPFUNC(func)) {
		/*
		 *  Ecmascript call
		 */

		duk_tval *tv_ret;
		duk_tval *tv_funret;

		DUK_ASSERT(func != NULL);
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPFUNC(func));
		act->curr_pc = DUK_HCOMPFUNC_GET_CODE_BASE(thr->heap, (duk_hcompfunc *) func);

		thr->valstack_bottom = thr->valstack_bottom + idx_func + 2;
		/* keep current valstack_top */
		DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
		DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
		DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

		/* [ ... func this | arg1 ... argN ] ('this' must precede new bottom) */

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
		 */

		/* thr->ptr_curr_pc is set by bytecode executor early on entry */
		DUK_ASSERT(thr->ptr_curr_pc == NULL);
		DUK_DDD(DUK_DDDPRINT("entering bytecode execution"));
		duk_js_execute_bytecode(thr);
		DUK_DDD(DUK_DDDPRINT("returned from bytecode execution"));

		/* Unwind. */

		DUK_ASSERT(thr->catchstack_top >= entry_catchstack_top);  /* may need unwind */
		DUK_ASSERT(thr->callstack_top == entry_callstack_top + 1);
		duk_hthread_catchstack_unwind_norz(thr, entry_catchstack_top);
		duk_hthread_catchstack_shrink_check(thr);
		duk_hthread_callstack_unwind_norz(thr, entry_callstack_top);  /* XXX: may now fail */
		duk_hthread_callstack_shrink_check(thr);

		thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;
		/* keep current valstack_top */
		DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
		DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
		DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
		DUK_ASSERT(thr->valstack_top - thr->valstack_bottom >= idx_func + 1);

		/* Return value handling. */

		/* [ ... func this (crud) retval ] */

		tv_ret = thr->valstack_bottom + idx_func;
		tv_funret = thr->valstack_top - 1;
#if defined(DUK_USE_FASTINT)
		/* Explicit check for fastint downgrade. */
		DUK_TVAL_CHKFAST_INPLACE_FAST(tv_funret);
#endif
		DUK_TVAL_SET_TVAL_UPDREF(thr, tv_ret, tv_funret);  /* side effects */
	} else {
		/*
		 *  Native call.
		 */

		duk_tval *tv_ret;
		duk_tval *tv_funret;

		thr->valstack_bottom = thr->valstack_bottom + idx_func + 2;
		/* keep current valstack_top */
		DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
		DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
		DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
		DUK_ASSERT(func == NULL || ((duk_hnatfunc *) func)->func != NULL);

		/* [ ... func this | arg1 ... argN ] ('this' must precede new bottom) */

		/* For native calls must be NULL so we don't sync back */
		DUK_ASSERT(thr->ptr_curr_pc == NULL);

		if (func) {
			rc = ((duk_hnatfunc *) func)->func((duk_context *) thr);
		} else {
			duk_c_function funcptr = DUK_TVAL_GET_LIGHTFUNC_FUNCPTR(tv_func);
			rc = funcptr((duk_context *) thr);
		}

		/* Automatic error throwing, retval check. */

		if (rc < 0) {
			duk_error_throw_from_negative_rc(thr, rc);
			DUK_UNREACHABLE();
		} else if (rc > 1) {
			DUK_ERROR_TYPE(thr, "c function returned invalid rc");
		}
		DUK_ASSERT(rc == 0 || rc == 1);

		/* Unwind. */

		DUK_ASSERT(thr->catchstack_top == entry_catchstack_top);  /* no need to unwind */
		DUK_ASSERT(thr->callstack_top == entry_callstack_top + 1);
		duk_hthread_callstack_unwind_norz(thr, entry_callstack_top);
		duk_hthread_callstack_shrink_check(thr);

		thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;
		/* keep current valstack_top */
		DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
		DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
		DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
		DUK_ASSERT(thr->valstack_top - thr->valstack_bottom >= idx_func + 1);

		/* Return value handling. */

		/* XXX: should this happen in the callee's activation or after unwinding? */
		tv_ret = thr->valstack_bottom + idx_func;
		if (rc == 0) {
			DUK_TVAL_SET_UNDEFINED_UPDREF(thr, tv_ret);  /* side effects */
		} else {
			/* [ ... func this (crud) retval ] */
			tv_funret = thr->valstack_top - 1;
#if defined(DUK_USE_FASTINT)
			/* Explicit check for fastint downgrade. */
			DUK_TVAL_CHKFAST_INPLACE_FAST(tv_funret);
#endif
			DUK_TVAL_SET_TVAL_UPDREF(thr, tv_ret, tv_funret);  /* side effects */
		}
	}

	duk_set_top(ctx, idx_func + 1);  /* XXX: unnecessary, handle in adjust */

	/* [ ... retval ] */

	/* Ensure there is internal valstack spare before we exit; this may
	 * throw an alloc error.  The same guaranteed size must be available
	 * as before the call.  This is not optimal now: we store the valstack
	 * allocated size during entry; this value may be higher than the
	 * minimal guarantee for an application.
	 */

	/* XXX: we should never shrink here; when we error out later, we'd
	 * need to potentially grow the value stack in error unwind which could
	 * cause another error.
	 */

	(void) duk_valstack_resize_raw((duk_context *) thr,
	                               entry_valstack_end,                    /* same as during entry */
	                               DUK_VSRESIZE_FLAG_SHRINK |             /* flags */
	                               DUK_VSRESIZE_FLAG_COMPACT |
	                               DUK_VSRESIZE_FLAG_THROW);

	/* Restore entry thread executor curr_pc stack frame pointer. */
	thr->ptr_curr_pc = entry_ptr_curr_pc;

	DUK_HEAP_SWITCH_THREAD(thr->heap, entry_curr_thread);  /* may be NULL */
	thr->state = (duk_uint8_t) entry_thread_state;

	/* Disabled assert: triggered with some torture tests. */
#if 0
	DUK_ASSERT((thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread == NULL) ||  /* first call */
	           (thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread != NULL) ||  /* other call */
	           (thr->state == DUK_HTHREAD_STATE_RUNNING && thr->heap->curr_thread == thr));     /* current thread */
#endif

	thr->heap->call_recursion_depth = entry_call_recursion_depth;

	/* If the debugger is active we need to force an interrupt so that
	 * debugger breakpoints are rechecked.  This is important for function
	 * calls caused by side effects (e.g. when doing a DUK_OP_GETPROP), see
	 * GH-303.  Only needed for success path, error path always causes a
	 * breakpoint recheck in the executor.  It would be enough to set this
	 * only when returning to an Ecmascript activation, but setting the flag
	 * on every return should have no ill effect.
	 */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	if (duk_debug_is_attached(thr->heap)) {
		DUK_DD(DUK_DDPRINT("returning with debugger enabled, force interrupt"));
		DUK_ASSERT(thr->interrupt_counter <= thr->interrupt_init);
		thr->interrupt_init -= thr->interrupt_counter;
		thr->interrupt_counter = 0;
		thr->heap->dbg_force_restart = 1;
	}
#endif

#if defined(DUK_USE_INTERRUPT_COUNTER) && defined(DUK_USE_DEBUG)
	duk__interrupt_fixup(thr, entry_curr_thread);
#endif

	/* Restored by success path. */
	DUK_ASSERT(thr->heap->call_recursion_depth == entry_call_recursion_depth);
	DUK_ASSERT(thr->ptr_curr_pc == entry_ptr_curr_pc);

	DUK_ASSERT_LJSTATE_UNSET(thr->heap);

	DUK_REFZERO_CHECK_FAST(thr);

	return;

 thread_state_error:
	DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "invalid thread state for call (%ld)", (long) thr->state);
	DUK_UNREACHABLE();
	return;  /* never executed */
}

DUK_LOCAL void duk__handle_call_error(duk_hthread *thr,
                                      duk_size_t entry_valstack_bottom_index,
                                      duk_size_t entry_valstack_end,
                                      duk_size_t entry_catchstack_top,
                                      duk_size_t entry_callstack_top,
                                      duk_int_t entry_call_recursion_depth,
                                      duk_hthread *entry_curr_thread,
                                      duk_uint_fast8_t entry_thread_state,
                                      duk_instr_t **entry_ptr_curr_pc,
                                      duk_idx_t idx_func,
                                      duk_jmpbuf *old_jmpbuf_ptr) {
	duk_context *ctx;
	duk_tval *tv_ret;

	ctx = (duk_context *) thr;

	DUK_DDD(DUK_DDDPRINT("error caught during duk__handle_call_inner(): %!T",
	                     (duk_tval *) &thr->heap->lj.value1));

	/* Other longjmp types are handled by executor before propagating
	 * the error here.
	 */
	DUK_ASSERT(thr->heap->lj.type == DUK_LJ_TYPE_THROW);
	DUK_ASSERT_LJSTATE_SET(thr->heap);
	DUK_ASSERT(thr->callstack_top >= entry_callstack_top);
	DUK_ASSERT(thr->catchstack_top >= entry_catchstack_top);

	/* We don't need to sync back thr->ptr_curr_pc here because
	 * the bytecode executor always has a setjmp catchpoint which
	 * does that before errors propagate to here.
	 */
	DUK_ASSERT(thr->ptr_curr_pc == NULL);

	/* Restore the previous setjmp catcher so that any error in
	 * error handling will propagate outwards rather than re-enter
	 * the same handler.  However, the error handling path must be
	 * designed to be error free so that sandboxing guarantees are
	 * reliable, see e.g. https://github.com/svaarala/duktape/issues/476.
	 */
	thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

	/* XXX: callstack unwind may now throw an error when closing
	 * scopes; this is a sandboxing issue, described in:
	 * https://github.com/svaarala/duktape/issues/476
	 */
	duk_hthread_catchstack_unwind_norz(thr, entry_catchstack_top);
	duk_hthread_catchstack_shrink_check(thr);
	duk_hthread_callstack_unwind_norz(thr, entry_callstack_top);
	duk_hthread_callstack_shrink_check(thr);

	thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;
	tv_ret = thr->valstack_bottom + idx_func;  /* XXX: byte offset? */
	DUK_TVAL_SET_TVAL_UPDREF(thr, tv_ret, &thr->heap->lj.value1);  /* side effects */
#if defined(DUK_USE_FASTINT)
	/* Explicit check for fastint downgrade. */
	DUK_TVAL_CHKFAST_INPLACE_FAST(tv_ret);
#endif
	duk_set_top(ctx, idx_func + 1);  /* XXX: could be eliminated with valstack adjust */

	/* [ ... errobj ] */

	/* Ensure there is internal valstack spare before we exit; this may
	 * throw an alloc error.  The same guaranteed size must be available
	 * as before the call.  This is not optimal now: we store the valstack
	 * allocated size during entry; this value may be higher than the
	 * minimal guarantee for an application.
	 */

	/* XXX: this needs to be reworked so that we never shrink the value
	 * stack on function entry so that we never need to grow it here.
	 * Needing to grow here is a sandboxing issue because we need to
	 * allocate which may cause an error in the error handling path
	 * and thus propagate an error out of a protected call.
	 */

	(void) duk_valstack_resize_raw((duk_context *) thr,
	                               entry_valstack_end,                    /* same as during entry */
	                               DUK_VSRESIZE_FLAG_SHRINK |             /* flags */
	                               DUK_VSRESIZE_FLAG_COMPACT |
	                               DUK_VSRESIZE_FLAG_THROW);


	/* These are just convenience "wiping" of state.  Side effects should
	 * not be an issue here: thr->heap and thr->heap->lj have a stable
	 * pointer.  Finalizer runs etc capture even out-of-memory errors so
	 * nothing should throw here.
	 */
	thr->heap->lj.type = DUK_LJ_TYPE_UNKNOWN;
	thr->heap->lj.iserror = 0;
	DUK_TVAL_SET_UNDEFINED_UPDREF(thr, &thr->heap->lj.value1);  /* side effects */
	DUK_TVAL_SET_UNDEFINED_UPDREF(thr, &thr->heap->lj.value2);  /* side effects */

	/* Restore entry thread executor curr_pc stack frame pointer. */
	thr->ptr_curr_pc = entry_ptr_curr_pc;

	DUK_HEAP_SWITCH_THREAD(thr->heap, entry_curr_thread);  /* may be NULL */
	thr->state = (duk_uint8_t) entry_thread_state;

	/* Disabled assert: triggered with some torture tests. */
#if 0
	DUK_ASSERT((thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread == NULL) ||  /* first call */
	           (thr->state == DUK_HTHREAD_STATE_INACTIVE && thr->heap->curr_thread != NULL) ||  /* other call */
	           (thr->state == DUK_HTHREAD_STATE_RUNNING && thr->heap->curr_thread == thr));     /* current thread */
#endif

	thr->heap->call_recursion_depth = entry_call_recursion_depth;

	/* If the debugger is active we need to force an interrupt so that
	 * debugger breakpoints are rechecked.  This is important for function
	 * calls caused by side effects (e.g. when doing a DUK_OP_GETPROP), see
	 * GH-303.  Only needed for success path, error path always causes a
	 * breakpoint recheck in the executor.  It would be enough to set this
	 * only when returning to an Ecmascript activation, but setting the flag
	 * on every return should have no ill effect.
	 */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	if (duk_debug_is_attached(thr->heap)) {
		DUK_DD(DUK_DDPRINT("returning with debugger enabled, force interrupt"));
		DUK_ASSERT(thr->interrupt_counter <= thr->interrupt_init);
		thr->interrupt_init -= thr->interrupt_counter;
		thr->interrupt_counter = 0;
		thr->heap->dbg_force_restart = 1;
	}
#endif

#if defined(DUK_USE_INTERRUPT_COUNTER) && defined(DUK_USE_DEBUG)
	duk__interrupt_fixup(thr, entry_curr_thread);
#endif

	/* Error handling complete, remove side effect protections and
	 * process pending finalizers.
	 */
#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(thr->heap->error_not_allowed == 1);
	thr->heap->error_not_allowed = 0;
#endif
	DUK_ASSERT(thr->heap->pf_prevent_count > 0);
	thr->heap->pf_prevent_count--;
	DUK_DD(DUK_DDPRINT("call error handled, pf_prevent_count updated to %ld", (long) thr->heap->pf_prevent_count));

	DUK_ASSERT_LJSTATE_UNSET(thr->heap);

	DUK_REFZERO_CHECK_SLOW(thr);
}

/*
 *  duk_handle_safe_call(): make a "C protected call" within the
 *  current activation.
 *
 *  The allowed thread states for making a call are the same as for
 *  duk_handle_call_xxx().
 *
 *  Error handling is similar to duk_handle_call_xxx(); errors may be thrown
 *  (and result in a fatal error) for insane arguments.
 */

/* XXX: bump preventcount by one for the duration of this call? */

DUK_INTERNAL duk_int_t duk_handle_safe_call(duk_hthread *thr,
                                            duk_safe_call_function func,
                                            void *udata,
                                            duk_idx_t num_stack_args,
                                            duk_idx_t num_stack_rets) {
	duk_context *ctx = (duk_context *) thr;
	duk_size_t entry_valstack_bottom_index;
	duk_size_t entry_callstack_top;
	duk_size_t entry_catchstack_top;
	duk_int_t entry_call_recursion_depth;
	duk_hthread *entry_curr_thread;
	duk_uint_fast8_t entry_thread_state;
	duk_instr_t **entry_ptr_curr_pc;
	duk_jmpbuf *old_jmpbuf_ptr = NULL;
	duk_jmpbuf our_jmpbuf;
	duk_idx_t idx_retbase;
	duk_int_t retval;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	/* Note: careful with indices like '-x'; if 'x' is zero, it refers to bottom */
	entry_valstack_bottom_index = (duk_size_t) (thr->valstack_bottom - thr->valstack);
	entry_callstack_top = thr->callstack_top;
	entry_catchstack_top = thr->catchstack_top;
	entry_call_recursion_depth = thr->heap->call_recursion_depth;
	entry_curr_thread = thr->heap->curr_thread;  /* Note: may be NULL if first call */
	entry_thread_state = thr->state;
	entry_ptr_curr_pc = thr->ptr_curr_pc;  /* may be NULL */
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
		/* Since stack indices are not reliable, we can't do anything useful
		 * here.  Invoke the existing setjmp catcher, or if it doesn't exist,
		 * call the fatal error handler.
		 */

		DUK_ERROR_TYPE_INVALID_ARGS(thr);
	}

	/* setjmp catchpoint setup */

	old_jmpbuf_ptr = thr->heap->lj.jmpbuf_ptr;
	thr->heap->lj.jmpbuf_ptr = &our_jmpbuf;

#if defined(DUK_USE_CPP_EXCEPTIONS)
	try {
#else
	DUK_ASSERT(thr->heap->lj.jmpbuf_ptr == &our_jmpbuf);
	if (DUK_SETJMP(our_jmpbuf.jb) == 0) {
		/* Success path. */
#endif
		DUK_DDD(DUK_DDDPRINT("safe_call setjmp catchpoint setup complete"));

		duk__handle_safe_call_inner(thr,
		                            func,
		                            udata,
		                            idx_retbase,
		                            num_stack_rets,
		                            entry_valstack_bottom_index,
		                            entry_callstack_top,
		                            entry_catchstack_top);

		/* Note: either pointer may be NULL (at entry), so don't assert */
		thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

		retval = DUK_EXEC_SUCCESS;
#if defined(DUK_USE_CPP_EXCEPTIONS)
	} catch (duk_internal_exception &exc) {
		DUK_UNREF(exc);
#else
	} else {
		/* Error path. */
#endif
		duk__handle_safe_call_error(thr,
		                            idx_retbase,
		                            num_stack_rets,
		                            entry_valstack_bottom_index,
		                            entry_callstack_top,
		                            entry_catchstack_top,
		                            old_jmpbuf_ptr);

		retval = DUK_EXEC_ERROR;
	}
#if defined(DUK_USE_CPP_EXCEPTIONS)
	catch (std::exception &exc) {
		const char *what = exc.what();
		if (!what) {
			what = "unknown";
		}
		DUK_D(DUK_DPRINT("unexpected c++ std::exception (perhaps thrown by user code)"));
		try {
			DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "caught invalid c++ std::exception '%s' (perhaps thrown by user code)", what);
		} catch (duk_internal_exception exc) {
			DUK_D(DUK_DPRINT("caught api error thrown from unexpected c++ std::exception"));
			DUK_UNREF(exc);
			duk__handle_safe_call_error(thr,
			                            idx_retbase,
			                            num_stack_rets,
			                            entry_valstack_bottom_index,
			                            entry_callstack_top,
			                            entry_catchstack_top,
			                            old_jmpbuf_ptr);
			retval = DUK_EXEC_ERROR;
		}
	} catch (...) {
		DUK_D(DUK_DPRINT("unexpected c++ exception (perhaps thrown by user code)"));
		try {
			DUK_ERROR_TYPE(thr, "caught invalid c++ exception (perhaps thrown by user code)");
		} catch (duk_internal_exception exc) {
			DUK_D(DUK_DPRINT("caught api error thrown from unexpected c++ exception"));
			DUK_UNREF(exc);
			duk__handle_safe_call_error(thr,
			                            idx_retbase,
			                            num_stack_rets,
			                            entry_valstack_bottom_index,
			                            entry_callstack_top,
			                            entry_catchstack_top,
			                            old_jmpbuf_ptr);
			retval = DUK_EXEC_ERROR;
		}
	}
#endif

	DUK_ASSERT(thr->heap->lj.jmpbuf_ptr == old_jmpbuf_ptr);  /* success/error path both do this */

	DUK_ASSERT_LJSTATE_UNSET(thr->heap);

	duk__handle_safe_call_shared(thr,
	                             idx_retbase,
	                             num_stack_rets,
	                             entry_call_recursion_depth,
	                             entry_curr_thread,
	                             entry_thread_state,
	                             entry_ptr_curr_pc);

	return retval;
}

DUK_LOCAL void duk__handle_safe_call_inner(duk_hthread *thr,
                                           duk_safe_call_function func,
                                           void *udata,
                                           duk_idx_t idx_retbase,
                                           duk_idx_t num_stack_rets,
                                           duk_size_t entry_valstack_bottom_index,
                                           duk_size_t entry_callstack_top,
                                           duk_size_t entry_catchstack_top) {
	duk_context *ctx;
	duk_ret_t rc;

	DUK_ASSERT(thr != NULL);
	ctx = (duk_context *) thr;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(entry_valstack_bottom_index);
	DUK_UNREF(entry_callstack_top);
	DUK_UNREF(entry_catchstack_top);

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
		DUK_ERROR_RANGE(thr, DUK_STR_C_CALLSTACK_LIMIT);
	}
	thr->heap->call_recursion_depth++;

	/*
	 *  Valstack spare check
	 */

	duk_require_stack(ctx, 0);  /* internal spare */

	/*
	 *  Make the C call
	 */

	rc = func(ctx, udata);

	DUK_DDD(DUK_DDDPRINT("safe_call, func rc=%ld", (long) rc));

	/*
	 *  Valstack manipulation for results.
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
		DUK_ERROR_RANGE(thr, "not enough stack values for safe_call rc");
	}

	DUK_ASSERT(thr->catchstack_top == entry_catchstack_top);  /* no need to unwind */
	DUK_ASSERT(thr->callstack_top == entry_callstack_top);

	duk__safe_call_adjust_valstack(thr, idx_retbase, num_stack_rets, rc);

	DUK_ASSERT_LJSTATE_UNSET(thr->heap);

	DUK_REFZERO_CHECK_FAST(thr);
	return;

 thread_state_error:
	DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "invalid thread state for safe_call (%ld)", (long) thr->state);
	DUK_UNREACHABLE();
}

DUK_LOCAL void duk__handle_safe_call_error(duk_hthread *thr,
                                           duk_idx_t idx_retbase,
                                           duk_idx_t num_stack_rets,
                                           duk_size_t entry_valstack_bottom_index,
                                           duk_size_t entry_callstack_top,
                                           duk_size_t entry_catchstack_top,
                                           duk_jmpbuf *old_jmpbuf_ptr) {
	duk_context *ctx;

	DUK_ASSERT(thr != NULL);
	ctx = (duk_context *) thr;
	DUK_ASSERT_CTX_VALID(ctx);

	/*
	 *  Error during call.  The error value is at heap->lj.value1.
	 *
	 *  The very first thing we do is restore the previous setjmp catcher.
	 *  This means that any error in error handling will propagate outwards
	 *  instead of causing a setjmp() re-entry above.
	 */

	DUK_DDD(DUK_DDDPRINT("error caught during protected duk_handle_safe_call()"));

	/* Other longjmp types are handled by executor before propagating
	 * the error here.
	 */
	DUK_ASSERT(thr->heap->lj.type == DUK_LJ_TYPE_THROW);
	DUK_ASSERT_LJSTATE_SET(thr->heap);
	DUK_ASSERT(thr->callstack_top >= entry_callstack_top);
	DUK_ASSERT(thr->catchstack_top >= entry_catchstack_top);

	/* Note: either pointer may be NULL (at entry), so don't assert. */
	thr->heap->lj.jmpbuf_ptr = old_jmpbuf_ptr;

	DUK_ASSERT(thr->catchstack_top >= entry_catchstack_top);
	DUK_ASSERT(thr->callstack_top >= entry_callstack_top);
	duk_hthread_catchstack_unwind_norz(thr, entry_catchstack_top);
	duk_hthread_catchstack_shrink_check(thr);
	duk_hthread_callstack_unwind_norz(thr, entry_callstack_top);
	duk_hthread_callstack_shrink_check(thr);
	thr->valstack_bottom = thr->valstack + entry_valstack_bottom_index;

	/* [ ... | (crud) ] */

	/* XXX: space in valstack?  see discussion in duk_handle_call_xxx(). */
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

	/* These are just convenience "wiping" of state.  Side effects should
	 * not be an issue here: thr->heap and thr->heap->lj have a stable
	 * pointer.  Finalizer runs etc capture even out-of-memory errors so
	 * nothing should throw here.
	 */
	thr->heap->lj.type = DUK_LJ_TYPE_UNKNOWN;
	thr->heap->lj.iserror = 0;
	DUK_TVAL_SET_UNDEFINED_UPDREF(thr, &thr->heap->lj.value1);  /* side effects */
	DUK_TVAL_SET_UNDEFINED_UPDREF(thr, &thr->heap->lj.value2);  /* side effects */

	/* Error handling complete, remove side effect protections and
	 * process pending finalizers.
	 */
#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(thr->heap->error_not_allowed == 1);
	thr->heap->error_not_allowed = 0;
#endif
	DUK_ASSERT(thr->heap->pf_prevent_count > 0);
	thr->heap->pf_prevent_count--;
	DUK_DD(DUK_DDPRINT("safe call error handled, pf_prevent_count updated to %ld", (long) thr->heap->pf_prevent_count));

	DUK_ASSERT_LJSTATE_UNSET(thr->heap);

	DUK_REFZERO_CHECK_SLOW(thr);
}

DUK_LOCAL void duk__handle_safe_call_shared(duk_hthread *thr,
                                            duk_idx_t idx_retbase,
                                            duk_idx_t num_stack_rets,
                                            duk_int_t entry_call_recursion_depth,
                                            duk_hthread *entry_curr_thread,
                                            duk_uint_fast8_t entry_thread_state,
                                            duk_instr_t **entry_ptr_curr_pc) {
	duk_context *ctx;

	DUK_ASSERT(thr != NULL);
	ctx = (duk_context *) thr;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(ctx);
	DUK_UNREF(idx_retbase);
	DUK_UNREF(num_stack_rets);

	/* Restore entry thread executor curr_pc stack frame pointer. */
	thr->ptr_curr_pc = entry_ptr_curr_pc;

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

	/* A debugger forced interrupt check is not needed here, as
	 * problematic safe calls are not caused by side effects.
	 */

#if defined(DUK_USE_INTERRUPT_COUNTER) && defined(DUK_USE_DEBUG)
	duk__interrupt_fixup(thr, entry_curr_thread);
#endif

	DUK_ASSERT_LJSTATE_UNSET(thr->heap);
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

DUK_INTERNAL duk_bool_t duk_handle_ecma_call_setup(duk_hthread *thr,
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
	duk_instr_t **entry_ptr_curr_pc;

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

	/* If thr->ptr_curr_pc is set, sync curr_pc to act->pc.  Then NULL
	 * thr->ptr_curr_pc so that it's not accidentally used with an incorrect
	 * activation when side effects occur.  If we end up not making the
	 * call we must restore the value.
	 */
	entry_ptr_curr_pc = thr->ptr_curr_pc;
	duk_hthread_sync_and_null_currpc(thr);

	/* if a tail call:
	 *   - an Ecmascript activation must be on top of the callstack
	 *   - there cannot be any active catchstack entries
	 */
#if defined(DUK_USE_ASSERTIONS)
	if (call_flags & DUK_CALL_FLAG_IS_TAILCALL) {
		duk_size_t our_callstack_index;
		duk_size_t i;

		DUK_ASSERT(thr->callstack_top >= 1);
		our_callstack_index = thr->callstack_top - 1;
		DUK_ASSERT_DISABLE(our_callstack_index >= 0);
		DUK_ASSERT(our_callstack_index < thr->callstack_size);
		DUK_ASSERT(DUK_ACT_GET_FUNC(thr->callstack + our_callstack_index) != NULL);
		DUK_ASSERT(DUK_HOBJECT_IS_COMPFUNC(DUK_ACT_GET_FUNC(thr->callstack + our_callstack_index)));

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
	/* XXX: rework */
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

	if (DUK_UNLIKELY(idx_func < 0 || idx_args < 0)) {
		/* XXX: assert? compiler is responsible for this never happening */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
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
	if (func == NULL || !DUK_HOBJECT_IS_COMPFUNC(func)) {
		DUK_DDD(DUK_DDDPRINT("final target is a lightfunc/nativefunc, cannot do ecma-to-ecma call"));
		thr->ptr_curr_pc = entry_ptr_curr_pc;
		return 0;
	}
	/* XXX: tv_func is not actually needed */

	DUK_ASSERT(func != NULL);
	DUK_ASSERT(!DUK_HOBJECT_HAS_BOUNDFUNC(func));
	DUK_ASSERT(DUK_HOBJECT_IS_COMPFUNC(func));

	duk__coerce_effective_this_binding(thr, func, idx_func + 1);
	DUK_DDD(DUK_DDDPRINT("effective 'this' binding is: %!T",
	                     duk_get_tval(ctx, idx_func + 1)));

	nargs = ((duk_hcompfunc *) func)->nargs;
	nregs = ((duk_hcompfunc *) func)->nregs;
	DUK_ASSERT(nregs >= nargs);

	/* [ ... func this arg1 ... argN ] */

	/*
	 *  Preliminary activation record and valstack manipulation.
	 *  The concrete actions depend on whether the we're dealing
	 *  with a tail call (reuse an existing activation), a resume,
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

		act = thr->callstack_curr;
		DUK_ASSERT(act != NULL);
		if (act->flags & DUK_ACT_FLAG_PREVENT_YIELD) {
			/* See: test-bug-tailcall-preventyield-assert.c. */
			DUK_DDD(DUK_DDDPRINT("tail call prevented by current activation having DUK_ACT_FLAG_PREVENTYIELD"));
			use_tailcall = 0;
		} else if (DUK_HOBJECT_HAS_NOTAIL(func)) {
			DUK_D(DUK_DPRINT("tail call prevented by function having a notail flag"));
			use_tailcall = 0;
		}
	}

	if (use_tailcall) {
		duk_tval *tv1, *tv2;
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
		 *  because there may be non-error-catching label entries in valid tail calls.
		 */

		DUK_DDD(DUK_DDDPRINT("is tail call, reusing activation at callstack top, at index %ld",
		                     (long) (thr->callstack_top - 1)));

		/* 'act' already set above */

		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUNDFUNC(func));
		DUK_ASSERT(!DUK_HOBJECT_HAS_NATFUNC(func));
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPFUNC(func));
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
		duk_hthread_catchstack_unwind_norz(thr, i_stk + 1);

		/* Unwind the topmost callstack entry before reusing it */
		DUK_ASSERT(thr->callstack_top > 0);
		duk_hthread_callstack_unwind_norz(thr, thr->callstack_top - 1);

		/* Then reuse the unwound activation; callstack was not shrunk so there is always space */
		DUK_ASSERT(thr->callstack_top < thr->callstack_size);
		act = thr->callstack + thr->callstack_top;
		thr->callstack_top++;
		thr->callstack_curr = act;

		/* Start filling in the activation */
		act->func = func;  /* don't want an intermediate exposed state with func == NULL */
#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
		act->prev_caller = NULL;
#endif
		DUK_ASSERT(func != NULL);
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPFUNC(func));
		/* don't want an intermediate exposed state with invalid pc */
		act->curr_pc = DUK_HCOMPFUNC_GET_CODE_BASE(thr->heap, (duk_hcompfunc *) func);
#if defined(DUK_USE_DEBUGGER_SUPPORT)
		act->prev_line = 0;
#endif
		DUK_TVAL_SET_OBJECT(&act->tv_func, func);  /* borrowed, no refcount */
#if defined(DUK_USE_REFERENCE_COUNTING)
		DUK_HOBJECT_INCREF(thr, func);
		act = thr->callstack_curr;  /* side effects (currently none though) */
#endif

#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
#if defined(DUK_USE_TAILCALL)
#error incorrect options: tail calls enabled with function caller property
#endif
		/* XXX: this doesn't actually work properly for tail calls, so
		 * tail calls are disabled when DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
		 * is in use.
		 */
		duk__update_func_caller_prop(thr, func);
		act = thr->callstack_curr;
#endif

		act->flags = (DUK_HOBJECT_HAS_STRICT(func) ?
		              DUK_ACT_FLAG_STRICT | DUK_ACT_FLAG_TAILCALLED :
		              DUK_ACT_FLAG_TAILCALLED);

		DUK_ASSERT(DUK_ACT_GET_FUNC(act) == func);      /* already updated */
		DUK_ASSERT(act->var_env == NULL);   /* already NULLed (by unwind) */
		DUK_ASSERT(act->lex_env == NULL);   /* already NULLed (by unwind) */
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
		 *  For tail calling to work properly, the valstack bottom must not grow
		 *  here; otherwise crud would accumulate on the valstack.
		 */

		tv1 = thr->valstack_bottom - 1;
		tv2 = thr->valstack_bottom + idx_func + 1;
		DUK_ASSERT(tv1 >= thr->valstack && tv1 < thr->valstack_top);  /* tv1 is -below- valstack_bottom */
		DUK_ASSERT(tv2 >= thr->valstack_bottom && tv2 < thr->valstack_top);
		DUK_TVAL_SET_TVAL_UPDREF(thr, tv1, tv2);  /* side effects */

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
		DUK_DDD(DUK_DDDPRINT("not a tail call, pushing a new activation to callstack, to index %ld",
		                     (long) (thr->callstack_top)));

		duk_hthread_callstack_grow(thr);

		if (call_flags & DUK_CALL_FLAG_IS_RESUME) {
			DUK_DDD(DUK_DDDPRINT("is resume -> no update to current activation (may not even exist)"));
		} else {
			DUK_DDD(DUK_DDDPRINT("update to current activation idx_retval"));
			DUK_ASSERT(thr->callstack_top < thr->callstack_size);
			DUK_ASSERT(thr->callstack_top >= 1);
			act = thr->callstack_curr;
			DUK_ASSERT(DUK_ACT_GET_FUNC(act) != NULL);
			DUK_ASSERT(DUK_HOBJECT_IS_COMPFUNC(DUK_ACT_GET_FUNC(act)));
			act->idx_retval = entry_valstack_bottom_index + idx_func;
		}

		DUK_ASSERT(thr->callstack_top < thr->callstack_size);
		act = thr->callstack + thr->callstack_top;
		thr->callstack_top++;
		thr->callstack_curr = act;
		DUK_ASSERT(thr->callstack_top <= thr->callstack_size);

		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUNDFUNC(func));
		DUK_ASSERT(!DUK_HOBJECT_HAS_NATFUNC(func));
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPFUNC(func));

		act->flags = (DUK_HOBJECT_HAS_STRICT(func) ?
		              DUK_ACT_FLAG_STRICT :
		              0);
		act->func = func;
		act->var_env = NULL;
		act->lex_env = NULL;
#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
		act->prev_caller = NULL;
#endif
		DUK_ASSERT(func != NULL);
		DUK_ASSERT(DUK_HOBJECT_HAS_COMPFUNC(func));
		act->curr_pc = DUK_HCOMPFUNC_GET_CODE_BASE(thr->heap, (duk_hcompfunc *) func);
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

#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
		duk__update_func_caller_prop(thr, func);
		act = thr->callstack_curr;
#endif
	}

	/* [ ... func this arg1 ... argN ]  (not tail call)
	 * [ this | arg1 ... argN ]         (tail call)
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

	/* XXX: unify handling with native call. */

	DUK_ASSERT(!DUK_HOBJECT_HAS_BOUNDFUNC(func));  /* bound function chain has already been resolved */

	if (!DUK_HOBJECT_HAS_NEWENV(func)) {
		/* use existing env (e.g. for non-strict eval); cannot have
		 * an own 'arguments' object (but can refer to the existing one)
		 */

		duk__handle_oldenv_for_call(thr, func, act);
		/* No need to re-lookup 'act' at present: no side effects. */

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

	/* [ ... arg1 ... argN envobj ] */

	/* original input stack before nargs/nregs handling must be
	 * intact for 'arguments' object
	 */
	DUK_ASSERT(DUK_HOBJECT_HAS_CREATEARGS(func));
	duk__handle_createargs_for_call(thr, func, env, num_stack_args);

	/* [ ... arg1 ... argN envobj ] */

	act = thr->callstack_curr;
	act->lex_env = env;
	act->var_env = env;
	DUK_HOBJECT_INCREF(thr, act->lex_env);
	DUK_HOBJECT_INCREF(thr, act->var_env);
	duk_pop(ctx);

 env_done:
	/* [ ... arg1 ... argN ] */

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

	DUK_REFZERO_CHECK_FAST(thr);
	return 1;
}
