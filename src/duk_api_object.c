/*
 *  Object handling: property access and other support functions.
 */

#include "duk_internal.h"

/*
 *  Property handling
 *
 *  The API exposes only the most common property handling functions.
 *  The caller can invoke Ecmascript built-ins for full control (e.g.
 *  defineProperty, getOwnPropertyDescriptor).
 */

DUK_EXTERNAL duk_bool_t duk_get_prop(duk_context *ctx, duk_idx_t obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property get right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -1);

	rc = duk_hobject_getprop(thr, tv_obj, tv_key);
	DUK_ASSERT(rc == 0 || rc == 1);
	/* a value is left on stack regardless of rc */

	duk_remove(ctx, -2);  /* remove key */
	return rc;  /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_get_prop_string(duk_context *ctx, duk_idx_t obj_index, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	return duk_get_prop(ctx, obj_index);
}

DUK_EXTERNAL duk_bool_t duk_get_prop_index(duk_context *ctx, duk_idx_t obj_index, duk_uarridx_t arr_index) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_uarridx(ctx, arr_index);
	return duk_get_prop(ctx, obj_index);
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(stridx >= 0);
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);
	DUK_UNREF(thr);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_get_prop(ctx, obj_index);
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx_boolean(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_bool_t *out_has_prop) {
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(stridx >= 0);
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);

	rc = duk_get_prop_stridx(ctx, obj_index, stridx);
	if (out_has_prop) {
		*out_has_prop = rc;
	}
	rc = duk_to_boolean(ctx, -1);
	DUK_ASSERT(rc == 0 || rc == 1);
	duk_pop(ctx);
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_put_prop(duk_context *ctx, duk_idx_t obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_tval *tv_val;
	duk_small_int_t throw_flag;
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property put right now (putprop protects
	 * against it internally).
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -2);
	tv_val = duk_require_tval(ctx, -1);
	throw_flag = duk_is_strict_call(ctx);

	rc = duk_hobject_putprop(thr, tv_obj, tv_key, tv_val, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_2(ctx);  /* remove key and value */
	return rc;  /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_put_prop_string(duk_context *ctx, duk_idx_t obj_index, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	duk_swap_top(ctx, -2);  /* [val key] -> [key val] */
	return duk_put_prop(ctx, obj_index);
}

DUK_EXTERNAL duk_bool_t duk_put_prop_index(duk_context *ctx, duk_idx_t obj_index, duk_uarridx_t arr_index) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_uarridx(ctx, arr_index);
	duk_swap_top(ctx, -2);  /* [val key] -> [key val] */
	return duk_put_prop(ctx, obj_index);
}

DUK_INTERNAL duk_bool_t duk_put_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(stridx >= 0);
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);
	DUK_UNREF(thr);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	duk_swap_top(ctx, -2);  /* [val key] -> [key val] */
	return duk_put_prop(ctx, obj_index);
}

DUK_EXTERNAL duk_bool_t duk_del_prop(duk_context *ctx, duk_idx_t obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_small_int_t throw_flag;
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property delete right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -1);
	throw_flag = duk_is_strict_call(ctx);

	rc = duk_hobject_delprop(thr, tv_obj, tv_key, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop(ctx);  /* remove key */
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_del_prop_string(duk_context *ctx, duk_idx_t obj_index, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	return duk_del_prop(ctx, obj_index);
}

DUK_EXTERNAL duk_bool_t duk_del_prop_index(duk_context *ctx, duk_idx_t obj_index, duk_uarridx_t arr_index) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_uarridx(ctx, arr_index);
	return duk_del_prop(ctx, obj_index);
}

DUK_INTERNAL duk_bool_t duk_del_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(stridx >= 0);
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);
	DUK_UNREF(thr);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_del_prop(ctx, obj_index);
}

DUK_EXTERNAL duk_bool_t duk_has_prop(duk_context *ctx, duk_idx_t obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property existence check right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -1);

	rc = duk_hobject_hasprop(thr, tv_obj, tv_key);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop(ctx);  /* remove key */
	return rc;  /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_has_prop_string(duk_context *ctx, duk_idx_t obj_index, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	return duk_has_prop(ctx, obj_index);
}

DUK_EXTERNAL duk_bool_t duk_has_prop_index(duk_context *ctx, duk_idx_t obj_index, duk_uarridx_t arr_index) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_uarridx(ctx, arr_index);
	return duk_has_prop(ctx, obj_index);
}

DUK_INTERNAL duk_bool_t duk_has_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(stridx >= 0);
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);
	DUK_UNREF(thr);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_has_prop(ctx, obj_index);
}

/* Define own property without inheritance looks and such.  This differs from
 * [[DefineOwnProperty]] because special behaviors (like Array 'length') are
 * not invoked by this method.  The caller must be careful to invoke any such
 * behaviors if necessary.
 */
DUK_INTERNAL void duk_xdef_prop(duk_context *ctx, duk_idx_t obj_index, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);
	key = duk_to_hstring(ctx, -2);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_require_tval(ctx, -1) != NULL);

	duk_hobject_define_property_internal(thr, obj, key, desc_flags);

	duk_pop(ctx);  /* pop key */
}

DUK_INTERNAL void duk_xdef_prop_index(duk_context *ctx, duk_idx_t obj_index, duk_uarridx_t arr_index, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);

	duk_hobject_define_property_internal_arridx(thr, obj, arr_index, desc_flags);
	/* value popped by call */
}

DUK_INTERNAL void duk_xdef_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(stridx >= 0);
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);
	key = DUK_HTHREAD_GET_STRING(thr, stridx);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_require_tval(ctx, -1) != NULL);

	duk_hobject_define_property_internal(thr, obj, key, desc_flags);
	/* value popped by call */
}

DUK_INTERNAL void duk_xdef_prop_stridx_builtin(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_int_t builtin_idx, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(stridx >= 0);
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);
	DUK_ASSERT_DISABLE(builtin_idx >= 0);
	DUK_ASSERT(builtin_idx < DUK_NUM_BUILTINS);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);
	key = DUK_HTHREAD_GET_STRING(thr, stridx);
	DUK_ASSERT(key != NULL);

	duk_push_hobject(ctx, thr->builtins[builtin_idx]);
	duk_hobject_define_property_internal(thr, obj, key, desc_flags);
	/* value popped by call */
}

/* This is a rare property helper; it sets the global thrower (E5 Section 13.2.3)
 * setter/getter into an object property.  This is needed by the 'arguments'
 * object creation code, function instance creation code, and Function.prototype.bind().
 */

DUK_INTERNAL void duk_xdef_prop_stridx_thrower(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj = duk_require_hobject(ctx, obj_index);
	duk_hobject *thrower = thr->builtins[DUK_BIDX_TYPE_ERROR_THROWER];
	duk_hobject_define_accessor_internal(thr, obj, DUK_HTHREAD_GET_STRING(thr, stridx), thrower, thrower, desc_flags);
}

/* Object.defineProperty() equivalent C binding. */
DUK_EXTERNAL void duk_def_prop(duk_context *ctx, duk_idx_t obj_index, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t idx_base;
	duk_hobject *obj;
	duk_hstring *key;
	duk_idx_t idx_value;
	duk_hobject *get;
	duk_hobject *set;
	duk_uint_t is_data_desc;
	duk_uint_t is_acc_desc;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_require_hobject(ctx, obj_index);

	is_data_desc = flags & (DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WRITABLE);
	is_acc_desc = flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);
	if (is_data_desc && is_acc_desc) {
		/* "Have" flags must not be conflicting so that they would
		 * apply to both a plain property and an accessor at the same
		 * time.
		 */
		goto fail_invalid_desc;
	}

	idx_base = duk_get_top_index(ctx);
	if (flags & DUK_DEFPROP_HAVE_SETTER) {
		duk_require_type_mask(ctx, idx_base, DUK_TYPE_MASK_UNDEFINED |
		                                     DUK_TYPE_MASK_OBJECT |
		                                     DUK_TYPE_MASK_LIGHTFUNC);
		set = duk_get_hobject_or_lfunc_coerce(ctx, idx_base);
		if (set != NULL && !DUK_HOBJECT_IS_CALLABLE(set)) {
			goto fail_not_callable;
		}
		idx_base--;
	} else {
		set = NULL;
	}
	if (flags & DUK_DEFPROP_HAVE_GETTER) {
		duk_require_type_mask(ctx, idx_base, DUK_TYPE_MASK_UNDEFINED |
		                                     DUK_TYPE_MASK_OBJECT |
		                                     DUK_TYPE_MASK_LIGHTFUNC);
		get = duk_get_hobject_or_lfunc_coerce(ctx, idx_base);
		if (get != NULL && !DUK_HOBJECT_IS_CALLABLE(get)) {
			goto fail_not_callable;
		}
		idx_base--;
	} else {
		get = NULL;
	}
	if (flags & DUK_DEFPROP_HAVE_VALUE) {
		idx_value = idx_base;
		idx_base--;
	} else {
		idx_value = (duk_idx_t) -1;
	}
	key = duk_require_hstring(ctx, idx_base);

	duk_require_valid_index(ctx, idx_base);

	duk_hobject_define_property_helper(ctx,
	                                   flags /*defprop_flags*/,
	                                   obj,
	                                   key,
	                                   idx_value,
	                                   get,
	                                   set);

	/* Clean up stack */

	duk_set_top(ctx, idx_base);

	/* [ ... obj ... ] */

	return;

 fail_invalid_desc:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_INVALID_DESCRIPTOR);
	return;

 fail_not_callable:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_CALLABLE);
	return;
}

/*
 *  Multi-get
 */

DUK_EXTERNAL void duk_get_prop_multi(duk_context *ctx, duk_idx_t index, const char *fmt, ...) {
	duk_hthread *thr;
	va_list ap;
	const char *p;
	const char *p_start_key;
	const char *p_end_key;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	if (!fmt) {
		goto type_error;
	}
	index = duk_require_normalize_index(ctx, index);

	va_start(ap, fmt);
	p = fmt;
	for (;;) {
		char c;

		if (*p == 0) {
			break;
		}

		/* scan key and colon */
		p_start_key = p;
		for (;;) {
			/* FIXME: space not required, but allowed; document this?
			 * this now allows e.g. "foo:dbar:u".
			 */
			c = *p++;
			if (c == DUK_ASC_SPACE) {
				p_start_key = p;
			} else {
				break;
			}
		}
		for (;;) {
			c = *p++;
			if (c == DUK_ASC_COLON) {
				p_end_key = p - 1;
				break;
			} else if (c == 0) {
				goto type_error;
			} else {
				;
			}
		}
		duk_push_lstring(ctx, p_start_key, (duk_size_t) (p_end_key - p_start_key));
		DUK_D(DUK_DPRINT("multi get key is %!T", duk_get_tval(ctx, -1)));
		duk_get_prop(ctx, index);  /* [ ... key ] -> [ ... val ] */

		c = *p++;
		DUK_D(DUK_DPRINT("type char is '%c', value is %!T, index is %ld", (char) c, duk_get_tval(ctx, -1), (long) index));
		switch ((int) c) {
		case DUK_ASC_LC_D:
		case DUK_ASC_LC_U: {
			/* The only difference between duk_int_t and duk_uint_t
			 * is sign, so this works and saves code footprint.
			 */
			duk_int_t *out_v;
			out_v = va_arg(ap, duk_int_t *);
			if (!out_v) {
				goto type_error;
			}
			*out_v = (c == DUK_ASC_LC_D ? duk_to_int(ctx, -1) : (duk_int_t) duk_to_uint(ctx, -1));
			break;
		}
		case DUK_ASC_LC_N: {
			duk_double_t *out_v;
			out_v = va_arg(ap, duk_double_t *);
			if (!out_v) {
				goto type_error;
			}
			*out_v = duk_to_number(ctx, -1);
			break;
		}
		case DUK_ASC_LC_B: {
			duk_bool_t *out_v;
			out_v = va_arg(ap, duk_bool_t *);
			if (!out_v) {
				goto type_error;
			}
			*out_v = duk_to_boolean(ctx, -1);
			break;
		}
		case DUK_ASC_LC_S: {
			/* FIXME: dangerous if string is returned by a getter! */
			const char **out_v;
			out_v = va_arg(ap, const char **);
			if (!out_v) {
				goto type_error;
			}
			*out_v = duk_to_string(ctx, -1);
			break;
		}
		case DUK_ASC_LC_P: {
			void **out_v;
			out_v = va_arg(ap, void **);
			if (!out_v) {
				goto type_error;
			}
			*out_v = duk_to_pointer(ctx, -1);
			break;
		}
		case DUK_ASC_LC_X: {
			/* FIXME: dangerous if buffer is returned by a getter! */
			void **out_v1;
			duk_size_t *out_v2;
			out_v1 = va_arg(ap, void **);
			out_v2 = va_arg(ap, duk_size_t *);
			if (!out_v1 || !out_v2) {
				goto type_error;
			}
			*out_v1 = duk_to_buffer(ctx, -1, out_v2);
			break;
		}
		case DUK_ASC_LC_V: {
			/* Leave value on value stack. */
			duk_require_stack(ctx, 1);  /* get space for next */
			continue;
		}
		default: {
			goto type_error;
		}
		}

		duk_pop(ctx);
	}
	va_end(ap);
	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "FIXME: arg/format error");
}

/*
 *  Object related
 *
 *  Note: seal() and freeze() are accessible through Ecmascript bindings,
 *  and are not exposed through the API.
 */

DUK_EXTERNAL void duk_compact(duk_context *ctx, duk_idx_t obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_get_hobject(ctx, obj_index);
	if (obj) {
		/* Note: this may fail, caller should protect the call if necessary */
		duk_hobject_compact_props(thr, obj);
	}
}

/* XXX: the duk_hobject_enum.c stack APIs should be reworked */

DUK_EXTERNAL void duk_enum(duk_context *ctx, duk_idx_t obj_index, duk_uint_t enum_flags) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_dup(ctx, obj_index);
	duk_require_hobject_or_lfunc_coerce(ctx, -1);
	duk_hobject_enumerator_create(ctx, enum_flags);   /* [target] -> [enum] */
}

DUK_EXTERNAL duk_bool_t duk_next(duk_context *ctx, duk_idx_t enum_index, duk_bool_t get_value) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_require_hobject(ctx, enum_index);
	duk_dup(ctx, enum_index);
	return duk_hobject_enumerator_next(ctx, get_value);
}

/*
 *  Helpers for writing multiple properties
 */

DUK_EXTERNAL void duk_put_function_list(duk_context *ctx, duk_idx_t obj_index, const duk_function_list_entry *funcs) {
	const duk_function_list_entry *ent = funcs;

	DUK_ASSERT_CTX_VALID(ctx);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	if (ent != NULL) {
		while (ent->key != NULL) {
			duk_push_c_function(ctx, ent->value, ent->nargs);
			duk_put_prop_string(ctx, obj_index, ent->key);
			ent++;
		}
	}
}

DUK_EXTERNAL void duk_put_number_list(duk_context *ctx, duk_idx_t obj_index, const duk_number_list_entry *numbers) {
	const duk_number_list_entry *ent = numbers;

	DUK_ASSERT_CTX_VALID(ctx);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	if (ent != NULL) {
		while (ent->key != NULL) {
			duk_push_number(ctx, ent->value);
			duk_put_prop_string(ctx, obj_index, ent->key);
			ent++;
		}
	}
}

/*
 *  Shortcut for accessing global object properties
 */

DUK_EXTERNAL duk_bool_t duk_get_global_string(duk_context *ctx, const char *key) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_bool_t ret;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(ctx, thr->builtins[DUK_BIDX_GLOBAL]);
	ret = duk_get_prop_string(ctx, -1, key);
	duk_remove(ctx, -2);
	return ret;
}

DUK_EXTERNAL duk_bool_t duk_put_global_string(duk_context *ctx, const char *key) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_bool_t ret;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(ctx, thr->builtins[DUK_BIDX_GLOBAL]);
	duk_insert(ctx, -2);
	ret = duk_put_prop_string(ctx, -2, key);  /* [ ... global val ] -> [ ... global ] */
	duk_pop(ctx);
	return ret;
}

/*
 *  Object prototype
 */

DUK_EXTERNAL void duk_get_prototype(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hobject *proto;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	obj = duk_require_hobject(ctx, index);
	DUK_ASSERT(obj != NULL);

	/* XXX: shared helper for duk_push_hobject_or_undefined()? */
	proto = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, obj);
	if (proto) {
		duk_push_hobject(ctx, proto);
	} else {
		duk_push_undefined(ctx);
	}
}

DUK_EXTERNAL void duk_set_prototype(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hobject *proto;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_require_hobject(ctx, index);
	DUK_ASSERT(obj != NULL);
	duk_require_type_mask(ctx, -1, DUK_TYPE_MASK_UNDEFINED |
	                               DUK_TYPE_MASK_OBJECT);
	proto = duk_get_hobject(ctx, -1);
	/* proto can also be NULL here (allowed explicitly) */

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_CONFIGURABLE);  /* XXX: "read only object"? */
		return;
	}
#endif

	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, obj, proto);

	duk_pop(ctx);
}

/*
 *  Object finalizer
 */

/* XXX: these could be implemented as macros calling an internal function
 * directly.
 * XXX: same issue as with Duktape.fin: there's no way to delete the property
 * now (just set it to undefined).
 */
DUK_EXTERNAL void duk_get_finalizer(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_get_prop_stridx(ctx, index, DUK_STRIDX_INT_FINALIZER);
}

DUK_EXTERNAL void duk_set_finalizer(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_put_prop_stridx(ctx, index, DUK_STRIDX_INT_FINALIZER);
}
