/*
 *  Manipulation of thread stacks (valstack, callstack).
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
 *  Valstack entries above 'top' are always kept initialized to "undefined".
 *  Callstack entries above 'top' are not zeroed and are left as garbage.
 *
 *  Value stack handling is mostly a part of the API implementation.
 */

#include "duk_internal.h"

DUK_LOCAL DUK_COLD DUK_NOINLINE void duk__hthread_do_callstack_grow(duk_hthread *thr) {
	duk_activation *new_ptr;
	duk_size_t old_size;
	duk_size_t new_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);   /* avoid warning (unsigned) */
	DUK_ASSERT(thr->callstack_size >= thr->callstack_top);

	old_size = thr->callstack_size;
	new_size = old_size + DUK_CALLSTACK_GROW_STEP;

	/* this is a bit approximate (errors out before max is reached); this is OK */
	if (new_size >= thr->callstack_max) {
		DUK_ERROR_RANGE(thr, DUK_STR_CALLSTACK_LIMIT);
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
		DUK_ERROR_ALLOC_FAILED(thr);
	}
	thr->callstack = new_ptr;
	thr->callstack_size = new_size;

	if (thr->callstack_top > 0) {
		thr->callstack_curr = thr->callstack + thr->callstack_top - 1;
	} else {
		thr->callstack_curr = NULL;
	}

	/* note: any entries above the callstack top are garbage and not zeroed */
}

/* check that there is space for at least one new entry */
DUK_INTERNAL void duk_hthread_callstack_grow(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);   /* avoid warning (unsigned) */
	DUK_ASSERT(thr->callstack_size >= thr->callstack_top);

	if (DUK_LIKELY(thr->callstack_top < thr->callstack_size)) {
		return;
	}
	duk__hthread_do_callstack_grow(thr);
}

DUK_LOCAL DUK_COLD DUK_NOINLINE void duk__hthread_do_callstack_shrink(duk_hthread *thr) {
	duk_size_t new_size;
	duk_activation *p;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);  /* avoid warning (unsigned) */
	DUK_ASSERT(thr->callstack_size >= thr->callstack_top);

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

		if (thr->callstack_top > 0) {
			thr->callstack_curr = thr->callstack + thr->callstack_top - 1;
		} else {
			thr->callstack_curr = NULL;
		}
	} else {
		/* Because new_size != 0, if condition doesn't need to be
		 * (p != NULL || new_size == 0).
		 */
		DUK_ASSERT(new_size != 0);
		DUK_D(DUK_DPRINT("callstack shrink failed, ignoring"));
	}

	/* note: any entries above the callstack top are garbage and not zeroed */
}

DUK_INTERNAL void duk_hthread_callstack_shrink_check(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);  /* avoid warning (unsigned) */
	DUK_ASSERT(thr->callstack_size >= thr->callstack_top);

	if (DUK_LIKELY(thr->callstack_size - thr->callstack_top < DUK_CALLSTACK_SHRINK_THRESHOLD)) {
		return;
	}

	duk__hthread_do_callstack_shrink(thr);
}

DUK_INTERNAL void duk_hthread_catcher_unwind_norz(duk_hthread *thr, duk_activation *act) {
	duk_catcher *cat;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(act != NULL);
	DUK_ASSERT(act->cat != NULL);  /* caller must check */
	cat = act->cat;
	DUK_ASSERT(cat != NULL);

	DUK_DDD(DUK_DDDPRINT("unwinding catch stack entry %p (lexenv check is done)", (void *) cat));

	if (DUK_CAT_HAS_LEXENV_ACTIVE(cat)) {
		duk_hobject *env;

		env = act->lex_env;             /* current lex_env of the activation (created for catcher) */
		DUK_ASSERT(env != NULL);        /* must be, since env was created when catcher was created */
		act->lex_env = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, env);  /* prototype is lex_env before catcher created */
		DUK_HOBJECT_INCREF(thr, act->lex_env);
		DUK_HOBJECT_DECREF_NORZ(thr, env);

		/* There is no need to decref anything else than 'env': if 'env'
		 * becomes unreachable, refzero will handle decref'ing its prototype.
		 */
	}

	act->cat = cat->parent;
	DUK_FREE_CHECKED(thr, (void *) cat);
}

DUK_INTERNAL void duk_hthread_catcher_unwind_nolexenv_norz(duk_hthread *thr, duk_activation *act) {
	duk_catcher *cat;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(act != NULL);
	DUK_ASSERT(act->cat != NULL);  /* caller must check */
	cat = act->cat;
	DUK_ASSERT(cat != NULL);

	DUK_DDD(DUK_DDDPRINT("unwinding catch stack entry %p (lexenv check is not done)", (void *) cat));

	DUK_ASSERT(!DUK_CAT_HAS_LEXENV_ACTIVE(cat));

	act->cat = cat->parent;
	DUK_FREE_CHECKED(thr, (void *) cat);
}

DUK_INTERNAL void duk_hthread_callstack_unwind_norz(duk_hthread *thr, duk_size_t new_top) {
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
		duk_hobject *tmp;
#if defined(DUK_USE_DEBUGGER_SUPPORT)
		duk_heap *heap;
#endif
		idx--;
		DUK_ASSERT_DISABLE(idx >= 0);  /* unsigned */
		DUK_ASSERT((duk_size_t) idx < thr->callstack_size);  /* true, despite side effect resizes */

		act = thr->callstack + idx;
		/* With lightfuncs, act 'func' may be NULL */

#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
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
				DUK_TVAL_DECREF_NORZ(thr, &tv_tmp);
			} else {
				h_tmp = act->prev_caller;
				if (h_tmp) {
					act->prev_caller = NULL;
					DUK_HOBJECT_DECREF_NORZ(thr, h_tmp);
				}
			}
			DUK_ASSERT(act == thr->callstack + idx);  /* no side effects */
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
			if (duk_debug_is_paused(heap)) {
				DUK_D(DUK_DPRINT("step pause trigger but already paused, ignoring"));
			} else {
				duk_debug_set_paused(heap);
				DUK_ASSERT(heap->dbg_step_thread == NULL);
			}
		}
#endif

		/*
		 *  Unwind catchers.
		 *
		 *  Since there are no references in the catcher structure,
		 *  unwinding is quite simple.  The only thing we need to
		 *  look out for is popping a possible lexical environment
		 *  established for an active catch clause.
		 */

		DUK_ASSERT(act == thr->callstack + idx);  /* no side effects */
		while (act->cat != NULL) {
			DUK_ASSERT(act == thr->callstack + idx);  /* no side effects */
			duk_hthread_catcher_unwind_norz(thr, act);
		}

		/*
		 *  Close environment record(s) if they exist.
		 *
		 *  Only variable environments are closed.  If lex_env != var_env, it
		 *  cannot currently contain any register bound declarations.
		 *
		 *  Only environments created for a NEWENV function are closed.  If an
		 *  environment is created for e.g. an eval call, it must not be closed.
		 */

		DUK_ASSERT(act == thr->callstack + idx);  /* no side effects */
		func = DUK_ACT_GET_FUNC(act);
		if (func != NULL && !DUK_HOBJECT_HAS_NEWENV(func)) {
			DUK_DDD(DUK_DDDPRINT("skip closing environments, envs not owned by this activation"));
			goto skip_env_close;
		}
		/* func is NULL for lightfunc */

		/* Catch sites are required to clean up their environments
		 * in FINALLY part before propagating, so this should
		 * always hold here.
		 */
		DUK_ASSERT(act->lex_env == act->var_env);

		DUK_ASSERT(act == thr->callstack + idx);  /* no side effects */
		if (act->var_env != NULL) {
			DUK_DDD(DUK_DDDPRINT("closing var_env record %p -> %!O",
			                     (void *) act->var_env, (duk_heaphdr *) act->var_env));
			duk_js_close_environment_record(thr, act->var_env);
			act = thr->callstack + idx;  /* avoid side effect issues */
		}

	 skip_env_close:

		/*
		 *  Update preventcount
		 */

		DUK_ASSERT(act == thr->callstack + idx);  /* no side effects */
		if (act->flags & DUK_ACT_FLAG_PREVENT_YIELD) {
			DUK_ASSERT(thr->callstack_preventcount >= 1);
			thr->callstack_preventcount--;
		}

		/*
		 *  Reference count updates, using NORZ macros so we don't
		 *  need to handle side effects.
		 */

		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, act->var_env);
		act->var_env = NULL;
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, act->lex_env);
		act->lex_env = NULL;

		/* Note: this may cause a corner case situation where a finalizer
		 * may see a currently reachable activation whose 'func' is NULL.
		 */
		tmp = DUK_ACT_GET_FUNC(act);
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, tmp);
		DUK_UNREF(tmp);
		act->func = NULL;
	}

	thr->callstack_top = new_top;
	if (new_top > 0) {
		thr->callstack_curr = thr->callstack + new_top - 1;
	} else {
		thr->callstack_curr = NULL;
	}

	/* We could clear the book-keeping variables for the topmost activation,
	 * but don't do so now.
	 */
#if 0
	if (thr->callstack_curr != NULL) {
		duk_activation *act = thr->callstack_curr;
		act->idx_retval = 0;
	}
#endif

	/* Note: any entries above the callstack top are garbage and not zeroed.
	 * Also topmost activation idx_retval is garbage (not zeroed), and must
	 * be ignored.
	 */
}

DUK_INTERNAL void duk_hthread_callstack_unwind(duk_hthread *thr, duk_size_t new_top) {
	duk_hthread_callstack_unwind_norz(thr, new_top);
	DUK_REFZERO_CHECK_FAST(thr);
}

#if defined(DUK_USE_FINALIZER_TORTURE)
DUK_INTERNAL void duk_hthread_valstack_torture_realloc(duk_hthread *thr) {
	duk_size_t alloc_size;
	duk_tval *new_ptr;
	duk_ptrdiff_t end_off;
	duk_ptrdiff_t bottom_off;
	duk_ptrdiff_t top_off;

	if (thr->valstack == NULL) {
		return;
	}

	end_off = (duk_ptrdiff_t) ((duk_uint8_t *) thr->valstack_end - (duk_uint8_t *) thr->valstack);
	bottom_off = (duk_ptrdiff_t) ((duk_uint8_t *) thr->valstack_bottom - (duk_uint8_t *) thr->valstack);
	top_off = (duk_ptrdiff_t) ((duk_uint8_t *) thr->valstack_top - (duk_uint8_t *) thr->valstack);
	alloc_size = (duk_size_t) end_off;
	if (alloc_size == 0) {
		return;
	}

	new_ptr = (duk_tval *) DUK_ALLOC(thr->heap, alloc_size);
	if (new_ptr != NULL) {
		DUK_MEMCPY((void *) new_ptr, (const void *) thr->valstack, alloc_size);
		DUK_MEMSET((void *) thr->valstack, 0x55, alloc_size);
		DUK_FREE_CHECKED(thr, (void *) thr->valstack);
		thr->valstack = new_ptr;
		thr->valstack_end = (duk_tval *) ((duk_uint8_t *) new_ptr + end_off);
		thr->valstack_bottom = (duk_tval *) ((duk_uint8_t *) new_ptr + bottom_off);
		thr->valstack_top = (duk_tval *) ((duk_uint8_t *) new_ptr + top_off);
		/* No change in size. */
	} else {
		DUK_D(DUK_DPRINT("failed to realloc valstack for torture, ignore"));
	}
}

DUK_INTERNAL void duk_hthread_callstack_torture_realloc(duk_hthread *thr) {
	duk_size_t alloc_size;
	duk_activation *new_ptr;
	duk_ptrdiff_t curr_off;

	if (thr->callstack == NULL) {
		return;
	}

	curr_off = (duk_ptrdiff_t) ((duk_uint8_t *) thr->callstack_curr - (duk_uint8_t *) thr->callstack);
	alloc_size = sizeof(duk_activation) * thr->callstack_size;
	if (alloc_size == 0) {
		return;
	}

	new_ptr = (duk_activation *) DUK_ALLOC(thr->heap, alloc_size);
	if (new_ptr != NULL) {
		DUK_MEMCPY((void *) new_ptr, (const void *) thr->callstack, alloc_size);
		DUK_MEMSET((void *) thr->callstack, 0x55, alloc_size);
		DUK_FREE_CHECKED(thr, (void *) thr->callstack);
		thr->callstack = new_ptr;
		thr->callstack_curr = (duk_activation *) ((duk_uint8_t *) new_ptr + curr_off);
		/* No change in size. */
	} else {
		DUK_D(DUK_DPRINT("failed to realloc callstack for torture, ignore"));
	}
}
#endif  /* DUK_USE_FINALIZER_TORTURE */
