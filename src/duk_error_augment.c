/*
 *  Augmenting errors at their creation site and their throw site.
 *
 *  When errors are created, traceback data is added by built-in code
 *  and a user error handler (if defined) can process or replace the
 *  error.  Similarly, when errors are thrown, a user error handler
 *  (if defined) can process or replace the error.
 *
 *  Augmentation and other processing at error creation time is nice
 *  because an error is only created once, but it may be thrown and
 *  rethrown multiple times.  User error handler registered for processing
 *  an error at its throw site must be careful to handle rethrowing in
 *  a useful manner.
 *
 *  Error augmentation may throw an internal error (e.g. alloc error).
 *
 *  Ecmascript allows throwing any values, so all values cannot be
 *  augmented.  Currently, the built-in augmentation at error creation
 *  only augments error values which are Error instances (= have the
 *  built-in Error.prototype in their prototype chain) and are also
 *  extensible.  User error handlers have no limitations in this respect.
 */

#include "duk_internal.h"

/*
 *  Helper for calling a user error handler.
 *
 *  'thr' must be the currently active thread; the error handler is called
 *  in its context.  The valstack of 'thr' must have the error value on
 *  top, and will be replaced by another error value based on the return
 *  value of the error handler.
 *
 *  The helper calls duk_handle_call() recursively in protected mode.
 *  Before that call happens, no longjmps should happen; as a consequence,
 *  we must assume that the valstack contains enough temporary space for
 *  arguments and such.
 *
 *  While the error handler runs, any errors thrown will not trigger a
 *  recursive error handler call (this is implemented using a heap level
 *  flag which will "follow" through any coroutines resumed inside the
 *  error handler).  If the error handler is not callable or throws an
 *  error, the resulting error replaces the original error (for Duktape
 *  internal errors, duk_error_throw.c further substitutes this error with
 *  a DoubleError which is not ideal).  This would be easy to change and
 *  even signal to the caller.
 *
 *  The user error handler is stored in 'Duktape.errCreate' or
 *  'Duktape.errThrow' depending on whether we're augmenting the error at
 *  creation or throw time.  There are several alternatives to this approach,
 *  see doc/error-objects.rst for discussion.
 *
 *  Note: since further longjmp()s may occur while calling the error handler
 *  (for many reasons, e.g. a labeled 'break' inside the handler), the
 *  caller can make no assumptions on the thr->heap->lj state after the
 *  call (this affects especially duk_error_throw.c).  This is not an issue
 *  as long as the caller writes to the lj state only after the error handler
 *  finishes.
 */

#if defined(DUK_USE_ERRTHROW) || defined(DUK_USE_ERRCREATE)
DUK_LOCAL void duk__err_augment_user(duk_hthread *thr, duk_small_uint_t stridx_cb) {
	duk_context *ctx = (duk_context *) thr;
	duk_tval *tv_hnd;
	duk_small_uint_t call_flags;
	duk_int_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT_DISABLE(stridx_cb >= 0);  /* unsigned */
	DUK_ASSERT(stridx_cb < DUK_HEAP_NUM_STRINGS);

	if (DUK_HEAP_HAS_ERRHANDLER_RUNNING(thr->heap)) {
		DUK_DD(DUK_DDPRINT("recursive call to error handler, ignore"));
		return;
	}

	/*
	 *  Check whether or not we have an error handler.
	 *
	 *  We must be careful of not triggering an error when looking up the
	 *  property.  For instance, if the property is a getter, we don't want
	 *  to call it, only plain values are allowed.  The value, if it exists,
	 *  is not checked.  If the value is not a function, a TypeError happens
	 *  when it is called and that error replaces the original one.
	 */

	DUK_ASSERT_VALSTACK_SPACE(thr, 4);  /* 3 entries actually needed below */

	/* [ ... errval ] */

	if (thr->builtins[DUK_BIDX_DUKTAPE] == NULL) {
		/* When creating built-ins, some of the built-ins may not be set
		 * and we want to tolerate that when throwing errors.
		 */
		DUK_DD(DUK_DDPRINT("error occurred when DUK_BIDX_DUKTAPE is NULL, ignoring"));
		return;
	}
	tv_hnd = duk_hobject_find_existing_entry_tval_ptr(thr->heap,
	                                                  thr->builtins[DUK_BIDX_DUKTAPE],
	                                                  DUK_HTHREAD_GET_STRING(thr, stridx_cb));
	if (tv_hnd == NULL) {
		DUK_DD(DUK_DDPRINT("error handler does not exist or is not a plain value: %!T",
		                   (duk_tval *) tv_hnd));
		return;
	}
	DUK_DDD(DUK_DDDPRINT("error handler dump (callability not checked): %!T",
	                     (duk_tval *) tv_hnd));
	duk_push_tval(ctx, tv_hnd);

	/* [ ... errval errhandler ] */

	duk_insert(ctx, -2);  /* -> [ ... errhandler errval ] */
	duk_push_undefined(ctx);
	duk_insert(ctx, -2);  /* -> [ ... errhandler undefined(= this) errval ] */

	/* [ ... errhandler undefined errval ] */

	/*
	 *  DUK_CALL_FLAG_IGNORE_RECLIMIT causes duk_handle_call() to ignore C
	 *  recursion depth limit (and won't increase it either).  This is
	 *  dangerous, but useful because it allows the error handler to run
	 *  even if the original error is caused by C recursion depth limit.
	 *
	 *  The heap level DUK_HEAP_FLAG_ERRHANDLER_RUNNING is set for the
	 *  duration of the error handler and cleared afterwards.  This flag
	 *  prevents the error handler from running recursively.  The flag is
	 *  heap level so that the flag properly controls even coroutines
	 *  launched by an error handler.  Since the flag is heap level, it is
	 *  critical to restore it correctly.
	 *
	 *  We ignore errors now: a success return and an error value both
	 *  replace the original error value.  (This would be easy to change.)
	 */

	DUK_ASSERT(!DUK_HEAP_HAS_ERRHANDLER_RUNNING(thr->heap));  /* since no recursive error handler calls */
	DUK_HEAP_SET_ERRHANDLER_RUNNING(thr->heap);

	call_flags = DUK_CALL_FLAG_IGNORE_RECLIMIT;  /* ignore reclimit, not constructor */

	rc = duk_handle_call_protected(thr,
	                               1,            /* num args */
	                               call_flags);  /* call_flags */
	DUK_UNREF(rc);  /* no need to check now: both success and error are OK */

	DUK_ASSERT(DUK_HEAP_HAS_ERRHANDLER_RUNNING(thr->heap));
	DUK_HEAP_CLEAR_ERRHANDLER_RUNNING(thr->heap);

	/* [ ... errval ] */
}
#endif  /* DUK_USE_ERRTHROW || DUK_USE_ERRCREATE */

/*
 *  Add ._Tracedata to an error on the stack top.
 */

#if defined(DUK_USE_TRACEBACKS)
DUK_LOCAL void duk__add_traceback(duk_hthread *thr, duk_hthread *thr_callstack, const char *c_filename, duk_int_t c_line, duk_bool_t noblame_fileline) {
	duk_context *ctx = (duk_context *) thr;
	duk_small_uint_t depth;
	duk_int_t i, i_min;
	duk_uarridx_t arr_idx;
	duk_double_t d;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr_callstack != NULL);
	DUK_ASSERT(ctx != NULL);

	/* [ ... error ] */

	/*
	 *  The traceback format is pretty arcane in an attempt to keep it compact
	 *  and cheap to create.  It may change arbitrarily from version to version.
	 *  It should be decoded/accessed through version specific accessors only.
	 *
	 *  See doc/error-objects.rst.
	 */

	DUK_DDD(DUK_DDDPRINT("adding traceback to object: %!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	duk_push_array(ctx);  /* XXX: specify array size, as we know it */
	arr_idx = 0;

	/* Compiler SyntaxErrors (and other errors) come first, and are
	 * blamed by default (not flagged "noblame").
	 */
	if (thr->compile_ctx != NULL && thr->compile_ctx->h_filename != NULL) {
		duk_push_hstring(ctx, thr->compile_ctx->h_filename);
		duk_xdef_prop_index_wec(ctx, -2, arr_idx);
		arr_idx++;

		duk_push_uint(ctx, (duk_uint_t) thr->compile_ctx->curr_token.start_line);  /* (flags<<32) + (line), flags = 0 */
		duk_xdef_prop_index_wec(ctx, -2, arr_idx);
		arr_idx++;
	}

	/* Filename/line from C macros (__FILE__, __LINE__) are added as an
	 * entry with a special format: (string, number).  The number contains
	 * the line and flags.
	 */

	/* XXX: optimize: allocate an array part to the necessary size (upwards
	 * estimate) and fill in the values directly into the array part; finally
	 * update 'length'.
	 */

	/* XXX: using duk_put_prop_index() would cause obscure error cases when Array.prototype
	 * has write-protected array index named properties.  This was seen as DoubleErrors
	 * in e.g. some test262 test cases.  Using duk_xdef_prop_index() is better but heavier.
	 * The best fix is to fill in the tracedata directly into the array part.  There are
	 * no side effect concerns if the array part is allocated directly and only INCREFs
	 * happen after that.
	 */

	/* [ ... error arr ] */

	if (c_filename) {
		duk_push_string(ctx, c_filename);
		duk_xdef_prop_index_wec(ctx, -2, arr_idx);
		arr_idx++;

		d = (noblame_fileline ? ((duk_double_t) DUK_TB_FLAG_NOBLAME_FILELINE) * DUK_DOUBLE_2TO32 : 0.0) +
		    (duk_double_t) c_line;
		duk_push_number(ctx, d);
		duk_xdef_prop_index_wec(ctx, -2, arr_idx);
		arr_idx++;
	}

	/* traceback depth doesn't take into account the filename/line
	 * special handling above (intentional)
	 */
	depth = DUK_USE_TRACEBACK_DEPTH;
	i_min = (thr_callstack->callstack_top > (duk_size_t) depth ? (duk_int_t) (thr_callstack->callstack_top - depth) : 0);
	DUK_ASSERT(i_min >= 0);

	/* [ ... error arr ] */

	DUK_ASSERT(thr_callstack->callstack_top <= DUK_INT_MAX);  /* callstack limits */
	for (i = (duk_int_t) (thr_callstack->callstack_top - 1); i >= i_min; i--) {
		duk_uint32_t pc;

		/*
		 *  Note: each API operation potentially resizes the callstack,
		 *  so be careful to re-lookup after every operation.  Currently
		 *  these is no issue because we don't store a temporary 'act'
		 *  pointer at all.  (This would be a non-issue if we operated
		 *  directly on the array part.)
		 */

		/* [... arr] */

		DUK_ASSERT_DISABLE(thr_callstack->callstack[i].pc >= 0);  /* unsigned */

		/* Add function object. */
		duk_push_tval(ctx, &(thr_callstack->callstack + i)->tv_func);
		duk_xdef_prop_index_wec(ctx, -2, arr_idx);
		arr_idx++;

		/* Add a number containing: pc, activation flags.
		 *
		 * PC points to next instruction, find offending PC.  Note that
		 * PC == 0 for native code.
		 */
		pc = duk_hthread_get_act_prev_pc(thr_callstack, thr_callstack->callstack + i);
		DUK_ASSERT_DISABLE(pc >= 0);  /* unsigned */
		DUK_ASSERT((duk_double_t) pc < DUK_DOUBLE_2TO32);  /* assume PC is at most 32 bits and non-negative */
		d = ((duk_double_t) thr_callstack->callstack[i].flags) * DUK_DOUBLE_2TO32 + (duk_double_t) pc;
		duk_push_number(ctx, d);  /* -> [... arr num] */
		duk_xdef_prop_index_wec(ctx, -2, arr_idx);
		arr_idx++;
	}

	/* XXX: set with duk_hobject_set_length() when tracedata is filled directly */
	duk_push_uint(ctx, (duk_uint_t) arr_idx);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_WC);

	/* [ ... error arr ] */

	duk_xdef_prop_stridx_wec(ctx, -2, DUK_STRIDX_INT_TRACEDATA);  /* -> [ ... error ] */
}
#endif  /* DUK_USE_TRACEBACKS */

/*
 *  Add .fileName and .lineNumber to an error on the stack top.
 */

#if !defined(DUK_USE_TRACEBACKS)
DUK_LOCAL void duk__add_fileline(duk_hthread *thr, duk_hthread *thr_callstack, const char *c_filename, duk_int_t c_line, duk_bool_t noblame_fileline) {
	duk_context *ctx;
#if defined(DUK_USE_ASSERTIONS)
	duk_int_t entry_top;
#endif

	ctx = (duk_context *) thr;
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(ctx);
#endif

	/*
	 *  If tracebacks are disabled, 'fileName' and 'lineNumber' are added
	 *  as plain own properties.  Since Error.prototype has accessors of
	 *  the same name, we need to define own properties directly (cannot
	 *  just use e.g. duk_put_prop_stridx).  Existing properties are not
	 *  overwritten in case they already exist.
	 */

	if (thr->compile_ctx != NULL && thr->compile_ctx->h_filename != NULL) {
		/* Compiler SyntaxError (or other error) gets the primary blame.
		 * Currently no flag to prevent blaming.
		 */
		duk_push_uint(ctx, (duk_uint_t) thr->compile_ctx->curr_token.start_line);
		duk_push_hstring(ctx, thr->compile_ctx->h_filename);
	} else if (c_filename && !noblame_fileline) {
		/* C call site gets blamed next, unless flagged not to do so.
		 * XXX: file/line is disabled in minimal builds, so disable this
		 * too when appropriate.
		 */
		duk_push_int(ctx, c_line);
		duk_push_string(ctx, c_filename);
	} else {
		/* Finally, blame the innermost callstack entry which has a
		 * .fileName property.
		 */
		duk_small_uint_t depth;
		duk_int_t i, i_min;
		duk_uint32_t ecma_line;

		depth = DUK_USE_TRACEBACK_DEPTH;
		i_min = (thr_callstack->callstack_top > (duk_size_t) depth ? (duk_int_t) (thr_callstack->callstack_top - depth) : 0);
		DUK_ASSERT(i_min >= 0);

		DUK_ASSERT(thr_callstack->callstack_top <= DUK_INT_MAX);  /* callstack limits */
		for (i = (duk_int_t) (thr_callstack->callstack_top - 1); i >= i_min; i--) {
			duk_activation *act;
			duk_hobject *func;
			duk_uint32_t pc;

			DUK_UNREF(pc);
			act = thr_callstack->callstack + i;
			DUK_ASSERT(act >= thr_callstack->callstack && act < thr_callstack->callstack + thr_callstack->callstack_size);

			func = DUK_ACT_GET_FUNC(act);
			if (func == NULL) {
				/* Lightfunc, not blamed now. */
				continue;
			}

			/* PC points to next instruction, find offending PC,
			 * PC == 0 for native code.
			 */
			pc = duk_hthread_get_act_prev_pc(thr, act);  /* thr argument only used for thr->heap, so specific thread doesn't matter */
			DUK_ASSERT_DISABLE(pc >= 0);  /* unsigned */
			DUK_ASSERT((duk_double_t) pc < DUK_DOUBLE_2TO32);  /* assume PC is at most 32 bits and non-negative */
			act = NULL;  /* invalidated by pushes, so get out of the way */

			duk_push_hobject(ctx, func);

			/* [ ... error func ] */

			duk_get_prop_stridx(ctx, -1, DUK_STRIDX_FILE_NAME);
			if (!duk_is_string(ctx, -1)) {
				duk_pop_2(ctx);
				continue;
			}

			/* [ ... error func fileName ] */

			ecma_line = 0;
#if defined(DUK_USE_PC2LINE)
			if (DUK_HOBJECT_IS_COMPILEDFUNCTION(func)) {
				ecma_line = duk_hobject_pc2line_query(ctx, -2, (duk_uint_fast32_t) pc);
			} else {
				/* Native function, no relevant lineNumber. */
			}
#endif  /* DUK_USE_PC2LINE */
			duk_push_u32(ctx, ecma_line);

			/* [ ... error func fileName lineNumber ] */

			duk_replace(ctx, -3);

			/* [ ... error lineNumber fileName ] */
			goto define_props;
		}

		/* No activation matches, use undefined for both .fileName and
		 * .lineNumber (matches what we do with a _Tracedata based
		 * no-match lookup.
		 */
		duk_push_undefined(ctx);
		duk_push_undefined(ctx);
	}

 define_props:
	/* [ ... error lineNumber fileName ] */
#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(duk_get_top(ctx) == entry_top + 2);
#endif
	duk_xdef_prop_stridx(ctx, -3, DUK_STRIDX_FILE_NAME, DUK_PROPDESC_FLAGS_WC | DUK_PROPDESC_FLAG_NO_OVERWRITE);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_LINE_NUMBER, DUK_PROPDESC_FLAGS_WC | DUK_PROPDESC_FLAG_NO_OVERWRITE);
}
#endif  /* !DUK_USE_TRACEBACKS */

/*
 *  Add line number to a compiler error.
 */

DUK_LOCAL void duk__add_compiler_error_line(duk_hthread *thr) {
	duk_context *ctx;

	/* Append a "(line NNN)" to the "message" property of any error
	 * thrown during compilation.  Usually compilation errors are
	 * SyntaxErrors but they can also be out-of-memory errors and
	 * the like.
	 */

	/* [ ... error ] */

	ctx = (duk_context *) thr;
	DUK_ASSERT(duk_is_object(ctx, -1));

	if (!(thr->compile_ctx != NULL && thr->compile_ctx->h_filename != NULL)) {
		return;
	}

	DUK_DDD(DUK_DDDPRINT("compile error, before adding line info: %!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	if (duk_get_prop_stridx(ctx, -1, DUK_STRIDX_MESSAGE)) {
		duk_push_sprintf(ctx, " (line %ld)", (long) thr->compile_ctx->curr_token.start_line);
		duk_concat(ctx, 2);
		duk_put_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE);
	} else {
		duk_pop(ctx);
	}

	DUK_DDD(DUK_DDDPRINT("compile error, after adding line info: %!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));
}

/*
 *  Augment an error being created using Duktape specific properties
 *  like _Tracedata or .fileName/.lineNumber.
 */

#if defined(DUK_USE_AUGMENT_ERROR_CREATE)
DUK_LOCAL void duk__err_augment_builtin_create(duk_hthread *thr, duk_hthread *thr_callstack, const char *c_filename, duk_int_t c_line, duk_small_int_t noblame_fileline, duk_hobject *obj) {
	duk_context *ctx = (duk_context *) thr;
#if defined(DUK_USE_ASSERTIONS)
	duk_int_t entry_top;
#endif

#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(ctx);
#endif
	DUK_ASSERT(obj != NULL);

	DUK_UNREF(obj);  /* unreferenced w/o tracebacks */
	DUK_UNREF(ctx);  /* unreferenced w/o asserts */

	duk__add_compiler_error_line(thr);

#if defined(DUK_USE_TRACEBACKS)
	/* If tracebacks are enabled, the '_Tracedata' property is the only
	 * thing we need: 'fileName' and 'lineNumber' are virtual properties
	 * which use '_Tracedata'.
	 */
	if (duk_hobject_hasprop_raw(thr, obj, DUK_HTHREAD_STRING_INT_TRACEDATA(thr))) {
		DUK_DDD(DUK_DDDPRINT("error value already has a '_Tracedata' property, not modifying it"));
	} else {
		duk__add_traceback(thr, thr_callstack, c_filename, c_line, noblame_fileline);
	}
#else
	/* Without tracebacks the concrete .fileName and .lineNumber need
	 * to be added directly.
	 */
	duk__add_fileline(thr, thr_callstack, c_filename, c_line, noblame_fileline);
#endif

#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(duk_get_top(ctx) == entry_top);
#endif
}
#endif  /* DUK_USE_AUGMENT_ERROR_CREATE */

/*
 *  Augment an error at creation time with _Tracedata/fileName/lineNumber
 *  and allow a user error handler (if defined) to process/replace the error.
 *  The error to be augmented is at the stack top.
 *
 *  thr: thread containing the error value
 *  thr_callstack: thread which should be used for generating callstack etc.
 *  c_filename: C __FILE__ related to the error
 *  c_line: C __LINE__ related to the error
 *  noblame_fileline: if true, don't fileName/line as error source, otherwise use traceback
 *                    (needed because user code filename/line are reported but internal ones
 *                    are not)
 *
 *  XXX: rename noblame_fileline to flags field; combine it to some existing
 *  field (there are only a few call sites so this may not be worth it).
 */

#if defined(DUK_USE_AUGMENT_ERROR_CREATE)
DUK_INTERNAL void duk_err_augment_error_create(duk_hthread *thr, duk_hthread *thr_callstack, const char *c_filename, duk_int_t c_line, duk_bool_t noblame_fileline) {
	duk_context *ctx = (duk_context *) thr;
	duk_hobject *obj;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr_callstack != NULL);
	DUK_ASSERT(ctx != NULL);

	/* [ ... error ] */

	/*
	 *  Criteria for augmenting:
	 *
	 *   - augmentation enabled in build (naturally)
	 *   - error value internal prototype chain contains the built-in
	 *     Error prototype object (i.e. 'val instanceof Error')
	 *
	 *  Additional criteria for built-in augmenting:
	 *
	 *   - error value is an extensible object
	 */

	obj = duk_get_hobject(ctx, -1);
	if (!obj) {
		DUK_DDD(DUK_DDDPRINT("value is not an object, skip both built-in and user augment"));
		return;
	}
	if (!duk_hobject_prototype_chain_contains(thr, obj, thr->builtins[DUK_BIDX_ERROR_PROTOTYPE], 1 /*ignore_loop*/)) {
		/* If the value has a prototype loop, it's critical not to
		 * throw here.  Instead, assume the value is not to be
		 * augmented.
		 */
		DUK_DDD(DUK_DDDPRINT("value is not an error instance, skip both built-in and user augment"));
		return;
	}
	if (DUK_HOBJECT_HAS_EXTENSIBLE(obj)) {
		DUK_DDD(DUK_DDDPRINT("error meets criteria, built-in augment"));
		duk__err_augment_builtin_create(thr, thr_callstack, c_filename, c_line, noblame_fileline, obj);
	} else {
		DUK_DDD(DUK_DDDPRINT("error does not meet criteria, no built-in augment"));
	}

	/* [ ... error ] */

#if defined(DUK_USE_ERRCREATE)
	duk__err_augment_user(thr, DUK_STRIDX_ERR_CREATE);
#endif
}
#endif  /* DUK_USE_AUGMENT_ERROR_CREATE */

/*
 *  Augment an error at throw time; allow a user error handler (if defined)
 *  to process/replace the error.  The error to be augmented is at the
 *  stack top.
 */

#if defined(DUK_USE_AUGMENT_ERROR_THROW)
DUK_INTERNAL void duk_err_augment_error_throw(duk_hthread *thr) {
#if defined(DUK_USE_ERRTHROW)
	duk__err_augment_user(thr, DUK_STRIDX_ERR_THROW);
#endif  /* DUK_USE_ERRTHROW */
}
#endif  /* DUK_USE_AUGMENT_ERROR_THROW */
