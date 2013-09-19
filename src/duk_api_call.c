/*
 *  Calls.
 *
 *  Protected variants should avoid ever throwing an error.
 */

#include "duk_internal.h"

static int resolve_errhandler(duk_context *ctx, int pop_count, int errhandler_index, duk_hobject **out_ptr) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(out_ptr != NULL);

	if (errhandler_index == DUK_INVALID_INDEX) {
		/* use existing, if any */
		*out_ptr = thr->heap->lj.errhandler;
		return 1;
	}

	if (duk_is_valid_index(ctx, errhandler_index)) {
		duk_tval *tv = duk_require_tval(ctx, errhandler_index);

		if (DUK_TVAL_IS_OBJECT(tv)) {
			duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
			DUK_ASSERT(h != NULL);
			if (DUK_HOBJECT_IS_CALLABLE(h)) {
				*out_ptr = h;
				return 1;
			}
		} else if (DUK_TVAL_IS_UNDEFINED(tv) || DUK_TVAL_IS_NULL(tv)) {
			/* explicitly force NULL handler */
			*out_ptr = NULL;
			return 1;
		}

		/* fall through to error */
	}

	/*
	 *  Error: don't throw anything here as we're part of the 'pcall' process.
	 *  Instead, simulate an error by pushing an error object on the top of
	 *  stack.  The current error handler will not be called for this particular
	 *  error though.
	 *
	 *  FIXME: this is naturally not reliable as pushing an error object may
	 *  result in out-of-memory.  The API call semantics may need to be changed,
	 *  or perhaps we need to push a pre-built object here to avoid any error
	 *  potential (or instead of a pre-built object a plain value like undefined).
	 */

	DUK_ASSERT(pop_count <= duk_get_top(ctx));  /* caller ensures */

	duk_pop_n(ctx, pop_count);
	(void) duk_push_error_object(ctx, DUK_ERR_API_ERROR, "invalid errhandler");
	return 0;
}

/* Prepare value stack for a method call through an object property.
 * May currently throw an error e.g. when getting the property.
 */
static void call_prop_prep_stack(duk_context *ctx, int normalized_obj_index, int nargs) {
	DUK_DDDPRINT("call_prop_prep_stack, normalized_obj_index=%d, nargs=%d, stacktop=%d",
	             normalized_obj_index, nargs, duk_get_top(ctx));

	/* [... key arg1 ... argN] */

	/* duplicate key */
	duk_dup(ctx, -nargs - 1);  /* Note: -nargs alone would fail for nargs == 0, this is OK */
	duk_get_prop(ctx, normalized_obj_index);

	DUK_DDDPRINT("func: %!T", duk_get_tval(ctx, -1));

	/* [... key arg1 ... argN func] */

	duk_replace(ctx, -nargs - 2);

	/* [... func arg1 ... argN] */

	duk_dup(ctx, normalized_obj_index);
	duk_insert(ctx, -nargs - 1);

	/* [... func this arg1 ... argN] */
}

void duk_call(duk_context *ctx, int nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *errhandler;
	int call_flags;
	int idx_func;
	int rc;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 1;  /* must work for nargs <= 0 */
	if (idx_func < 0 || nargs < 0) {
		/* note that we can't reliably pop anything here */
		DUK_ERROR(ctx, DUK_ERR_API_ERROR, "invalid call args");
	}

	/* awkward; we assume there is space for this */
	duk_push_undefined(ctx);
	duk_insert(ctx, idx_func + 1);

	errhandler = thr->heap->lj.errhandler;  /* use existing one (if any) */

	call_flags = 0;  /* not protected, respect reclimit, not constructor */

	rc = duk_handle_call(thr,           /* thread */
	                     nargs,         /* num_stack_args */
	                     call_flags,    /* call_flags */
	                     errhandler);   /* errhandler */
	DUK_UNREF(rc);
}

void duk_call_method(duk_context *ctx, int nargs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *errhandler;
	int call_flags;
	int idx_func;
	int rc;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 2;  /* must work for nargs <= 0 */
	if (idx_func < 0 || nargs < 0) {
		/* note that we can't reliably pop anything here */
		DUK_ERROR(ctx, DUK_ERR_API_ERROR, "invalid call args");
	}

	errhandler = thr->heap->lj.errhandler;  /* use existing one (if any) */

	call_flags = 0;  /* not protected, respect reclimit, not constructor */

	rc = duk_handle_call(thr,           /* thread */
	                     nargs,         /* num_stack_args */
	                     call_flags,    /* call_flags */
	                     errhandler);   /* errhandler */
	DUK_UNREF(rc);
}

void duk_call_prop(duk_context *ctx, int obj_index, int nargs) {
	/*
	 *  XXX: if duk_handle_call() took values through indices, this could be
	 *  made much more sensible.  However, duk_handle_call() needs to fudge
	 *  the 'this' and 'func' values to handle bound function chains, which
	 *  is now done "in-place", so this is not a trivial change.
	 */

	obj_index = duk_require_normalize_index(ctx, obj_index);  /* make absolute */

	call_prop_prep_stack(ctx, obj_index, nargs);

	duk_call_method(ctx, nargs);
}

int duk_pcall(duk_context *ctx, int nargs, int errhandler_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *errhandler = NULL;
	int call_flags;
	int idx_func;
	int rc;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 1;  /* must work for nargs <= 0 */
	if (idx_func < 0 || nargs < 0) {
		/* note that we can't reliably pop anything here */
		DUK_ERROR(ctx, DUK_ERR_API_ERROR, "invalid call args");
		/* FIXME: actually terminate thread? */
		return DUK_ERR_EXEC_TERM;
	}

	if (!resolve_errhandler(ctx, nargs + 1, errhandler_index, &errhandler)) {
		/* error on top of stack */
		return DUK_ERR_EXEC_ERROR;
	}

	/* awkward; we assume there is space for this */
	duk_push_undefined(ctx);
	duk_insert(ctx, idx_func + 1);

	call_flags = DUK_CALL_FLAG_PROTECTED;  /* protected, respect reclimit, not constructor */

	rc = duk_handle_call(thr,           /* thread */
	                     nargs,         /* num_stack_args */
	                     call_flags,    /* call_flags */
	                     errhandler);   /* errhandler */

	return rc;
}

int duk_pcall_method(duk_context *ctx, int nargs, int errhandler_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *errhandler = NULL;
	int call_flags;
	int idx_func;
	int rc;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	idx_func = duk_get_top(ctx) - nargs - 2;  /* must work for nargs <= 0 */
	if (idx_func < 0 || nargs < 0) {
		/* note that we can't reliably pop anything here */
		/* FIXME: actually terminate thread? */
		return DUK_ERR_EXEC_TERM;
	}

	if (!resolve_errhandler(ctx, nargs + 2, errhandler_index, &errhandler)) {
		/* error on top of stack */
		return DUK_ERR_EXEC_ERROR;
	}

	call_flags = DUK_CALL_FLAG_PROTECTED;  /* protected, respect reclimit, not constructor */

	rc = duk_handle_call(thr,           /* thread */
	                     nargs,         /* num_stack_args */
	                     call_flags,    /* call_flags */
	                     errhandler);   /* errhandler */

	return rc;
}

int duk_pcall_prop(duk_context *ctx, int obj_index, int nargs, int errhandler_index) {
	/* FIXME: these will throw errors now, so this is a bad idea */
	obj_index = duk_require_normalize_index(ctx, obj_index);  /* make absolute */
	call_prop_prep_stack(ctx, obj_index, nargs);

	return duk_pcall_method(ctx, nargs, errhandler_index);
}

int duk_safe_call(duk_context *ctx, duk_safe_call_function func, int nargs, int nrets, int errhandler_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *errhandler = NULL;
	int rc;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	if (duk_get_top(ctx) < nargs || nrets < 0) {
		/* also covers sanity for negative 'nargs'; note that we can't
		 * reliably pop anything here
		 */
		/* FIXME: actually terminate thread? */
		return DUK_ERR_EXEC_TERM;
	}

	if (!resolve_errhandler(ctx, nargs, errhandler_index, &errhandler)) {
		/* error on top of stack (args popped) */
		return DUK_ERR_EXEC_ERROR;
	}

	rc = duk_handle_safe_call(thr,           /* thread */
	                          func,          /* func */
	                          nargs,         /* num_stack_args */
	                          nrets,         /* num_stack_res */
	                          errhandler);   /* errhandler */

	return rc;
}

void duk_new(duk_context *ctx, int nargs) {
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
	 *  object in that case.  FIXME.
	 *
	 *  Note: functions called via "new" need to know they are called as a
	 *  constructor.  For instance, built-in constructors behave differently
	 *  depending on how they are called.
	 */

	/* FIXME: should this go to duk_js_call.c? it implements core semantics. */

	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *proto;
	duk_hobject *cons;
	duk_hobject *fallback;
	int idx_cons;
	duk_hobject *errhandler = NULL;
	int call_flags;
	int rc;

	/* [... constructor arg1 ... argN] */

	idx_cons = duk_require_normalize_index(ctx, -nargs - 1);

	DUK_DDDPRINT("top=%d, nargs=%d, idx_cons=%d", duk_get_top(ctx), nargs, idx_cons);

	/* FIXME: code duplication */

	/*
	 *  Figure out the final, non-bound constructor, to get "prototype"
	 *  property.
	 */

	duk_dup(ctx, idx_cons);
	for (;;) {
		cons = duk_get_hobject(ctx, -1);
		if (cons == NULL || !DUK_HOBJECT_HAS_CONSTRUCTABLE(cons)) {
			/* Checking constructability from anything else than the
			 * initial constructor is not strictly necessary, but a
			 * nice sanity check.
			 */
			goto not_constructable;
		}
		if (!DUK_HOBJECT_HAS_BOUND(cons)) {
			break;
		}
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_TARGET);  /* -> [... cons target] */
		duk_remove(ctx, -2);                                  /* -> [... target] */
	}
	DUK_ASSERT(cons != NULL && !DUK_HOBJECT_HAS_BOUND(cons));

	/* [... constructor arg1 ... argN final_cons] */

	/*
	 *  Create "fallback" object to be used as the object instance,
	 *  unless the constructor returns a replacement value.
	 *  Its internal prototype needs to be set based on "prototype"
	 *  property of the constructor.
	 */

	duk_push_object(ctx);  /* class Object, extensible */

	/* [... constructor arg1 ... argN final_cons fallback] */

	duk_get_prop_stridx(ctx, -2, DUK_STRIDX_PROTOTYPE);
	proto = duk_get_hobject(ctx, -1);
	if (!proto) {
		DUK_DDDPRINT("constructor has no 'prototype' property, or value not an object "
		             "-> leave standard Object prototype as fallback prototype");
	} else {
		DUK_DDDPRINT("constructor has 'prototype' property with object value "
		             "-> set fallback prototype to that value: %!iO", proto);
		fallback = duk_get_hobject(ctx, -2);
		DUK_ASSERT(fallback != NULL);
		DUK_HOBJECT_SET_PROTOTYPE(thr, fallback, proto);
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

	DUK_DDDPRINT("before call, idx_cons+1 (constructor) -> %!T, idx_cons+2 (fallback/this) -> %!T, "
	             "nargs=%d, top=%d",
	             duk_get_tval(ctx, idx_cons + 1), duk_get_tval(ctx, idx_cons + 2),
	             nargs, duk_get_top(ctx));

	/*
	 *  Call the constructor function (called in "constructor mode").
	 */

	errhandler = thr->heap->lj.errhandler;  /* use existing one (if any) */

	call_flags = DUK_CALL_FLAG_CONSTRUCTOR_CALL;  /* not protected, respect reclimit, is a constructor call */

	rc = duk_handle_call(thr,           /* thread */
	                     nargs,         /* num_stack_args */
	                     call_flags,    /* call_flags */
	                     errhandler);   /* errhandler */
	DUK_UNREF(rc);

	/* [... fallback retval] */

	DUK_DDDPRINT("constructor call finished, rc=%d, fallback=%!iT, retval=%!iT",
	             rc, duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

	/*
	 *  Determine whether to use the constructor return value as the created
	 *  object instance or not.
	 */

	if (duk_is_object(ctx, -1)) {
		duk_remove(ctx, -2);
	} else {
		duk_pop(ctx);
	}

	/* FIXME: error augmenatation here? */

	/* [... retval] */

	return;

 not_constructable:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not constructable");
}

int duk_is_constructor_call(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->callstack_top >= 0);

	if (thr->callstack_top <= 0) {
		return 0;
	}

	act = thr->callstack + thr->callstack_top - 1;
	return ((act->flags & DUK_ACT_FLAG_CONSTRUCT) != 0 ? 1 : 0);
}

int duk_is_strict_call(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->callstack_top >= 0);

	if (thr->callstack_top <= 0) {
		return 0;
	}

	act = thr->callstack + thr->callstack_top - 1;
	return ((act->flags & DUK_ACT_FLAG_STRICT) != 0 ? 1 : 0);
}

