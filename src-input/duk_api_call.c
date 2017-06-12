/*
 *  Calls.
 *
 *  Protected variants should avoid ever throwing an error.
 */

#include "duk_internal.h"

/* Prepare value stack for a method call through an object property.
 * May currently throw an error e.g. when getting the property.
 */
DUK_LOCAL void duk__call_prop_prep_stack(duk_context *ctx, duk_idx_t normalized_obj_idx, duk_idx_t nargs) {
	DUK_ASSERT_CTX_VALID(ctx);

	DUK_DDD(DUK_DDDPRINT("duk__call_prop_prep_stack, normalized_obj_idx=%ld, nargs=%ld, stacktop=%ld",
	                     (long) normalized_obj_idx, (long) nargs, (long) duk_get_top(ctx)));

	/* [... key arg1 ... argN] */

	/* duplicate key */
	duk_dup(ctx, -nargs - 1);  /* Note: -nargs alone would fail for nargs == 0, this is OK */
	duk_get_prop(ctx, normalized_obj_idx);

	DUK_DDD(DUK_DDDPRINT("func: %!T", (duk_tval *) duk_get_tval(ctx, -1)));

	/* [... key arg1 ... argN func] */

	duk_replace(ctx, -nargs - 2);

	/* [... func arg1 ... argN] */

	duk_dup(ctx, normalized_obj_idx);
	duk_insert(ctx, -nargs - 1);

	/* [... func this arg1 ... argN] */
}

DUK_EXTERNAL void duk_call(duk_context *ctx, duk_idx_t nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_small_uint_t call_flags;
	duk_idx_t idx_func;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 1;
	if (DUK_UNLIKELY(idx_func < 0 || nargs < 0)) {
		/* note that we can't reliably pop anything here */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
	}

	/* XXX: awkward; we assume there is space for this, overwrite
	 * directly instead?
	 */
	duk_push_undefined(ctx);
	duk_insert(ctx, idx_func + 1);

	call_flags = 0;  /* not protected, respect reclimit, not constructor */

	duk_handle_call_unprotected(thr,           /* thread */
	                            nargs,         /* num_stack_args */
	                            call_flags);   /* call_flags */
}

DUK_EXTERNAL void duk_call_method(duk_context *ctx, duk_idx_t nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_small_uint_t call_flags;
	duk_idx_t idx_func;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 2;  /* must work for nargs <= 0 */
	if (DUK_UNLIKELY(idx_func < 0 || nargs < 0)) {
		/* note that we can't reliably pop anything here */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
	}

	call_flags = 0;  /* not protected, respect reclimit, not constructor */

	duk_handle_call_unprotected(thr,           /* thread */
	                            nargs,         /* num_stack_args */
	                            call_flags);   /* call_flags */
}

DUK_EXTERNAL void duk_call_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs) {
	/*
	 *  XXX: if duk_handle_call() took values through indices, this could be
	 *  made much more sensible.  However, duk_handle_call() needs to fudge
	 *  the 'this' and 'func' values to handle bound function chains, which
	 *  is now done "in-place", so this is not a trivial change.
	 */

	DUK_ASSERT_CTX_VALID(ctx);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);  /* make absolute */

	duk__call_prop_prep_stack(ctx, obj_idx, nargs);

	duk_call_method(ctx, nargs);
}

DUK_EXTERNAL duk_int_t duk_pcall(duk_context *ctx, duk_idx_t nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_small_uint_t call_flags;
	duk_idx_t idx_func;
	duk_int_t rc;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 1;  /* must work for nargs <= 0 */
	if (DUK_UNLIKELY(idx_func < 0 || nargs < 0)) {
		/* We can't reliably pop anything here because the stack input
		 * shape is incorrect.  So we throw an error; if the caller has
		 * no catch point for this, a fatal error will occur.  Another
		 * alternative would be to just return an error.  But then the
		 * stack would be in an unknown state which might cause some
		 * very hard to diagnose problems later on.  Also note that even
		 * if we did not throw an error here, the underlying call handler
		 * might STILL throw an out-of-memory error or some other internal
		 * fatal error.
		 */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
		return DUK_EXEC_ERROR;  /* unreachable */
	}

	/* awkward; we assume there is space for this */
	duk_push_undefined(ctx);
	duk_insert(ctx, idx_func + 1);

	call_flags = 0;  /* respect reclimit, not constructor */

	rc = duk_handle_call_protected(thr,           /* thread */
	                               nargs,         /* num_stack_args */
	                               call_flags);   /* call_flags */

	return rc;
}

DUK_EXTERNAL duk_int_t duk_pcall_method(duk_context *ctx, duk_idx_t nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_small_uint_t call_flags;
	duk_idx_t idx_func;
	duk_int_t rc;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 2;  /* must work for nargs <= 0 */
	if (DUK_UNLIKELY(idx_func < 0 || nargs < 0)) {
		/* See comments in duk_pcall(). */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
		return DUK_EXEC_ERROR;  /* unreachable */
	}

	call_flags = 0;  /* respect reclimit, not constructor */

	rc = duk_handle_call_protected(thr,           /* thread */
	                               nargs,         /* num_stack_args */
	                               call_flags);   /* call_flags */

	return rc;
}

struct duk__pcall_prop_args {
	duk_idx_t obj_idx;
	duk_idx_t nargs;
};
typedef struct duk__pcall_prop_args duk__pcall_prop_args;

DUK_LOCAL duk_ret_t duk__pcall_prop_raw(duk_context *ctx, void *udata) {
	duk_idx_t obj_idx;
	duk_idx_t nargs;
	duk__pcall_prop_args *args;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(udata != NULL);

	args = (duk__pcall_prop_args *) udata;
	obj_idx = args->obj_idx;
	nargs = args->nargs;

	obj_idx = duk_require_normalize_index(ctx, obj_idx);  /* make absolute */
	duk__call_prop_prep_stack(ctx, obj_idx, nargs);
	duk_call_method(ctx, nargs);
	return 1;
}

DUK_EXTERNAL duk_int_t duk_pcall_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk__pcall_prop_args args;

	/*
	 *  Must be careful to catch errors related to value stack manipulation
	 *  and property lookup, not just the call itself.
	 */

	DUK_ASSERT_CTX_VALID(ctx);

	args.obj_idx = obj_idx;
	args.nargs = nargs;
	if (DUK_UNLIKELY(nargs < 0)) {
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
		return DUK_EXEC_ERROR;  /* unreachable */
	}

	/* Inputs: explicit arguments (nargs), +1 for key.  If the value stack
	 * does not contain enough args, an error is thrown; this matches
	 * behavior of the other protected call API functions.
	 */
	return duk_safe_call(ctx, duk__pcall_prop_raw, (void *) &args /*udata*/, nargs + 1 /*nargs*/, 1 /*nrets*/);
}

DUK_EXTERNAL duk_int_t duk_safe_call(duk_context *ctx, duk_safe_call_function func, void *udata, duk_idx_t nargs, duk_idx_t nrets) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_int_t rc;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	if (DUK_UNLIKELY(duk_get_top(ctx) < nargs || nargs < 0 || nrets < 0)) {
		/* See comments in duk_pcall(). */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
		return DUK_EXEC_ERROR;  /* unreachable */
	}

	rc = duk_handle_safe_call(thr,           /* thread */
	                          func,          /* func */
	                          udata,         /* udata */
	                          nargs,         /* num_stack_args */
	                          nrets);        /* num_stack_res */

	return rc;
}

DUK_EXTERNAL void duk_new(duk_context *ctx, duk_idx_t nargs) {
	/*
	 *  There are two [[Construct]] operations in the specification:
	 *
	 *    - E5 Section 13.2.2: for Function objects
	 *    - E5 Section 15.3.4.5.2: for "bound" Function objects
	 *
	 *  The chain of bound functions is resolved in Section 15.3.4.5.2,
	 *  with arguments "piling up" until the [[Construct]] internal
	 *  method is called on the final, actual Function object.  Note
	 *  that the "prototype" property is looked up *only* from the
	 *  final object, *before* calling the constructor.
	 *
	 *  Currently we follow the bound function chain here to get the
	 *  "prototype" property value from the final, non-bound function.
	 *  However, we let duk_handle_call() handle the argument "piling"
	 *  when the constructor is called.  The bound function chain is
	 *  thus now processed twice.
	 *
	 *  When constructing new Array instances, an unnecessary object is
	 *  created and discarded now: the standard [[Construct]] creates an
	 *  object, and calls the Array constructor.  The Array constructor
	 *  returns an Array instance, which is used as the result value for
	 *  the "new" operation; the object created before the Array constructor
	 *  call is discarded.
	 *
	 *  This would be easy to fix, e.g. by knowing that the Array constructor
	 *  will always create a replacement object and skip creating the fallback
	 *  object in that case.
	 *
	 *  Note: functions called via "new" need to know they are called as a
	 *  constructor.  For instance, built-in constructors behave differently
	 *  depending on how they are called.
	 */

	/* XXX: merge this with duk_js_call.c, as this function implements
	 * core semantics (or perhaps merge the two files altogether).
	 */

	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *proto;
	duk_hobject *cons;
	duk_hobject *fallback;
	duk_idx_t idx_cons;
	duk_small_uint_t call_flags;

	DUK_ASSERT_CTX_VALID(ctx);

	/* [... constructor arg1 ... argN] */

	if (DUK_UNLIKELY(nargs < 0)) {
		/* note that we can't reliably pop anything here */
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
	}
	idx_cons = duk_require_normalize_index(ctx, -nargs - 1);

	DUK_DDD(DUK_DDDPRINT("top=%ld, nargs=%ld, idx_cons=%ld",
	                     (long) duk_get_top(ctx), (long) nargs, (long) idx_cons));

	/* XXX: code duplication */

	/*
	 *  Figure out the final, non-bound constructor, to get "prototype"
	 *  property.
	 */

	duk_dup(ctx, idx_cons);
	for (;;) {
		duk_tval *tv;
		tv = DUK_GET_TVAL_NEGIDX(ctx, -1);
		DUK_ASSERT(tv != NULL);

		if (DUK_TVAL_IS_OBJECT(tv)) {
			cons = DUK_TVAL_GET_OBJECT(tv);
			DUK_ASSERT(cons != NULL);
			if (!DUK_HOBJECT_IS_CALLABLE(cons) || !DUK_HOBJECT_HAS_CONSTRUCTABLE(cons)) {
				/* Checking callability of the immediate target
				 * is important, same for constructability.
				 * Checking it for functions down the bound
				 * function chain is not strictly necessary
				 * because .bind() should normally reject them.
				 * But it's good to check anyway because it's
				 * technically possible to edit the bound function
				 * chain via internal keys.
				 */
				goto not_constructable;
			}
			if (!DUK_HOBJECT_HAS_BOUNDFUNC(cons)) {
				break;
			}
		} else if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
			/* Lightfuncs cannot be bound. */
			break;
		} else {
			/* Anything else is not constructable. */
			goto not_constructable;
		}
		duk_get_prop_stridx_short(ctx, -1, DUK_STRIDX_INT_TARGET);  /* -> [... cons target] */
		duk_remove_m2(ctx);                                         /* -> [... target] */
	}
	DUK_ASSERT(duk_is_callable(ctx, -1));
	DUK_ASSERT(duk_is_lightfunc(ctx, -1) ||
	           (duk_get_hobject(ctx, -1) != NULL && !DUK_HOBJECT_HAS_BOUNDFUNC(duk_get_hobject(ctx, -1))));

	/* [... constructor arg1 ... argN final_cons] */

	/*
	 *  Create "fallback" object to be used as the object instance,
	 *  unless the constructor returns a replacement value.
	 *  Its internal prototype needs to be set based on "prototype"
	 *  property of the constructor.
	 */

	duk_push_object(ctx);  /* class Object, extensible */

	/* [... constructor arg1 ... argN final_cons fallback] */

	duk_get_prop_stridx_short(ctx, -2, DUK_STRIDX_PROTOTYPE);
	proto = duk_get_hobject(ctx, -1);
	if (!proto) {
		DUK_DDD(DUK_DDDPRINT("constructor has no 'prototype' property, or value not an object "
		                     "-> leave standard Object prototype as fallback prototype"));
	} else {
		DUK_DDD(DUK_DDDPRINT("constructor has 'prototype' property with object value "
		                     "-> set fallback prototype to that value: %!iO", (duk_heaphdr *) proto));
		fallback = duk_get_hobject(ctx, -2);
		DUK_ASSERT(fallback != NULL);
		DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, fallback, proto);
	}
	duk_pop(ctx);

	/* [... constructor arg1 ... argN final_cons fallback] */

	/*
	 *  Manipulate callstack for the call.
	 */

	duk_dup_top(ctx);
	duk_insert(ctx, idx_cons + 1);  /* use fallback as 'this' value */
	duk_insert(ctx, idx_cons);      /* also stash it before constructor,
	                                 * in case we need it (as the fallback value)
	                                 */
	duk_pop(ctx);                   /* pop final_cons */


	/* [... fallback constructor fallback(this) arg1 ... argN];
	 * Note: idx_cons points to first 'fallback', not 'constructor'.
	 */

	DUK_DDD(DUK_DDDPRINT("before call, idx_cons+1 (constructor) -> %!T, idx_cons+2 (fallback/this) -> %!T, "
	                     "nargs=%ld, top=%ld",
	                     (duk_tval *) duk_get_tval(ctx, idx_cons + 1),
	                     (duk_tval *) duk_get_tval(ctx, idx_cons + 2),
	                     (long) nargs,
	                     (long) duk_get_top(ctx)));

	/*
	 *  Call the constructor function (called in "constructor mode").
	 */

	call_flags = DUK_CALL_FLAG_CONSTRUCTOR_CALL;  /* not protected, respect reclimit, is a constructor call */

	duk_handle_call_unprotected(thr,           /* thread */
	                            nargs,         /* num_stack_args */
	                            call_flags);   /* call_flags */

	/* [... fallback retval] */

	DUK_DDD(DUK_DDDPRINT("constructor call finished, fallback=%!iT, retval=%!iT",
	                     (duk_tval *) duk_get_tval(ctx, -2),
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	/*
	 *  Determine whether to use the constructor return value as the created
	 *  object instance or not.
	 */

	if (duk_check_type_mask(ctx, -1, DUK_TYPE_MASK_OBJECT |
	                                 DUK_TYPE_MASK_BUFFER |
	                                 DUK_TYPE_MASK_LIGHTFUNC)) {
		duk_remove_m2(ctx);
	} else {
		duk_pop(ctx);
	}

	/*
	 *  Augment created errors upon creation (not when they are thrown or
	 *  rethrown).  __FILE__ and __LINE__ are not desirable here; the call
	 *  stack reflects the caller which is correct.
	 */

#if defined(DUK_USE_AUGMENT_ERROR_CREATE)
	duk_hthread_sync_currpc(thr);
	duk_err_augment_error_create(thr, thr, NULL, 0, 1 /*noblame_fileline*/);
#endif

	/* [... retval] */

	return;

 not_constructable:
#if defined(DUK_USE_VERBOSE_ERRORS)
#if defined(DUK_USE_PARANOID_ERRORS)
	DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "%s not constructable", duk_get_type_name(ctx, -1));
#else
	DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "%s not constructable", duk_push_string_readable(ctx, -1));
#endif
#else
	DUK_ERROR_TYPE(thr, "not constructable");
#endif
}

DUK_LOCAL duk_ret_t duk__pnew_helper(duk_context *ctx, void *udata) {
	duk_idx_t nargs;

	DUK_ASSERT(udata != NULL);
	nargs = *((duk_idx_t *) udata);

	duk_new(ctx, nargs);
	return 1;
}

DUK_EXTERNAL duk_int_t duk_pnew(duk_context *ctx, duk_idx_t nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_int_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* For now, just use duk_safe_call() to wrap duk_new().  We can't
	 * simply use a protected duk_handle_call() because there's post
	 * processing which might throw.  It should be possible to ensure
	 * the post processing never throws (except in internal errors and
	 * out of memory etc which are always allowed) and then remove this
	 * wrapper.
	 */

	if (DUK_UNLIKELY(nargs < 0)) {
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
		return DUK_EXEC_ERROR;  /* unreachable */
	}

	rc = duk_safe_call(ctx, duk__pnew_helper, (void *) &nargs /*udata*/, nargs + 1 /*nargs*/, 1 /*nrets*/);
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_is_constructor_call(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);

	act = thr->callstack_curr;
	if (act != NULL) {
		return ((act->flags & DUK_ACT_FLAG_CONSTRUCT) != 0 ? 1 : 0);
	}
	return 0;
}

/* XXX: Make this obsolete by adding a function flag for rejecting a
 * non-constructor call automatically?
 */
DUK_INTERNAL void duk_require_constructor_call(duk_context *ctx) {
	if (!duk_is_constructor_call(ctx)) {
		DUK_ERROR_TYPE((duk_hthread *) ctx, DUK_STR_CONSTRUCT_ONLY);
	}
}

DUK_EXTERNAL duk_bool_t duk_is_strict_call(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;

	/* For user code this could just return 1 (strict) always
	 * because all Duktape/C functions are considered strict,
	 * and strict is also the default when nothing is running.
	 * However, Duktape may call this function internally when
	 * the current activation is an Ecmascript function, so
	 * this cannot be replaced by a 'return 1' without fixing
	 * the internal call sites.
	 */

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);

	act = thr->callstack_curr;
	if (act != NULL) {
		return ((act->flags & DUK_ACT_FLAG_STRICT) != 0 ? 1 : 0);
	} else {
		/* Strict by default. */
		return 1;
	}
}

/*
 *  Duktape/C function magic
 */

DUK_EXTERNAL duk_int_t duk_get_current_magic(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;
	duk_hobject *func;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);

	act = thr->callstack_curr;
	if (act) {
		func = DUK_ACT_GET_FUNC(act);
		if (!func) {
			duk_tval *tv = &act->tv_func;
			duk_small_uint_t lf_flags;
			lf_flags = DUK_TVAL_GET_LIGHTFUNC_FLAGS(tv);
			return (duk_int_t) DUK_LFUNC_FLAGS_GET_MAGIC(lf_flags);
		}
		DUK_ASSERT(func != NULL);

		if (DUK_HOBJECT_IS_NATFUNC(func)) {
			duk_hnatfunc *nf = (duk_hnatfunc *) func;
			return (duk_int_t) nf->magic;
		}
	}
	return 0;
}

DUK_EXTERNAL duk_int_t duk_get_magic(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, idx);
	if (DUK_TVAL_IS_OBJECT(tv)) {
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		if (!DUK_HOBJECT_HAS_NATFUNC(h)) {
			goto type_error;
		}
		return (duk_int_t) ((duk_hnatfunc *) h)->magic;
	} else if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
		duk_small_uint_t lf_flags = DUK_TVAL_GET_LIGHTFUNC_FLAGS(tv);
		return (duk_int_t) DUK_LFUNC_FLAGS_GET_MAGIC(lf_flags);
	}

	/* fall through */
 type_error:
	DUK_ERROR_TYPE(thr, DUK_STR_UNEXPECTED_TYPE);
	return 0;
}

DUK_EXTERNAL void duk_set_magic(duk_context *ctx, duk_idx_t idx, duk_int_t magic) {
	duk_hnatfunc *nf;

	DUK_ASSERT_CTX_VALID(ctx);

	nf = duk_require_hnatfunc(ctx, idx);
	DUK_ASSERT(nf != NULL);
	nf->magic = (duk_int16_t) magic;
}

/*
 *  Misc helpers
 */

DUK_INTERNAL void duk_resolve_nonbound_function(duk_context *ctx) {
	duk_uint_t sanity;
	duk_tval *tv;

	sanity = DUK_HOBJECT_BOUND_CHAIN_SANITY;
	do {
		tv = DUK_GET_TVAL_NEGIDX(ctx, -1);
		if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
			/* Lightweight function: never bound, so terminate. */
			break;
		} else if (DUK_TVAL_IS_OBJECT(tv)) {
			duk_hobject *func;

			func = DUK_TVAL_GET_OBJECT(tv);
			DUK_ASSERT(func != NULL);
			if (!DUK_HOBJECT_IS_CALLABLE(func) || !DUK_HOBJECT_HAS_BOUNDFUNC(func)) {
				break;
			}
			duk_get_prop_stridx_short(ctx, -1, DUK_STRIDX_INT_TARGET);
			duk_replace(ctx, -2);
		} else {
			break;
		}
	} while (--sanity > 0);
}
