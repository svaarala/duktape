/*
 *  Error built-ins
 */

#include "duk_internal.h"

static int duk_error_constructor_helper(duk_context *ctx, int bidx_prototype) {
	/* Behavior for constructor and non-constructor call is
	 * exactly the same.
	 */

	/* same for both error and each subclass like TypeError */
	int flags_and_class = DUK_HOBJECT_FLAG_EXTENSIBLE |
	                      DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ERROR);
	
	duk_push_object_helper(ctx, flags_and_class, bidx_prototype);

	if (!duk_is_undefined(ctx, 0)) {
		duk_to_string(ctx, 0);
		duk_dup(ctx, 0);  /* [ message error message ] */
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	}

	return 1;
}

int duk_builtin_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_ERROR_PROTOTYPE);
}

int duk_builtin_eval_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_EVAL_ERROR_PROTOTYPE);
}

int duk_builtin_range_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_RANGE_ERROR_PROTOTYPE);
}

int duk_builtin_reference_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_REFERENCE_ERROR_PROTOTYPE);
}

int duk_builtin_syntax_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_SYNTAX_ERROR_PROTOTYPE);
}

int duk_builtin_type_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_TYPE_ERROR_PROTOTYPE);
}

int duk_builtin_uri_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_URI_ERROR_PROTOTYPE);
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

/* FIXME: This needs to actually be a getter/setter pair, this is
 * for prototyping traceback formatting.  The resulting code is
 * also way too large, so needs rework.
 */
int duk_builtin_error_prototype_stack(duk_context *ctx) {
	int idx_td;
	int i;
	const char *str_tailcalled = " tailcalled";
	const char *str_strict = " strict";
	const char *str_construct = " construct";
	const char *str_empty = "";

	duk_push_this(ctx);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TRACEDATA);
	idx_td = duk_get_top_index(ctx);

	duk_push_hstring_stridx(ctx, DUK_STRIDX_NEWLINE_TAB);
	duk_push_this(ctx);
	duk_to_string(ctx, -1);

	/* [ ... this tracedata sep ToString(this) ] */

	/* FIXME: indicate truncated traceback */
	/* FIXME: skip null filename? */

	if (duk_check_type(ctx, idx_td, DUK_TYPE_OBJECT)) {
		int t;

		/* Current tracedata contains 2 entries per callstack entry. */
		for (i = 0; ; i += 2) {
			int pc;
			int line;
			int flags;
			double d;
			duk_hobject *h;
			duk_hbuffer_fixed *pc2line;

			duk_require_stack(ctx, 5);
			duk_get_prop_index(ctx, idx_td, i);
			duk_get_prop_index(ctx, idx_td, i + 1);
			d = duk_to_number(ctx, -1);
			t = duk_get_type(ctx, -2);
			if (t == DUK_TYPE_OBJECT) {
				h = duk_get_hobject(ctx, -2);
				DUK_ASSERT(h != NULL);

				pc = (int) fmod(d, DUK_DOUBLE_2TO32);
				flags = (int) floor(d / DUK_DOUBLE_2TO32);
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

				if (DUK_HOBJECT_HAS_NATIVEFUNCTION(h)) {
					duk_push_sprintf(ctx, "%s %s native%s%s%s",
					                 duk_get_string(ctx, -2), duk_get_string(ctx, -1),
					                 (flags & DUK_ACT_FLAG_STRICT) ? str_strict : str_empty,
					                 (flags & DUK_ACT_FLAG_TAILCALLED) ? str_tailcalled : str_empty,
					                 (flags & DUK_ACT_FLAG_CONSTRUCT) ? str_construct : str_empty);

				} else {
					duk_push_sprintf(ctx, "%s %s:%d%s%s%s",
					                 duk_get_string(ctx, -2), duk_get_string(ctx, -1), line,
					                 (flags & DUK_ACT_FLAG_STRICT) ? str_strict : str_empty,
					                 (flags & DUK_ACT_FLAG_TAILCALLED) ? str_tailcalled : str_empty,
					                 (flags & DUK_ACT_FLAG_CONSTRUCT) ? str_construct : str_empty);
				}
				duk_replace(ctx, -5);   /* [ ... v1 v2 name filename str ] -> [ ... str v2 name filename ] */
				duk_pop_n(ctx, 3);      /* -> [ ... str ] */
			} else if (t == DUK_TYPE_STRING) {
				duk_push_sprintf(ctx, "%s:%d",
				                      duk_get_string(ctx, -2), (int) d);
				duk_replace(ctx, -3);  /* [ ... v1 v2 str ] -> [ ... str v2 ] */
				duk_pop(ctx);          /* -> [ ... str ] */
			} else {
				duk_pop_2(ctx);
				break;
			}
		}

		if (i >= DUK_OPT_TRACEBACK_DEPTH * 2) {
			/* Possibly truncated; there is no explicit truncation
			 * marker so this is the best we can do.
			 */
			duk_push_hstring_stridx(ctx, DUK_STRIDX_BRACKETED_ELLIPSIS);
		}
	}

	/* [ ... this tracedata sep ToString(this) str1 ... strN ] */

	duk_join(ctx, duk_get_top(ctx) - (idx_td + 2) /*count, not including sep*/);
	return 1;
}

