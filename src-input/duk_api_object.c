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

DUK_EXTERNAL duk_bool_t duk_get_prop(duk_context *ctx, duk_idx_t obj_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property get right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_idx);
	tv_key = duk_require_tval(ctx, -1);

	rc = duk_hobject_getprop(thr, tv_obj, tv_key);
	DUK_ASSERT(rc == 0 || rc == 1);
	/* a value is left on stack regardless of rc */

	duk_remove_m2(ctx);  /* remove key */
	return rc;  /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_get_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_string(ctx, key);
	return duk_get_prop(ctx, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_get_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_lstring(ctx, key, key_len);
	return duk_get_prop(ctx, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_get_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_uarridx(ctx, arr_idx);
	return duk_get_prop(ctx, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_STRIDX_VALID(stridx);
	DUK_UNREF(thr);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_get_prop(ctx, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx_short_raw(duk_context *ctx, duk_uint_t packed_args) {
	return duk_get_prop_stridx(ctx, (duk_idx_t) (duk_int16_t) (packed_args >> 16),
	                                (duk_small_uint_t) (packed_args & 0xffffUL));
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx_boolean(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx, duk_bool_t *out_has_prop) {
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_STRIDX_VALID(stridx);

	rc = duk_get_prop_stridx(ctx, obj_idx, stridx);
	if (out_has_prop) {
		*out_has_prop = rc;
	}
	rc = duk_to_boolean(ctx, -1);
	DUK_ASSERT(rc == 0 || rc == 1);
	duk_pop(ctx);
	return rc;
}

DUK_LOCAL duk_bool_t duk__put_prop_shared(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t idx_key) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_tval *tv_val;
	duk_small_int_t throw_flag;
	duk_bool_t rc;

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property put right now (putprop protects
	 * against it internally).
	 */

	/* Key and value indices are either (-2, -1) or (-1, -2).  Given idx_key,
	 * idx_val is always (idx_key ^ 0x01).
	 */
	DUK_ASSERT((idx_key == -2 && (idx_key ^ 1) == -1) ||
	           (idx_key == -1 && (idx_key ^ 1) == -2));
	/* XXX: Direct access; faster validation. */
	tv_obj = duk_require_tval(ctx, obj_idx);
	tv_key = duk_require_tval(ctx, idx_key);
	tv_val = duk_require_tval(ctx, idx_key ^ 1);
	throw_flag = duk_is_strict_call(ctx);

	rc = duk_hobject_putprop(thr, tv_obj, tv_key, tv_val, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_2(ctx);  /* remove key and value */
	return rc;  /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_put_prop(duk_context *ctx, duk_idx_t obj_idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__put_prop_shared(ctx, obj_idx, -2);
}

DUK_EXTERNAL duk_bool_t duk_put_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	/* Careful here and with other duk_put_prop_xxx() helpers: the
	 * target object and the property value may be in the same value
	 * stack slot (unusual, but still conceptually clear).
	 */
	obj_idx = duk_normalize_index(ctx, obj_idx);
	(void) duk_push_string(ctx, key);
	return duk__put_prop_shared(ctx, obj_idx, -1);
}

DUK_EXTERNAL duk_bool_t duk_put_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_normalize_index(ctx, obj_idx);
	(void) duk_push_lstring(ctx, key, key_len);
	return duk__put_prop_shared(ctx, obj_idx, -1);
}

DUK_EXTERNAL duk_bool_t duk_put_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_uarridx(ctx, arr_idx);
	return duk__put_prop_shared(ctx, obj_idx, -1);
}

DUK_INTERNAL duk_bool_t duk_put_prop_stridx(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_STRIDX_VALID(stridx);
	DUK_UNREF(thr);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk__put_prop_shared(ctx, obj_idx, -1);
}

DUK_INTERNAL duk_bool_t duk_put_prop_stridx_short_raw(duk_context *ctx, duk_uint_t packed_args) {
	return duk_put_prop_stridx(ctx, (duk_idx_t) (duk_int16_t) (packed_args >> 16),
	                                (duk_small_uint_t) (packed_args & 0xffffUL));
}

DUK_EXTERNAL duk_bool_t duk_del_prop(duk_context *ctx, duk_idx_t obj_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_small_int_t throw_flag;
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property delete right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_idx);
	tv_key = duk_require_tval(ctx, -1);
	throw_flag = duk_is_strict_call(ctx);

	rc = duk_hobject_delprop(thr, tv_obj, tv_key, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop(ctx);  /* remove key */
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_del_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_string(ctx, key);
	return duk_del_prop(ctx, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_del_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_lstring(ctx, key, key_len);
	return duk_del_prop(ctx, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_del_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_uarridx(ctx, arr_idx);
	return duk_del_prop(ctx, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_del_prop_stridx(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_STRIDX_VALID(stridx);
	DUK_UNREF(thr);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_del_prop(ctx, obj_idx);
}

#if 0
DUK_INTERNAL duk_bool_t duk_del_prop_stridx_short_raw(duk_context *ctx, duk_uint_t packed_args) {
	return duk_del_prop_stridx(ctx, (duk_idx_t) (duk_int16_t) (packed_args >> 16),
	                                (duk_small_uint_t) (packed_args & 0xffffUL));
}
#endif

DUK_EXTERNAL duk_bool_t duk_has_prop(duk_context *ctx, duk_idx_t obj_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_bool_t rc;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property existence check right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_idx);
	tv_key = duk_require_tval(ctx, -1);

	rc = duk_hobject_hasprop(thr, tv_obj, tv_key);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop(ctx);  /* remove key */
	return rc;  /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_has_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_string(ctx, key);
	return duk_has_prop(ctx, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_has_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_lstring(ctx, key, key_len);
	return duk_has_prop(ctx, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_has_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_uarridx(ctx, arr_idx);
	return duk_has_prop(ctx, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_has_prop_stridx(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_STRIDX_VALID(stridx);
	DUK_UNREF(thr);

	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_has_prop(ctx, obj_idx);
}

#if 0
DUK_INTERNAL duk_bool_t duk_has_prop_stridx_short_raw(duk_context *ctx, duk_uint_t packed_args) {
	return duk_has_prop_stridx(ctx, (duk_idx_t) (duk_int16_t) (packed_args >> 16),
	                                (duk_small_uint_t) (packed_args & 0xffffUL));
}
#endif

/* Define own property without inheritance lookups and such.  This differs from
 * [[DefineOwnProperty]] because special behaviors (like Array 'length') are
 * not invoked by this method.  The caller must be careful to invoke any such
 * behaviors if necessary.
 */
DUK_INTERNAL void duk_xdef_prop(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_require_hobject(ctx, obj_idx);
	DUK_ASSERT(obj != NULL);
	key = duk_to_hstring(ctx, -2);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_require_tval(ctx, -1) != NULL);

	duk_hobject_define_property_internal(thr, obj, key, desc_flags);

	duk_pop(ctx);  /* pop key */
}

DUK_INTERNAL void duk_xdef_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_require_hobject(ctx, obj_idx);
	DUK_ASSERT(obj != NULL);

	duk_hobject_define_property_internal_arridx(thr, obj, arr_idx, desc_flags);
	/* value popped by call */
}

DUK_INTERNAL void duk_xdef_prop_stridx(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_STRIDX_VALID(stridx);

	obj = duk_require_hobject(ctx, obj_idx);
	DUK_ASSERT(obj != NULL);
	key = DUK_HTHREAD_GET_STRING(thr, stridx);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_require_tval(ctx, -1) != NULL);

	duk_hobject_define_property_internal(thr, obj, key, desc_flags);
	/* value popped by call */
}

DUK_INTERNAL void duk_xdef_prop_stridx_short_raw(duk_context *ctx, duk_uint_t packed_args) {
	duk_xdef_prop_stridx(ctx, (duk_idx_t) (duk_int8_t) (packed_args >> 24),
	                          (duk_small_uint_t) (packed_args >> 8) & 0xffffUL,
	                          (duk_small_uint_t) (packed_args & 0xffL));
}

DUK_INTERNAL void duk_xdef_prop_stridx_builtin(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx, duk_small_int_t builtin_idx, duk_small_uint_t desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_STRIDX_VALID(stridx);
	DUK_ASSERT_BIDX_VALID(builtin_idx);

	obj = duk_require_hobject(ctx, obj_idx);
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

DUK_INTERNAL void duk_xdef_prop_stridx_thrower(duk_context *ctx, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	obj_idx = duk_require_normalize_index(ctx, obj_idx);
	duk_push_hstring_stridx(ctx, stridx);
	duk_push_hobject_bidx(ctx, DUK_BIDX_TYPE_ERROR_THROWER);
	duk_dup_top(ctx);
	duk_def_prop(ctx, obj_idx, DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_FORCE);  /* attributes always 0 */
}

/* Object.getOwnPropertyDescriptor() equivalent C binding. */
DUK_EXTERNAL void duk_get_prop_desc(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags) {
	DUK_UNREF(flags);  /* no flags defined yet */

	duk_hobject_object_get_own_property_descriptor(ctx, obj_idx);  /* [ ... key ] -> [ ... desc ] */
}

/* Object.defineProperty() equivalent C binding. */
DUK_EXTERNAL void duk_def_prop(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags) {
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

	obj = duk_require_hobject(ctx, obj_idx);

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
		set = duk_get_hobject_promote_lfunc(ctx, idx_base);
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
		get = duk_get_hobject_promote_lfunc(ctx, idx_base);
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
	key = duk_to_hstring(ctx, idx_base);  /* XXX: later should accept symbols */
	DUK_ASSERT(key != NULL);

	duk_require_valid_index(ctx, idx_base);

	duk_hobject_define_property_helper(ctx,
	                                   flags /*defprop_flags*/,
	                                   obj,
	                                   key,
	                                   idx_value,
	                                   get,
	                                   set,
	                                   1 /*throw_flag*/);

	/* Clean up stack */

	duk_set_top(ctx, idx_base);

	/* [ ... obj ... ] */

	return;

 fail_invalid_desc:
	DUK_ERROR_TYPE(thr, DUK_STR_INVALID_DESCRIPTOR);
	return;

 fail_not_callable:
	DUK_ERROR_TYPE(thr, DUK_STR_NOT_CALLABLE);
	return;
}

/*
 *  Object related
 *
 *  Note: seal() and freeze() are accessible through Ecmascript bindings,
 *  and are not exposed through the API.
 */

DUK_EXTERNAL void duk_compact(duk_context *ctx, duk_idx_t obj_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_get_hobject(ctx, obj_idx);
	if (obj) {
		/* Note: this may fail, caller should protect the call if necessary */
		duk_hobject_compact_props(thr, obj);
	}
}

DUK_INTERNAL void duk_compact_m1(duk_context *ctx) {
	duk_compact(ctx, -1);
}

/* XXX: the duk_hobject_enum.c stack APIs should be reworked */

DUK_EXTERNAL void duk_enum(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t enum_flags) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_dup(ctx, obj_idx);
	duk_require_hobject_promote_mask(ctx, -1, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
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

#if defined(DUK_USE_UNION_INITIALIZERS)
#define DUK__PROP_SELECT(unionval,structval) (unionval)
#else
#define DUK__PROP_SELECT(unionval,structval) (structval)
#endif

DUK_EXTERNAL void duk_def_prop_list(duk_context *ctx, duk_idx_t obj_index, const duk_prop_list_entry *props) {
	const duk_prop_list_entry *ent = props;
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT_CTX_VALID(ctx);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(ctx);
#endif

	obj_index = duk_require_normalize_index(ctx, obj_index);
	for (;;) {
		/* FIXME: optimize by using DUK_TVAL_SET_xxx() */
		duk_uint_t defprop_flags;

#if defined(DUK_USE_ASSERTIONS)
		DUK_ASSERT_TOP(ctx, entry_top);
#endif
		if (ent->key == NULL) {
			DUK_ASSERT(ent->etype == DUK_PROPINIT_END);
			break;
		}
		duk_push_string(ctx, ent->key);

		defprop_flags = ent->eattr;  /* these are assumed to be correct, up to the duk_api_public.h macros */

		DUK_ASSERT(ent->etype != DUK_PROPINIT_END);  /* key == NULL <=> DUK_PROPINIT_END */
		switch (ent->etype) {
		case DUK_PROPINIT_UNDEFINED: {
			duk_push_undefined(ctx);
			break;
		}
		case DUK_PROPINIT_NULL: {
			duk_push_null(ctx);
			break;
		}
		case DUK_PROPINIT_BOOLEAN: {
			duk_push_boolean(ctx, DUK__PROP_SELECT(ent->u.bval, ent->s.ival));
			break;
		}
		case DUK_PROPINIT_NUMBER: {
			duk_push_number(ctx, DUK__PROP_SELECT(ent->u.num, ent->s.num));
			break;
		}
		case DUK_PROPINIT_STRING: {
			duk_push_string(ctx, (const char *) DUK__PROP_SELECT(ent->u.str.str, ent->s.ptr));
			break;
		}
		case DUK_PROPINIT_LSTRING: {
			duk_push_lstring(ctx,
			                 (const char *) DUK__PROP_SELECT(ent->u.str.str, ent->s.ptr),
			                 (duk_size_t) DUK__PROP_SELECT(ent->u.str.len, ent->s.idx));
			break;
		}
		case DUK_PROPINIT_OBJECT: {
			DUK_ERROR_INTERNAL((duk_hthread *) ctx);  /* FIXME: unimplemented */
			break;
		}
		case DUK_PROPINIT_BUFFER: {
			/* FIXME: buffer nature: fixed/dynamic */
			/* FIXME: initialization? */
			/* FIXME: buffer objects */
			void *bufdata;
			const void *initdata;
			duk_size_t buflen;
			buflen = (duk_size_t) DUK__PROP_SELECT(ent->u.str.len, ent->s.idx);
			bufdata = duk_push_fixed_buffer(ctx, buflen);
			DUK_ASSERT(bufdata != NULL);
			initdata = (const void *) DUK__PROP_SELECT(ent->u.buf.buf, ent->s.ptr);
			if (initdata) {
				DUK_MEMCPY(bufdata, initdata, buflen);
			}
			break;
		}
		case DUK_PROPINIT_POINTER: {
			/* FIXME: lose const for struct case */
			duk_push_pointer(ctx, (void *) DUK__PROP_SELECT(ent->u.ptr, ent->s.ptr));
			break;
		}
		case DUK_PROPINIT_FUNCTION: {
			duk_push_c_function(ctx,
			                    DUK__PROP_SELECT(ent->u.func.func, ent->s.func1),
			                    DUK__PROP_SELECT(ent->u.func.nargs, ent->s.idx));
			duk_set_magic(ctx, -1, DUK__PROP_SELECT(ent->u.func.magic, ent->s.ival));
			/* FIXME: 'length' for a non-lightfunc? */
			break;
		}
		case DUK_PROPINIT_LIGHTFUNC: {
			duk_push_c_lightfunc(ctx,
			                    DUK__PROP_SELECT(ent->u.func.func, ent->s.func1),
			                    DUK__PROP_SELECT(ent->u.func.nargs, ent->s.idx),
			                    DUK__PROP_SELECT(ent->u.func.length, (duk_idx_t) ent->s.num),
			                    DUK__PROP_SELECT(ent->u.func.magic, (duk_int_t) ent->s.ival));
			break;
		}
		case DUK_PROPINIT_ACCESSOR: {
			/* FIXME: getter/setter nargs; provided by user or not?
			 * Should the default be sensitive to custom key argument presence?
			 */
			/* FIXME: tolerating NULL? */
			duk_push_c_function(ctx,
			                    DUK__PROP_SELECT(ent->u.accessor.getter, ent->s.func1),
			                    1 /*nargs*/);
			duk_push_c_function(ctx,
			                    DUK__PROP_SELECT(ent->u.accessor.setter, ent->s.func2),
			                    2 /*nargs*/);
			DUK_ASSERT(defprop_flags & DUK_DEFPROP_HAVE_GETTER);
			DUK_ASSERT(defprop_flags & DUK_DEFPROP_HAVE_SETTER);
			break;
		}
		case DUK_PROPINIT_PROPLIST: {
			const duk_prop_list_entry *sub_props;
			duk_push_object(ctx);
			sub_props = (const duk_prop_list_entry *) DUK__PROP_SELECT(ent->u.props, ent->s.ptr);
			DUK_ASSERT(sub_props != NULL);
			duk_require_stack(ctx, 1);  /* this matters for deep recursion */
			duk_def_prop_list(ctx, -1, sub_props);
			break;
		}
		case DUK_PROPINIT_STACK: {
			DUK_ERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);  /* FIXME */
			break;
		}
		default: {
			DUK_ERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
		}
		}  /* switch */

		duk_def_prop(ctx, obj_index, defprop_flags);
		ent++;
	}

	/* Because this call is generally used to initialize objects, it's
	 * useful to automatically compact the object here.
	 */
	duk_compact(ctx, obj_index);

#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT_TOP(ctx, entry_top);
#endif
}
#undef DUK__PROP_SELECT

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
	duk_remove_m2(ctx);
	return ret;
}

DUK_EXTERNAL duk_bool_t duk_get_global_lstring(duk_context *ctx, const char *key, duk_size_t key_len) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_bool_t ret;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(ctx, thr->builtins[DUK_BIDX_GLOBAL]);
	ret = duk_get_prop_lstring(ctx, -1, key, key_len);
	duk_remove_m2(ctx);
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

DUK_EXTERNAL duk_bool_t duk_put_global_lstring(duk_context *ctx, const char *key, duk_size_t key_len) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_bool_t ret;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(ctx, thr->builtins[DUK_BIDX_GLOBAL]);
	duk_insert(ctx, -2);
	ret = duk_put_prop_lstring(ctx, -2, key, key_len);  /* [ ... global val ] -> [ ... global ] */
	duk_pop(ctx);
	return ret;
}

/*
 *  Object prototype
 */

DUK_EXTERNAL void duk_get_prototype(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hobject *proto;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	obj = duk_require_hobject(ctx, idx);
	DUK_ASSERT(obj != NULL);

	/* XXX: shared helper for duk_push_hobject_or_undefined()? */
	proto = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, obj);
	if (proto) {
		duk_push_hobject(ctx, proto);
	} else {
		duk_push_undefined(ctx);
	}
}

DUK_EXTERNAL void duk_set_prototype(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hobject *proto;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_require_hobject(ctx, idx);
	DUK_ASSERT(obj != NULL);
	duk_require_type_mask(ctx, -1, DUK_TYPE_MASK_UNDEFINED |
	                               DUK_TYPE_MASK_OBJECT);
	proto = duk_get_hobject(ctx, -1);
	/* proto can also be NULL here (allowed explicitly) */

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj)) {
		DUK_ERROR_TYPE(thr, DUK_STR_NOT_CONFIGURABLE);  /* XXX: "read only object"? */
		return;
	}
#endif

	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, obj, proto);

	duk_pop(ctx);
}

/*
 *  Object finalizer
 */

#if defined(DUK_USE_FINALIZER_SUPPORT)
/* XXX: these could be implemented as macros calling an internal function
 * directly.
 * XXX: same issue as with Duktape.fin: there's no way to delete the property
 * now (just set it to undefined).
 */
DUK_EXTERNAL void duk_get_finalizer(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_get_prop_stridx(ctx, idx, DUK_STRIDX_INT_FINALIZER);
}

DUK_EXTERNAL void duk_set_finalizer(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_put_prop_stridx(ctx, idx, DUK_STRIDX_INT_FINALIZER);
}
#else  /* DUK_USE_FINALIZER_SUPPORT */
DUK_EXTERNAL void duk_get_finalizer(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(idx);
	DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
}

DUK_EXTERNAL void duk_set_finalizer(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(idx);
	DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
}
#endif  /* DUK_USE_FINALIZER_SUPPORT */
