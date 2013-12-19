/*
 *  Error built-ins
 */

#include "duk_internal.h"

int duk_builtin_error_constructor_shared(duk_context *ctx) {
	/* Behavior for constructor and non-constructor call is
	 * the same except for augmenting the created error.  When
	 * called as a constructor, the caller (duk_new()) will handle
	 * augmentation; when called as normal function, we need to do
	 * it here.
	 */

	duk_hthread *thr = (duk_hthread *) ctx;
	int bidx_prototype = duk_get_magic(ctx);

	DUK_UNREF(thr);

	/* same for both error and each subclass like TypeError */
	int flags_and_class = DUK_HOBJECT_FLAG_EXTENSIBLE |
	                      DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ERROR);
	
	duk_push_object_helper(ctx, flags_and_class, bidx_prototype);

	/* If message is undefined, the own property 'message' is not set at
	 * all to save property space.  An empty message is inherited anyway.
	 */
	if (!duk_is_undefined(ctx, 0)) {
		duk_to_string(ctx, 0);
		duk_dup(ctx, 0);  /* [ message error message ] */
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	}

	/* Augment the error if called as a normal function.  __FILE__ and __LINE__
	 * are not desirable in this case.
	 */

#ifdef DUK_USE_AUGMENT_ERRORS
	if (!duk_is_constructor_call(ctx)) {
		duk_err_augment_error(thr, thr, -1, NULL, 0, 1 /*noblame_fileline*/);
	}
#endif

	return 1;
}

int duk_builtin_error_prototype_to_string(duk_context *ctx) {
	/* FIXME: optimize with more direct internal access */

	duk_push_this(ctx);
	if (!duk_is_object(ctx, -1)) {
		goto type_error;
	}

	/* [ ... this ] */

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_NAME);
	if (duk_is_undefined(ctx, -1)) {
		duk_pop(ctx);
		duk_push_string(ctx, "Error");
	} else {
		duk_to_string(ctx, -1);
	}

	/* [ ... this name ] */

	/* FIXME: Are steps 6 and 7 in E5 Section 15.11.4.4 duplicated by
	 * accident or are they actually needed?  The first ToString()
	 * could conceivably return 'undefined'.
	 */
	duk_get_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE);
	if (duk_is_undefined(ctx, -1)) {
		duk_pop(ctx);
		duk_push_string(ctx, "");
	} else {
		duk_to_string(ctx, -1);
	}

	/* [ ... this name message ] */

	if (duk_get_length(ctx, -2) == 0) {
		/* name is empty -> return message */
		return 1;
	}
	if (duk_get_length(ctx, -1) == 0) {
		/* message is empty -> return name */
		duk_pop(ctx);
		return 1;
	}
	duk_push_string(ctx, ": ");
	duk_insert(ctx, -2);  /* ... name ': ' message */
	duk_concat(ctx, 3);

	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

#ifdef DUK_USE_TRACEBACKS

/*
 *  Traceback handling
 *
 *  The unified helper decodes the traceback and produces various requested
 *  outputs.  It should be optimized for size, and may leave garbage on stack,
 *  only the topmost return value matters.  For instance, traceback separator
 *  and decoded strings are pushed even when looking for filename only.
 */

/* constants arbitrary, chosen for small loads */
#define DUK__OUTPUT_TYPE_TRACEBACK   (-1)
#define DUK__OUTPUT_TYPE_FILENAME    0
#define DUK__OUTPUT_TYPE_LINENUMBER  1

static int traceback_getter_helper(duk_context *ctx, int output_type) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int idx_td;
	int i;
	const char *str_tailcalled = " tailcalled";
	const char *str_strict = " strict";
	const char *str_construct = " construct";
	const char *str_prevyield = " preventsyield";
	const char *str_directeval = " directeval";
	const char *str_empty = "";

	DUK_ASSERT_TOP(ctx, 0);  /* fixed arg count */

	duk_push_this(ctx);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TRACEDATA);
	idx_td = duk_get_top_index(ctx);

	duk_push_hstring_stridx(ctx, DUK_STRIDX_NEWLINE_TAB);
	duk_push_this(ctx);
	duk_to_string(ctx, -1);

	/* [ ... this tracedata sep ToString(this) ] */

	/* FIXME: skip null filename? */

	if (duk_check_type(ctx, idx_td, DUK_TYPE_OBJECT)) {
		int t;

		/* Current tracedata contains 2 entries per callstack entry. */
		for (i = 0; ; i += 2) {
			int pc;
			int line;
			int flags;
			double d;
			const char *funcname;
			duk_hobject *h_func;
			duk_hstring *h_name;
			duk_hbuffer_fixed *pc2line;

			duk_require_stack(ctx, 5);
			duk_get_prop_index(ctx, idx_td, i);
			duk_get_prop_index(ctx, idx_td, i + 1);
			d = duk_to_number(ctx, -1);
			pc = (int) fmod(d, DUK_DOUBLE_2TO32);
			flags = (int) floor(d / DUK_DOUBLE_2TO32);
			t = duk_get_type(ctx, -2);

			if (t == DUK_TYPE_OBJECT) {
				/*
				 *  Ecmascript/native function call
				 */

				/* [ ... v1(func) v2(pc+flags) ] */

				h_func = duk_get_hobject(ctx, -2);
				DUK_ASSERT(h_func != NULL);

				duk_get_prop_stridx(ctx, -2, DUK_STRIDX_NAME);
				duk_get_prop_stridx(ctx, -3, DUK_STRIDX_FILE_NAME);

				duk_get_prop_stridx(ctx, -4, DUK_STRIDX_INT_PC2LINE);
				if (duk_is_buffer(ctx, -1)) {
					pc2line = (duk_hbuffer_fixed *) duk_get_hbuffer(ctx, -1);
					DUK_ASSERT(!DUK_HBUFFER_HAS_DYNAMIC(pc2line));
					line = duk_hobject_pc2line_query(pc2line, pc);
				} else {
					line = 0;
				}
				duk_pop(ctx);

				/* [ ... v1 v2 name filename ] */

				if (output_type == DUK__OUTPUT_TYPE_FILENAME) {
					return 1;
				} else if (output_type == DUK__OUTPUT_TYPE_LINENUMBER) {
					duk_push_int(ctx, line);
					return 1;
				}

				h_name = duk_get_hstring(ctx, -2);  /* may be NULL */
				funcname = (h_name == NULL || h_name == DUK_HTHREAD_STRING_EMPTY_STRING(thr)) ?
				           "anon" : (const char *) DUK_HSTRING_GET_DATA(h_name);
				if (DUK_HOBJECT_HAS_NATIVEFUNCTION(h_func)) {
					duk_push_sprintf(ctx, "%s %s native%s%s%s%s%s",
					                 funcname,
					                 duk_get_string(ctx, -1),
					                 (flags & DUK_ACT_FLAG_STRICT) ? str_strict : str_empty,
					                 (flags & DUK_ACT_FLAG_TAILCALLED) ? str_tailcalled : str_empty,
					                 (flags & DUK_ACT_FLAG_CONSTRUCT) ? str_construct : str_empty,
					                 (flags & DUK_ACT_FLAG_DIRECT_EVAL) ? str_directeval : str_empty,
					                 (flags & DUK_ACT_FLAG_PREVENT_YIELD) ? str_prevyield : str_empty);

				} else {
					duk_push_sprintf(ctx, "%s %s:%d%s%s%s%s%s",
					                 funcname,
					                 duk_get_string(ctx, -1),
					                 line,
					                 (flags & DUK_ACT_FLAG_STRICT) ? str_strict : str_empty,
					                 (flags & DUK_ACT_FLAG_TAILCALLED) ? str_tailcalled : str_empty,
					                 (flags & DUK_ACT_FLAG_CONSTRUCT) ? str_construct : str_empty,
					                 (flags & DUK_ACT_FLAG_DIRECT_EVAL) ? str_directeval : str_empty,
					                 (flags & DUK_ACT_FLAG_PREVENT_YIELD) ? str_prevyield : str_empty);
				}
				duk_replace(ctx, -5);   /* [ ... v1 v2 name filename str ] -> [ ... str v2 name filename ] */
				duk_pop_n(ctx, 3);      /* -> [ ... str ] */
			} else if (t == DUK_TYPE_STRING) {
				/*
				 *  __FILE__ / __LINE__ entry, here 'pc' is line number directly.
				 *  Sometimes __FILE__ / __LINE__ is reported as the source for
				 *  the error (fileName, lineNumber), sometimes not.
				 */

				/* [ ... v1(filename) v2(line+flags) ] */

				if (!(flags & DUK_TB_FLAG_NOBLAME_FILELINE)) {
					if (output_type == DUK__OUTPUT_TYPE_FILENAME) {
						duk_pop(ctx);
						return 1;
					} else if (output_type == DUK__OUTPUT_TYPE_LINENUMBER) {
						duk_push_int(ctx, pc);
						return 1;
					}
				}

				duk_push_sprintf(ctx, "%s:%d",
				                 duk_get_string(ctx, -2), pc);
				duk_replace(ctx, -3);  /* [ ... v1 v2 str ] -> [ ... str v2 ] */
				duk_pop(ctx);          /* -> [ ... str ] */
			} else {
				/* unknown, ignore */
				duk_pop_2(ctx);
				break;
			}
		}

		if (i >= DUK_USE_TRACEBACK_DEPTH * 2) {
			/* Possibly truncated; there is no explicit truncation
			 * marker so this is the best we can do.
			 */

			duk_push_hstring_stridx(ctx, DUK_STRIDX_BRACKETED_ELLIPSIS);
		}
	}

	/* [ ... this tracedata sep ToString(this) str1 ... strN ] */

	if (output_type != DUK__OUTPUT_TYPE_TRACEBACK) {
		return 0;
	} else {
		duk_join(ctx, duk_get_top(ctx) - (idx_td + 2) /*count, not including sep*/);
		return 1;
	}
}

/* FIXME: output type could be encoded into native function 'magic' value to
 * save space.
 */

int duk_builtin_error_prototype_stack_getter(duk_context *ctx) {
	return traceback_getter_helper(ctx, DUK__OUTPUT_TYPE_TRACEBACK);
}

int duk_builtin_error_prototype_filename_getter(duk_context *ctx) {
	return traceback_getter_helper(ctx, DUK__OUTPUT_TYPE_FILENAME);
}

int duk_builtin_error_prototype_linenumber_getter(duk_context *ctx) {
	return traceback_getter_helper(ctx, DUK__OUTPUT_TYPE_LINENUMBER);
}

#undef DUK__OUTPUT_TYPE_TRACEBACK
#undef DUK__OUTPUT_TYPE_FILENAME
#undef DUK__OUTPUT_TYPE_LINENUMBER

#else  /* DUK_USE_TRACEBACKS */

/*
 *  Traceback handling when tracebacks disabled.
 *
 *  The fileName / lineNumber stubs are now necessary because built-in
 *  data will include the accessor properties in Error.prototype.  If those
 *  are removed for builds without tracebacks, these can also be removed.
 *  'stack' should still be present and produce a ToString() equivalent:
 *  this is useful for user code which prints a stacktrace and expects to
 *  see something useful.  A normal stacktrace also begins with a ToString()
 *  of the error so this makes sense.
 */

int duk_builtin_error_prototype_stack_getter(duk_context *ctx) {
	/* FIXME: remove this native function and map 'stack' accessor
	 * to the toString() implementation directly.
	 */
	return duk_builtin_error_prototype_to_string(ctx);
}

int duk_builtin_error_prototype_filename_getter(duk_context *ctx) {
	return 0;
}

int duk_builtin_error_prototype_linenumber_getter(duk_context *ctx) {
	return 0;
}

#endif  /* DUK_USE_TRACEBACKS */

int duk_builtin_error_prototype_nop_setter(duk_context *ctx) {
	/* Attempt to write 'stack', 'fileName', 'lineNumber' is a silent no-op.
	 * User can use Object.defineProperty() to override this behavior.
	 */
	DUK_ASSERT_TOP(ctx, 1);  /* fixed arg count */
	DUK_UNREF(ctx);
	return 0;
}
