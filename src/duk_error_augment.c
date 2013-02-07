/*
 *  Augment an error object with custom fields like raw traceback data.
 *  An error is augmented when it is created, not when it is thrown
 *  (otherwise rethrowing would work poorly).
 *
 *  May throw an error (e.g. alloc error).
 *
 *  Ecmascript allows throwing any values, so all values cannot be
 *  augmented.  Currently, we only augment error values which are Error
 *  instances and are also extensible.
 */

#include "duk_internal.h"

#ifdef DUK_USE_AUGMENT_ERRORS

#ifdef DUK_USE_TRACEBACKS
static void add_traceback(duk_hthread *thr, duk_hthread *thr_callstack, duk_hobject *obj, int err_index) {
	duk_context *ctx = (duk_context *) thr;
	int depth;
	int i, i_min;
	int arr_idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr_callstack != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(err_index >= 0);
	DUK_ASSERT(ctx != NULL);

	/*
	 *  The traceback format is pretty arcane in an attempt to keep it compact
	 *  and cheap to create.  It may change arbitrarily from version to version.
	 *  It should be decoded/accessed through version specific accessors.
	 *
	 *  See doc/error-objects.txt.
	 */

	DUK_DDDPRINT("adding traceback to object: %!O", (duk_heaphdr *) obj);

	duk_push_new_array(ctx);  /* XXX: specify array size, as we know it */

	depth = DUK_OPT_TRACEBACK_DEPTH;
	i_min = (thr_callstack->callstack_top > depth ? thr_callstack->callstack_top - depth : 0);
	DUK_ASSERT(i_min >= 0);
	arr_idx = 0;

	for (i = thr_callstack->callstack_top - 1; i >= i_min; i--) {
		double d;

		/*
		 *  Note: each API operation potentially resizes the callstack,
		 *  so be careful to re-lookup after every operation.  Currently
		 *  these is no issue because we don't store a temporary 'act'
		 *  pointer at all.
		 */

		/* [... arr] */

		DUK_ASSERT(thr_callstack->callstack[i].func != NULL);
		DUK_ASSERT(thr_callstack->callstack[i].pc >= 0);

		/* add function */
		duk_push_hobject(ctx, thr_callstack->callstack[i].func);  /* -> [... arr func] */
		duk_put_prop_index(ctx, -2, arr_idx);
		arr_idx++;

		/* add a number containing: pc, activation flags */
		d = ((double) thr_callstack->callstack[i].flags) * DUK_DOUBLE_2TO32 +  /* assume PC is at most 32 bits and non-negative */
		    (double) thr_callstack->callstack[i].pc;
		duk_push_number(ctx, d);  /* -> [... arr num] */
		duk_put_prop_index(ctx, -2, arr_idx);
		arr_idx++;

		/* FIXME: some more features to record (somehow):
		 *   - current this binding?
		 */

		/* FIXME: efficient array pushing, e.g. preallocate array, write DIRECTLY to array entries, etc. */
	}

	/* [... arr] */
	duk_put_prop_stridx(ctx, err_index, DUK_HEAP_STRIDX_TRACEBACK);  /* -> [...] */
}
#endif  /* DUK_USE_TRACEBACKS */

/*
 *  thr: thread containing the error value
 *  thr_callstack: thread which should be used for generating callstack etc.
 */

void duk_err_augment_error(duk_hthread *thr, duk_hthread *thr_callstack, int err_index) {
	duk_context *ctx = (duk_context *) thr;
	duk_hobject *obj;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr_callstack != NULL);
	DUK_ASSERT(ctx != NULL);

	err_index = duk_require_normalize_index(ctx, err_index);

	/*
	 *  Criteria for augmenting:
	 *
	 *   - augmentation enabled in build (naturally)
	 *   - error value is an extensible object
	 *   - error value internal prototype chain contains the built-in
	 *     Error prototype object (i.e. 'val instanceof Error')
	 */

	obj = duk_require_hobject(ctx, err_index);
	if (!obj) {	
		DUK_DDDPRINT("error value not an object, not augmented");
		return;
	}
	if (!DUK_HOBJECT_HAS_EXTENSIBLE(obj)) {
		DUK_DDDPRINT("error value not extensible, not augmented");
		return;
	}
	if (!duk_hobject_prototype_chain_contains(thr, obj, thr->builtins[DUK_BIDX_ERROR_PROTOTYPE])) {
		DUK_DDDPRINT("error value not inherited from Error, not augmented");
		return;
	}

	/* Yes, augment error. */

	/* FIXME: here we'd like to have a variant of "duk_def_prop_stridx" which
	 * would refuse to add a property if it already exists to avoid any issues
	 * with protected properties.
	 */

#ifdef DUK_USE_TRACEBACKS
	if (duk_hobject_hasprop_raw(thr, obj, DUK_HTHREAD_STRING_TRACEBACK(thr))) {
		DUK_DDDPRINT("error value already has a 'traceback' property, not modifying it");
	} else {
		add_traceback(thr, thr_callstack, obj, err_index);
	}
#endif  /* DUK_USE_TRACEBACKS */

	if (thr_callstack->callstack_top > 0) {
		duk_activation *act;
		duk_hobject *func;
		duk_hbuffer *pc2line;

		act = thr_callstack->callstack + thr_callstack->callstack_top - 1;
		func = act->func;
		if (func) {
			int pc = act->pc;
			duk_u32 line;
			act = NULL;  /* invalidated by pushes */

			pc--;  /* PC points to next instruction, find offending PC */

			duk_push_hobject(ctx, func);

			/* FIXME: now set function name as filename; record function and file name
			 * separately?  Function object is already in traceback though.
			 */
			duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_NAME);
			duk_put_prop_stridx(ctx, err_index, DUK_HEAP_STRIDX_FILE_NAME);

			if (DUK_HOBJECT_IS_COMPILEDFUNCTION(func)) {
				duk_push_false(ctx);
				duk_put_prop_stridx(ctx, err_index, DUK_HEAP_STRIDX_IS_NATIVE);

				/* FIXME: add PC only if pc2line fails? */
				duk_push_number(ctx, pc);
				duk_put_prop_stridx(ctx, err_index, DUK_HEAP_STRIDX_PC);

				duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_INT_PC2LINE);
				if (duk_is_buffer(ctx, -1)) {
					pc2line = duk_get_hbuffer(ctx, -1);
					DUK_ASSERT(!DUK_HBUFFER_HAS_GROWABLE(pc2line));
					line = duk_hobject_pc2line_query((duk_hbuffer_fixed *) pc2line, pc);
					duk_push_number(ctx, (double) line);  /* FIXME: u32 */
					duk_put_prop_stridx(ctx, err_index, DUK_HEAP_STRIDX_LINE_NUMBER);
				}
				duk_pop(ctx);
			} else {
				duk_push_true(ctx);
				duk_put_prop_stridx(ctx, err_index, DUK_HEAP_STRIDX_IS_NATIVE);
			}

			duk_pop(ctx);
		}
	}
}

#endif  /* DUK_USE_AUGMENT_ERRORS */

