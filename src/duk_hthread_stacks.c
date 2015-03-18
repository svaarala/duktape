/*
 *  Manipulation of thread stacks (valstack, callstack, catchstack).
 *
 *  Ideally unwinding of stacks should have no side effects, which would
 *  then favor separate unwinding and shrink check primitives for each
 *  stack type.  A shrink check may realloc and thus have side effects.
 *
 *  However, currently callstack unwinding itself has side effects, as it
 *  needs to DECREF multiple objects, close environment records, etc.
 *  Stacks must thus be unwound in the correct order by the caller.
 *
 *  (XXX: This should be probably reworked so that there is a shared
 *  unwind primitive which handles all stacks as requested, and knows
 *  the proper order for unwinding.)
 *
 *  Valstack entries above 'top' are always kept initialized to
 *  "undefined unused".  Callstack and catchstack entries above 'top'
 *  are not zeroed and are left as garbage.
 *
 *  Value stack handling is mostly a part of the API implementation.
 */

#include "duk_internal.h"

/* check that there is space for at least one new entry */
DUK_INTERNAL void duk_hthread_callstack_grow(duk_hthread *thr) {
	duk_activation *new_ptr;
	duk_size_t old_size;
	duk_size_t new_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);   /* avoid warning (unsigned) */
	DUK_ASSERT(thr->callstack_size >= thr->callstack_top);

	if (thr->callstack_top < thr->callstack_size) {
		return;
	}

	old_size = thr->callstack_size;
	new_size = old_size + DUK_CALLSTACK_GROW_STEP;

	/* this is a bit approximate (errors out before max is reached); this is OK */
	if (new_size >= thr->callstack_max) {
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_CALLSTACK_LIMIT);
	}

	DUK_DD(DUK_DDPRINT("growing callstack %ld -> %ld", (long) old_size, (long) new_size));

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	DUK_ASSERT(new_size > 0);
	new_ptr = (duk_activation *) DUK_REALLOC_INDIRECT(thr->heap, duk_hthread_get_callstack_ptr, (void *) thr, sizeof(duk_activation) * new_size);
	if (!new_ptr) {
		/* No need for a NULL/zero-size check because new_size > 0) */
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_REALLOC_FAILED);
	}
	thr->callstack = new_ptr;
	thr->callstack_size = new_size;

	/* note: any entries above the callstack top are garbage and not zeroed */
}

DUK_INTERNAL void duk_hthread_callstack_shrink_check(duk_hthread *thr) {
	duk_size_t new_size;
	duk_activation *p;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);  /* avoid warning (unsigned) */
	DUK_ASSERT(thr->callstack_size >= thr->callstack_top);

	if (thr->callstack_size - thr->callstack_top < DUK_CALLSTACK_SHRINK_THRESHOLD) {
		return;
	}

	new_size = thr->callstack_top + DUK_CALLSTACK_SHRINK_SPARE;
	DUK_ASSERT(new_size >= thr->callstack_top);

	DUK_DD(DUK_DDPRINT("shrinking callstack %ld -> %ld", (long) thr->callstack_size, (long) new_size));

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	/* shrink failure is not fatal */
	p = (duk_activation *) DUK_REALLOC_INDIRECT(thr->heap, duk_hthread_get_callstack_ptr, (void *) thr, sizeof(duk_activation) * new_size);
	if (p) {
		thr->callstack = p;
		thr->callstack_size = new_size;
	} else {
		/* Because new_size != 0, if condition doesn't need to be
		 * (p != NULL || new_size == 0).
		 */
		DUK_ASSERT(new_size != 0);
		DUK_D(DUK_DPRINT("callstack shrink failed, ignoring"));
	}

	/* note: any entries above the callstack top are garbage and not zeroed */
}

DUK_INTERNAL void duk_hthread_callstack_unwind(duk_hthread *thr, duk_size_t new_top) {
	duk_size_t idx;

	DUK_DDD(DUK_DDDPRINT("unwind callstack top of thread %p from %ld to %ld",
	                     (void *) thr,
	                     (thr != NULL ? (long) thr->callstack_top : (long) -1),
	                     (long) new_top));

	DUK_ASSERT(thr);
	DUK_ASSERT(thr->heap);
	DUK_ASSERT_DISABLE(new_top >= 0);  /* unsigned */
	DUK_ASSERT((duk_size_t) new_top <= thr->callstack_top);  /* cannot grow */

	/*
	 *  The loop below must avoid issues with potential callstack
	 *  reallocations.  A resize (and other side effects) may happen
	 *  e.g. due to finalizer/errhandler calls caused by a refzero or
	 *  mark-and-sweep.  Arbitrary finalizers may run, because when
	 *  an environment record is refzero'd, it may refer to arbitrary
	 *  values which also become refzero'd.
	 *
	 *  So, the pointer 'p' is re-looked-up below whenever a side effect
	 *  might have changed it.
	 */

	idx = thr->callstack_top;
	while (idx > new_top) {
		duk_activation *act;
		duk_hobject *func;
#ifdef DUK_USE_REFERENCE_COUNTING
		duk_hobject *tmp;
#endif
#ifdef DUK_USE_DEBUGGER_SUPPORT
		duk_heap *heap;
#endif

		idx--;
		DUK_ASSERT_DISABLE(idx >= 0);  /* unsigned */
		DUK_ASSERT((duk_size_t) idx < thr->callstack_size);  /* true, despite side effect resizes */

		act = thr->callstack + idx;
		/* With lightfuncs, act 'func' may be NULL */

#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
		/*
		 *  Restore 'caller' property for non-strict callee functions.
		 */

		func = DUK_ACT_GET_FUNC(act);
		if (func != NULL && !DUK_HOBJECT_HAS_STRICT(func)) {
			duk_tval *tv_caller;
			duk_tval tv_tmp;
			duk_hobject *h_tmp;

			tv_caller = duk_hobject_find_existing_entry_tval_ptr(thr->heap, func, DUK_HTHREAD_STRING_CALLER(thr));

			/* The act->prev_caller should only be set if the entry for 'caller'
			 * exists (as it is only set in that case, and the property is not
			 * configurable), but handle all the cases anyway.
			 */

			if (tv_caller) {
				DUK_TVAL_SET_TVAL(&tv_tmp, tv_caller);
				if (act->prev_caller) {
					/* Just transfer the refcount from act->prev_caller to tv_caller,
					 * so no need for a refcount update.  This is the expected case.
					 */
					DUK_TVAL_SET_OBJECT(tv_caller, act->prev_caller);
					act->prev_caller = NULL;
				} else {
					DUK_TVAL_SET_NULL(tv_caller);   /* no incref needed */
					DUK_ASSERT(act->prev_caller == NULL);
				}
				DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
			} else {
				h_tmp = act->prev_caller;
				if (h_tmp) {
					act->prev_caller = NULL;
					DUK_HOBJECT_DECREF(thr, h_tmp);  /* side effects */
				}
			}
			act = thr->callstack + idx;  /* avoid side effects */
			DUK_ASSERT(act->prev_caller == NULL);
		}
#endif

		/*
		 *  Unwind debugger state.  If we unwind while stepping
		 *  (either step over or step into), pause execution.
		 */

#if defined(DUK_USE_DEBUGGER_SUPPORT)
		heap = thr->heap;
		if (heap->dbg_step_thread == thr &&
		    heap->dbg_step_csindex == idx) {
			/* Pause for all step types: step into, step over, step out.
			 * This is the only place explicitly handling a step out.
			 */
			DUK_HEAP_SET_PAUSED(heap);
			DUK_ASSERT(heap->dbg_step_thread == NULL);
		}
#endif

		/*
		 *  Close environment record(s) if they exist.
		 *
		 *  Only variable environments are closed.  If lex_env != var_env, it
		 *  cannot currently contain any register bound declarations.
		 *
		 *  Only environments created for a NEWENV function are closed.  If an
		 *  environment is created for e.g. an eval call, it must not be closed.
		 */

		func = DUK_ACT_GET_FUNC(act);
		if (func != NULL && !DUK_HOBJECT_HAS_NEWENV(func)) {
			DUK_DDD(DUK_DDDPRINT("skip closing environments, envs not owned by this activation"));
			goto skip_env_close;
		}
		/* func is NULL for lightfunc */

		DUK_ASSERT(act->lex_env == act->var_env);
		if (act->var_env != NULL) {
			DUK_DDD(DUK_DDDPRINT("closing var_env record %p -> %!O",
			                     (void *) act->var_env, (duk_heaphdr *) act->var_env));
			duk_js_close_environment_record(thr, act->var_env, func, act->idx_bottom);
			act = thr->callstack + idx;  /* avoid side effect issues */
		}

#if 0
		if (act->lex_env != NULL) {
			if (act->lex_env == act->var_env) {
				/* common case, already closed, so skip */
				DUK_DD(DUK_DDPRINT("lex_env and var_env are the same and lex_env "
				                   "already closed -> skip closing lex_env"));
				;
			} else {
				DUK_DD(DUK_DDPRINT("closing lex_env record %p -> %!O",
				                   (void *) act->lex_env, (duk_heaphdr *) act->lex_env));
				duk_js_close_environment_record(thr, act->lex_env, DUK_ACT_GET_FUNC(act), act->idx_bottom);
				act = thr->callstack + idx;  /* avoid side effect issues */
			}
		}
#endif

		DUK_ASSERT((act->lex_env == NULL) ||
		           ((duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->lex_env, DUK_HTHREAD_STRING_INT_CALLEE(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->lex_env, DUK_HTHREAD_STRING_INT_VARMAP(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->lex_env, DUK_HTHREAD_STRING_INT_THREAD(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->lex_env, DUK_HTHREAD_STRING_INT_REGBASE(thr)) == NULL)));

		DUK_ASSERT((act->var_env == NULL) ||
		           ((duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->var_env, DUK_HTHREAD_STRING_INT_CALLEE(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->var_env, DUK_HTHREAD_STRING_INT_VARMAP(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->var_env, DUK_HTHREAD_STRING_INT_THREAD(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(thr->heap, act->var_env, DUK_HTHREAD_STRING_INT_REGBASE(thr)) == NULL)));

	 skip_env_close:

		/*
		 *  Update preventcount
		 */

		if (act->flags & DUK_ACT_FLAG_PREVENT_YIELD) {
			DUK_ASSERT(thr->callstack_preventcount >= 1);
			thr->callstack_preventcount--;
		}

		/*
		 *  Reference count updates
		 *
		 *  Note: careful manipulation of refcounts.  The top is
		 *  not updated yet, so all the activations are reachable
		 *  for mark-and-sweep (which may be triggered by decref).
		 *  However, the pointers are NULL so this is not an issue.
		 */

#ifdef DUK_USE_REFERENCE_COUNTING
		tmp = act->var_env;
#endif
		act->var_env = NULL;
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_DECREF_ALLOWNULL(thr, tmp);
		act = thr->callstack + idx;  /* avoid side effect issues */
#endif

#ifdef DUK_USE_REFERENCE_COUNTING
		tmp = act->lex_env;
#endif
		act->lex_env = NULL;
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_DECREF_ALLOWNULL(thr, tmp);
		act = thr->callstack + idx;  /* avoid side effect issues */
#endif

		/* Note: this may cause a corner case situation where a finalizer
		 * may see a currently reachable activation whose 'func' is NULL.
		 */
#ifdef DUK_USE_REFERENCE_COUNTING
		tmp = DUK_ACT_GET_FUNC(act);
#endif
		act->func = NULL;
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_DECREF_ALLOWNULL(thr, tmp);
		act = thr->callstack + idx;  /* avoid side effect issues */
		DUK_UNREF(act);
#endif
	}

	thr->callstack_top = new_top;

	/*
	 *  We could clear the book-keeping variables for the topmost activation,
	 *  but don't do so now.
	 */
#if 0
	if (thr->callstack_top > 0) {
		duk_activation *act = thr->callstack + thr->callstack_top - 1;
		act->idx_retval = 0;
	}
#endif

	/* Note: any entries above the callstack top are garbage and not zeroed.
	 * Also topmost activation idx_retval is garbage (not zeroed), and must
	 * be ignored.
	 */
}

DUK_INTERNAL void duk_hthread_catchstack_grow(duk_hthread *thr) {
	duk_catcher *new_ptr;
	duk_size_t old_size;
	duk_size_t new_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->catchstack_top);  /* avoid warning (unsigned) */
	DUK_ASSERT(thr->catchstack_size >= thr->catchstack_top);

	if (thr->catchstack_top < thr->catchstack_size) {
		return;
	}

	old_size = thr->catchstack_size;
	new_size = old_size + DUK_CATCHSTACK_GROW_STEP;

	/* this is a bit approximate (errors out before max is reached); this is OK */
	if (new_size >= thr->catchstack_max) {
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_CATCHSTACK_LIMIT);
	}

	DUK_DD(DUK_DDPRINT("growing catchstack %ld -> %ld", (long) old_size, (long) new_size));

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	DUK_ASSERT(new_size > 0);
	new_ptr = (duk_catcher *) DUK_REALLOC_INDIRECT(thr->heap, duk_hthread_get_catchstack_ptr, (void *) thr, sizeof(duk_catcher) * new_size);
	if (!new_ptr) {
		/* No need for a NULL/zero-size check because new_size > 0) */
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_REALLOC_FAILED);
	}
	thr->catchstack = new_ptr;
	thr->catchstack_size = new_size;

	/* note: any entries above the catchstack top are garbage and not zeroed */
}

DUK_INTERNAL void duk_hthread_catchstack_shrink_check(duk_hthread *thr) {
	duk_size_t new_size;
	duk_catcher *p;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->catchstack_top >= 0);  /* avoid warning (unsigned) */
	DUK_ASSERT(thr->catchstack_size >= thr->catchstack_top);

	if (thr->catchstack_size - thr->catchstack_top < DUK_CATCHSTACK_SHRINK_THRESHOLD) {
		return;
	}

	new_size = thr->catchstack_top + DUK_CATCHSTACK_SHRINK_SPARE;
	DUK_ASSERT(new_size >= thr->catchstack_top);

	DUK_DD(DUK_DDPRINT("shrinking catchstack %ld -> %ld", (long) thr->catchstack_size, (long) new_size));

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	/* shrink failure is not fatal */
	p = (duk_catcher *) DUK_REALLOC_INDIRECT(thr->heap, duk_hthread_get_catchstack_ptr, (void *) thr, sizeof(duk_catcher) * new_size);
	if (p) {
		thr->catchstack = p;
		thr->catchstack_size = new_size;
	} else {
		/* Because new_size != 0, if condition doesn't need to be
		 * (p != NULL || new_size == 0).
		 */
		DUK_ASSERT(new_size != 0);
		DUK_D(DUK_DPRINT("catchstack shrink failed, ignoring"));
	}

	/* note: any entries above the catchstack top are garbage and not zeroed */
}

DUK_INTERNAL void duk_hthread_catchstack_unwind(duk_hthread *thr, duk_size_t new_top) {
	duk_size_t idx;

	DUK_DDD(DUK_DDDPRINT("unwind catchstack top of thread %p from %ld to %ld",
	                     (void *) thr,
	                     (thr != NULL ? (long) thr->catchstack_top : (long) -1),
	                     (long) new_top));

	DUK_ASSERT(thr);
	DUK_ASSERT(thr->heap);
	DUK_ASSERT_DISABLE(new_top >= 0);  /* unsigned */
	DUK_ASSERT((duk_size_t) new_top <= thr->catchstack_top);  /* cannot grow */

	/*
	 *  Since there are no references in the catcher structure,
	 *  unwinding is quite simple.  The only thing we need to
	 *  look out for is popping a possible lexical environment
	 *  established for an active catch clause.
	 */

	idx = thr->catchstack_top;
	while (idx > new_top) {
		duk_catcher *p;
		duk_activation *act;
		duk_hobject *env;

		idx--;
		DUK_ASSERT_DISABLE(idx >= 0);  /* unsigned */
		DUK_ASSERT((duk_size_t) idx < thr->catchstack_size);

		p = thr->catchstack + idx;

		if (DUK_CAT_HAS_LEXENV_ACTIVE(p)) {
			DUK_DDD(DUK_DDDPRINT("unwinding catchstack idx %ld, callstack idx %ld, callstack top %ld: lexical environment active",
			                     (long) idx, (long) p->callstack_index, (long) thr->callstack_top));

			/* XXX: Here we have a nasty dependency: the need to manipulate
			 * the callstack means that catchstack must always be unwound by
			 * the caller before unwinding the callstack.  This should be fixed
			 * later.
			 */

			/* Note that multiple catchstack entries may refer to the same
			 * callstack entry.
			 */
			act = thr->callstack + p->callstack_index;
			DUK_ASSERT(act >= thr->callstack);
			DUK_ASSERT(act < thr->callstack + thr->callstack_top);

			DUK_DDD(DUK_DDDPRINT("catchstack_index=%ld, callstack_index=%ld, lex_env=%!iO",
			                     (long) idx, (long) p->callstack_index,
			                     (duk_heaphdr *) act->lex_env));

			env = act->lex_env;             /* current lex_env of the activation (created for catcher) */
			DUK_ASSERT(env != NULL);        /* must be, since env was created when catcher was created */
			act->lex_env = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, env);  /* prototype is lex_env before catcher created */
			DUK_HOBJECT_DECREF(thr, env);

			/* There is no need to decref anything else than 'env': if 'env'
			 * becomes unreachable, refzero will handle decref'ing its prototype.
			 */
		}
	}

	thr->catchstack_top = new_top;

	/* note: any entries above the catchstack top are garbage and not zeroed */
}
