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
 *  (FIXME: This should be probably reworked so that there is a shared
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
void duk_hthread_callstack_grow(duk_hthread *thr) {
	int old_size;
	int new_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->callstack_top >= 0 &&
	           thr->callstack_size >= thr->callstack_top);

	if (thr->callstack_top < thr->callstack_size) {
		return;
	}

	old_size = thr->callstack_size;
	new_size = old_size + DUK_CALLSTACK_GROW_STEP;

	/* this is a bit approximate (errors out before max is reached); this is OK */
	if (new_size >= thr->callstack_max) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "callstack limit reached");
	}

	DUK_DDPRINT("growing callstack %d -> %d", old_size, new_size);

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	thr->callstack = DUK_REALLOC_INDIRECT_CHECKED(thr, (void **) &thr->callstack, sizeof(duk_activation) * new_size);
	thr->callstack_size = new_size;

	/* note: any entries above the callstack top are garbage and not zeroed */
}

void duk_hthread_callstack_shrink_check(duk_hthread *thr) {
	int new_size;
	duk_activation *p;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->callstack_top >= 0 &&
	           thr->callstack_size >= thr->callstack_top);

	if (thr->callstack_size - thr->callstack_top < DUK_CALLSTACK_SHRINK_THRESHOLD) {
		return;
	}

	new_size = thr->callstack_top + DUK_CALLSTACK_SHRINK_SPARE;
	DUK_ASSERT(new_size >= thr->callstack_top);

	DUK_DDPRINT("shrinking callstack %d -> %d", thr->callstack_size, new_size);

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	/* shrink failure is not fatal */
	p = (duk_activation *) DUK_REALLOC_INDIRECT(thr->heap, (void **) &thr->callstack, sizeof(duk_activation) * new_size);
	if (p) {
		thr->callstack = p;
		thr->callstack_size = new_size;
	} else {
		DUK_DPRINT("callstack shrink failed, ignoring");
	}

	/* note: any entries above the callstack top are garbage and not zeroed */
}

void duk_hthread_callstack_unwind(duk_hthread *thr, int new_top) {
	int idx;

	DUK_ASSERT(thr);
	DUK_ASSERT(thr->heap);
	DUK_ASSERT(new_top >= 0);
	DUK_ASSERT(new_top <= thr->callstack_top);  /* cannot grow */

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
		duk_activation *p;
#ifdef DUK_USE_REFERENCE_COUNTING
		duk_hobject *tmp;
#endif

		idx--;
		DUK_ASSERT(idx >= 0 && idx < thr->callstack_size);  /* true, despite side effect resizes */

		p = &thr->callstack[idx];
		DUK_ASSERT(p->func != NULL);

		/*
		 *  Close environment record(s) if they exist.
		 *
		 *  Only variable environments are closed.  If lex_env != var_env, it
		 *  cannot currently contain any register bound declarations.
		 *
		 *  Only environments created for a NEWENV function are closed.  If an
		 *  environment is created for e.g. an eval call, it must not be closed.
		 */

		if (!DUK_HOBJECT_HAS_NEWENV(p->func)) {
			DUK_DDDPRINT("skip closing environments, envs not owned by this activation");
			goto skip_env_close;
		}

		if (p->var_env != NULL) {
			DUK_DDDPRINT("closing var_env record %p -> %!O",
			             (void *) p->var_env, (duk_heaphdr *) p->var_env);
			duk_js_close_environment_record(thr, p->var_env, p->func, p->idx_bottom);
			p = &thr->callstack[idx];  /* avoid side effect issues */
		}

#if 0
		if (p->lex_env != NULL) {
			if (p->lex_env == p->var_env) {
				/* common case, already closed, so skip */
				DUK_DDPRINT("lex_env and var_env are the same and lex_env "
				            "already closed -> skip closing lex_env");
				;
			} else {
				DUK_DDPRINT("closing lex_env record %p -> %!O",
				            (void *) p->lex_env, (duk_heaphdr *) p->lex_env);
				duk_js_close_environment_record(thr, p->lex_env, p->func, p->idx_bottom);
				p = &thr->callstack[idx];  /* avoid side effect issues */
			}
		}
#endif

		DUK_ASSERT((p->lex_env == NULL) ||
		           ((duk_hobject_find_existing_entry_tval_ptr(p->lex_env, DUK_HEAP_STRING_INT_CALLEE(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(p->lex_env, DUK_HEAP_STRING_INT_VARMAP(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(p->lex_env, DUK_HEAP_STRING_INT_THREAD(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(p->lex_env, DUK_HEAP_STRING_INT_REGBASE(thr)) == NULL)));

		DUK_ASSERT((p->var_env == NULL) ||
		           ((duk_hobject_find_existing_entry_tval_ptr(p->var_env, DUK_HEAP_STRING_INT_CALLEE(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(p->var_env, DUK_HEAP_STRING_INT_VARMAP(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(p->var_env, DUK_HEAP_STRING_INT_THREAD(thr)) == NULL) &&
		            (duk_hobject_find_existing_entry_tval_ptr(p->var_env, DUK_HEAP_STRING_INT_REGBASE(thr)) == NULL)));

	 skip_env_close:

		/*
		 *  Update preventcount
		 */

		if (p->flags & DUK_ACT_FLAG_PREVENT_YIELD) {
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
		tmp = p->var_env;
#endif
		p->var_env = NULL;
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_DECREF(thr, tmp);
		p = &thr->callstack[idx];  /* avoid side effect issues */
#endif

#ifdef DUK_USE_REFERENCE_COUNTING
		tmp = p->lex_env;
#endif
		p->lex_env = NULL;
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_DECREF(thr, tmp);
		p = &thr->callstack[idx];  /* avoid side effect issues */
#endif

		/* Note: this may cause a corner case situation where a finalizer
		 * may see a currently reachable activation whose 'func' is NULL.
		 */
#ifdef DUK_USE_REFERENCE_COUNTING
		tmp = p->func;
#endif
		p->func = NULL;
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_DECREF(thr, tmp);
		p = &thr->callstack[idx];  /* avoid side effect issues */
#endif
	}

	thr->callstack_top = new_top;

	/*
	 *  We could clear the book-keeping variables for the topmost activation,
	 *  but don't do so now.
	 */
#if 0
	if (thr->callstack_top > 0) {
		duk_activation *p = thr->callstack + thr->callstack_top - 1;
		p->idx_retval = -1;
	}
#endif

	/* Note: any entries above the callstack top are garbage and not zeroed.
	 * Also topmost activation idx_retval is garbage and not zeroed.
	 */
}

void duk_hthread_catchstack_grow(duk_hthread *thr) {
	int old_size;
	int new_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->catchstack_top >= 0 &&
	           thr->catchstack_size >= thr->catchstack_top);

	if (thr->catchstack_top < thr->catchstack_size) {
		return;
	}

	old_size = thr->catchstack_size;
	new_size = old_size + DUK_CATCHSTACK_GROW_STEP;

	/* this is a bit approximate (errors out before max is reached); this is OK */
	if (new_size >= thr->catchstack_max) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "catchstack limit reached");
	}

	DUK_DDPRINT("growing catchstack %d -> %d", old_size, new_size);

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	thr->catchstack = DUK_REALLOC_INDIRECT_CHECKED(thr, (void **) &thr->catchstack, sizeof(duk_catcher) * new_size);
	thr->catchstack_size = new_size;

	/* note: any entries above the catchstack top are garbage and not zeroed */
}

void duk_hthread_catchstack_shrink_check(duk_hthread *thr) {
	int new_size;
	duk_catcher *p;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->catchstack_top >= 0 &&
	           thr->catchstack_size >= thr->catchstack_top);

	if (thr->catchstack_size - thr->catchstack_top < DUK_CATCHSTACK_SHRINK_THRESHOLD) {
		return;
	}

	new_size = thr->catchstack_top + DUK_CATCHSTACK_SHRINK_SPARE;
	DUK_ASSERT(new_size >= thr->catchstack_top);

	DUK_DDPRINT("shrinking catchstack %d -> %d", thr->catchstack_size, new_size);

	/*
	 *  Note: must use indirect variant of DUK_REALLOC() because underlying
	 *  pointer may be changed by mark-and-sweep.
	 */

	/* shrink failure is not fatal */
	p = (duk_catcher *) DUK_REALLOC_INDIRECT(thr->heap, (void **) &thr->catchstack, sizeof(duk_catcher) * new_size);
	if (p) {
		thr->catchstack = p;
		thr->catchstack_size = new_size;
	} else {
		DUK_DPRINT("catchstack shrink failed, ignoring");
	}

	/* note: any entries above the catchstack top are garbage and not zeroed */
}

void duk_hthread_catchstack_unwind(duk_hthread *thr, int new_top) {
	int idx;

	DUK_ASSERT(thr);
	DUK_ASSERT(thr->heap);
	DUK_ASSERT(new_top >= 0);
	DUK_ASSERT(new_top <= thr->catchstack_top);  /* cannot grow */

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
		DUK_ASSERT(idx >= 0 && idx < thr->catchstack_size);

		p = &thr->catchstack[idx];

		if (DUK_CAT_HAS_LEXENV_ACTIVE(p)) {
			DUK_DDDPRINT("unwinding catchstack idx %d: lexical environment active", idx);

			/* FIXME: Here we have a nasty dependency: the need to manipulate
			 * the callstack means that catchstack must always be unwound by
			 * the caller before unwinding the callstack.  This should be fixed
			 * later.
			 */

			act = &thr->callstack[p->callstack_index];
			DUK_ASSERT(act >= thr->callstack);
			DUK_ASSERT(act < &thr->callstack[thr->callstack_top]);
			DUK_ASSERT(act->lex_env != NULL);  /* must be, since env was created */

			DUK_DDDPRINT("callstack_index=%d, lex_env=%!iO", p->callstack_index, act->lex_env);

			env = act->lex_env;
			act->lex_env = env->prototype;
			DUK_HOBJECT_DECREF(thr, env);
		}
	}

	thr->catchstack_top = new_top;

	/* note: any entries above the catchstack top are garbage and not zeroed */
}

