/*
 *  Augment an error object with custom fields like raw traceback data.
 *  An error is augmented when it is created, not when it is thrown
 *  (otherwise rethrowing would work poorly).
 *
 *  May throw an error (e.g. alloc error).
 *
 *  Ecmascript allows throwing any values, so all values cannot be
 *  augmented.  Currently, we only augment error values which are Error
 *  instances (= have the built-in Error.prototype in their prototype
 *  chain) and are also extensible.
 */

#include "duk_internal.h"

#ifdef DUK_USE_AUGMENT_ERRORS

#ifdef DUK_USE_TRACEBACKS
static void add_traceback(duk_hthread *thr, duk_hthread *thr_callstack, duk_hobject *obj, int err_index, const char *filename, int line) {
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
	 *  It should be decoded/accessed through version specific accessors only.
	 *
	 *  See doc/error-objects.txt.
	 */

	DUK_DDDPRINT("adding traceback to object: %!O", (duk_heaphdr *) obj);

	duk_push_array(ctx);  /* XXX: specify array size, as we know it */
	arr_idx = 0;

	/* filename/line from C macros (__FILE__, __LINE__) are added as an
	 * entry with a special format: (string, number)
	 */
	if (filename) {
		duk_push_string(ctx, filename);
		duk_put_prop_index(ctx, -2, arr_idx);
		arr_idx++;
		duk_push_int(ctx, line);
		duk_put_prop_index(ctx, -2, arr_idx);
		arr_idx++;
	}

	/* traceback depth doesn't take into account the filename/line
	 * special handling above (intentional)
	 */
	depth = DUK_OPT_TRACEBACK_DEPTH;
	i_min = (thr_callstack->callstack_top > depth ? thr_callstack->callstack_top - depth : 0);
	DUK_ASSERT(i_min >= 0);

	for (i = thr_callstack->callstack_top - 1; i >= i_min; i--) {
		double d;
		int pc;

		/*
		 *  Note: each API operation potentially resizes the callstack,
		 *  so be careful to re-lookup after every operation.  Currently
		 *  these is no issue because we don't store a temporary 'act'
		 *  pointer at all.  (This would be a non-issue if we operated
		 *  directly on the array part.)
		 */

		/* [... arr] */

		DUK_ASSERT(thr_callstack->callstack[i].func != NULL);
		DUK_ASSERT(thr_callstack->callstack[i].pc >= 0);

		/* add function */
		duk_push_hobject(ctx, thr_callstack->callstack[i].func);  /* -> [... arr func] */
		duk_put_prop_index(ctx, -2, arr_idx);
		arr_idx++;

		/* add a number containing: pc, activation flags */
		pc = thr_callstack->callstack[i].pc;
		pc--;  /* PC points to next instruction, find offending PC; note that
		        * PC == 0 should never be possible for an error.
		        */
		DUK_ASSERT(pc >= 0 && (double) pc < DUK_DOUBLE_2TO32);  /* assume PC is at most 32 bits and non-negative */
		d = ((double) thr_callstack->callstack[i].flags) * DUK_DOUBLE_2TO32 + (double) pc;
		duk_push_number(ctx, d);  /* -> [... arr num] */
		duk_put_prop_index(ctx, -2, arr_idx);
		arr_idx++;
	}

	/* [... arr] */
	duk_put_prop_stridx(ctx, err_index, DUK_STRIDX_TRACEDATA);  /* -> [...] */
}
#endif  /* DUK_USE_TRACEBACKS */

/*
 *  Augment an error with tracedata, fileName, lineNumber, etc.
 *
 *  thr: thread containing the error value
 *  thr_callstack: thread which should be used for generating callstack etc.
 */

void duk_err_augment_error(duk_hthread *thr, duk_hthread *thr_callstack, int err_index, const char *filename, int line) {
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

#ifdef DUK_USE_TRACEBACKS
	/*
	 *  If tracebacks are enabled, the 'tracedata' property is the only
	 *  thing we need: 'fileName' and 'lineNumber' are virtual properties
	 *  which use 'tracedata'.
	 */

	if (duk_hobject_hasprop_raw(thr, obj, DUK_HTHREAD_STRING_TRACEDATA(thr))) {
		DUK_DDDPRINT("error value already has a 'traceback' property, not modifying it");
	} else {
		add_traceback(thr, thr_callstack, obj, err_index, filename, line);
	}
#else
	/*
	 *  If tracebacks are disabled, 'fileName' and 'lineNumber' are added
	 *  as plain own properties.  Since Error.prototype has accessors of
	 *  the same name, we need to define own properties directly (cannot
	 *  just use e.g. duk_put_prop_stridx).  Existing properties are not
	 *  overwritten in case they already exist.
	 */

	if (filename) {
		/* XXX: __FILE__/__LINE__ may not always be the most relevant,
		 * see comments in traceback formatting.  Further these are
		 * not currently set in minimal builds anyway, so disable?
		 */
		duk_push_string(ctx, filename);
		duk_def_prop_stridx(ctx, err_index, DUK_STRIDX_FILE_NAME, DUK_PROPDESC_FLAGS_WC | DUK_PROPDESC_FLAG_NO_OVERWRITE);
		duk_push_int(ctx, line);
		duk_def_prop_stridx(ctx, err_index, DUK_STRIDX_LINE_NUMBER, DUK_PROPDESC_FLAGS_WC | DUK_PROPDESC_FLAG_NO_OVERWRITE);
	} else if (thr_callstack->callstack_top > 0) {
		duk_activation *act;
		duk_hobject *func;
		duk_hbuffer *pc2line;

		act = thr_callstack->callstack + thr_callstack->callstack_top - 1;
		DUK_ASSERT(act >= thr_callstack->callstack && act < thr_callstack->callstack + thr_callstack->callstack_size);
		func = act->func;
		if (func) {
			int pc;
			duk_u32 line;

			pc = act->pc;
			pc--;  /* PC points to next instruction, find offending PC; note that
			        * PC == 0 should never be possible for an error.
			        */
			DUK_ASSERT(pc >= 0 && (double) pc < DUK_DOUBLE_2TO32);  /* assume PC is at most 32 bits and non-negative */
			act = NULL;  /* invalidated by pushes, so get out of the way */

			duk_push_hobject(ctx, func);

			duk_get_prop_stridx(ctx, -1, DUK_STRIDX_FILE_NAME);
			duk_def_prop_stridx(ctx, err_index, DUK_STRIDX_FILE_NAME, DUK_PROPDESC_FLAGS_WC | DUK_PROPDESC_FLAG_NO_OVERWRITE);
			if (DUK_HOBJECT_IS_COMPILEDFUNCTION(func)) {
#if 0
				duk_push_number(ctx, pc);
				duk_def_prop_stridx(ctx, err_index, DUK_STRIDX_PC, DUK_PROPDESC_FLAGS_WC | DUK_PROPDESC_FLAGS_NO_OVERWRITE);
#endif

				duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_PC2LINE);
				if (duk_is_buffer(ctx, -1)) {
					pc2line = duk_get_hbuffer(ctx, -1);
					DUK_ASSERT(pc2line != NULL);
					DUK_ASSERT(!DUK_HBUFFER_HAS_DYNAMIC(pc2line));
					line = duk_hobject_pc2line_query((duk_hbuffer_fixed *) pc2line, pc);
					duk_push_number(ctx, (double) line);  /* FIXME: u32 */
					duk_def_prop_stridx(ctx, err_index, DUK_STRIDX_LINE_NUMBER, DUK_PROPDESC_FLAGS_WC | DUK_PROPDESC_FLAG_NO_OVERWRITE);
				}
				duk_pop(ctx);
			} else {
				/* Native function, no relevant lineNumber. */
			}

			duk_pop(ctx);
		}
	}
#endif  /* DUK_USE_TRACEBACKS */
}

#endif  /* DUK_USE_AUGMENT_ERRORS */

