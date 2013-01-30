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

int duk_get_prop(duk_context *ctx, int obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	int rc;

	DUK_ASSERT(ctx != NULL);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property get right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -1);

	rc = duk_hobject_getprop(thr, tv_obj, tv_key);
	/* a value is left on stack regardless of rc */

	duk_remove(ctx, -2);  /* remove key */
	return rc;  /* 1 if property found, 0 otherwise */
}

int duk_get_prop_string(duk_context *ctx, int obj_index, const char *key) {
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	return duk_get_prop(ctx, obj_index);
}

int duk_get_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index) {
	DUK_ASSERT(ctx != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_number(ctx, (double) arr_index);
	return duk_get_prop(ctx, obj_index);
}

int duk_get_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, thr->strs[stridx]);
	return duk_get_prop(ctx, obj_index);
}

/* FIXME: checked variant? */
int duk_put_prop(duk_context *ctx, int obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_tval *tv_val;
	int throw_flag = 0;
	int rc;

	DUK_ASSERT(ctx != NULL);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property put right now (putprop protects
	 * against it internally).
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -2);
	tv_val = duk_require_tval(ctx, -1);
	throw_flag = 0;  /* 0 = don't throw */

	rc = duk_hobject_putprop(thr, tv_obj, tv_key, tv_val, throw_flag);

	duk_pop_2(ctx);  /* remove key and value */
	return rc;  /* 1 if property found, 0 otherwise */
}

int duk_put_prop_string(duk_context *ctx, int obj_index, const char *key) {
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	duk_swap_top(ctx, -2);  /* [val key] -> [key val] */
	return duk_put_prop(ctx, obj_index);
}

int duk_put_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index) {
	DUK_ASSERT(ctx != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_number(ctx, (double) arr_index);
	duk_swap_top(ctx, -2);  /* [val key] -> [key val] */
	return duk_put_prop(ctx, obj_index);
}

int duk_put_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, thr->strs[stridx]);
	duk_swap_top(ctx, -2);  /* [val key] -> [key val] */
	return duk_put_prop(ctx, obj_index);
}

/* FIXME: checked variant? */
int duk_del_prop(duk_context *ctx, int obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	int throw_flag;
	int rc;

	DUK_ASSERT(ctx != NULL);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property delete right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -1);
	throw_flag = 0;  /* 0 = don't throw */

	rc = duk_hobject_delprop(thr, tv_obj, tv_key, throw_flag);

	duk_pop(ctx);  /* remove key */
	return rc;
}

int duk_del_prop_string(duk_context *ctx, int obj_index, const char *key) {
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	return duk_del_prop(ctx, obj_index);
}

int duk_del_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index) {
	DUK_ASSERT(ctx != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_number(ctx, (double) arr_index);
	return duk_del_prop(ctx, obj_index);
}

int duk_del_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, thr->strs[stridx]);
	return duk_del_prop(ctx, obj_index);
}

int duk_has_prop(duk_context *ctx, int obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	int rc;

	DUK_ASSERT(ctx != NULL);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property existence check right now.
	 */

	tv_obj = duk_require_tval(ctx, obj_index);
	tv_key = duk_require_tval(ctx, -1);

	rc = duk_hobject_hasprop(thr, tv_obj, tv_key);

	duk_pop(ctx);  /* remove key */
	return rc;  /* 1 if property found, 0 otherwise */
}

int duk_has_prop_string(duk_context *ctx, int obj_index, const char *key) {
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(key != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_string(ctx, key);
	return duk_has_prop(ctx, obj_index);
}

int duk_has_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index) {
	DUK_ASSERT(ctx != NULL);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_number(ctx, (double) arr_index);
	return duk_has_prop(ctx, obj_index);
}

int duk_has_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);

	obj_index = duk_require_normalize_index(ctx, obj_index);
	duk_push_hstring(ctx, thr->strs[stridx]);
	return duk_has_prop(ctx, obj_index);
}

void duk_def_prop(duk_context *ctx, int obj_index, int desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT(ctx != NULL);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);
	key = duk_to_hstring(ctx, -2);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_require_tval(ctx, -1) != NULL);

	duk_hobject_define_property_internal(thr, obj, key, desc_flags);

	duk_pop(ctx);  /* pop key */
}

void duk_def_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx, int desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);
	key = thr->strs[stridx];
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_require_tval(ctx, -1) != NULL);

	duk_hobject_define_property_internal(thr, obj, key, desc_flags);

	/* value already popped */
}

void duk_def_prop_stridx_builtin(duk_context *ctx, int obj_index, unsigned int stridx, unsigned int builtin_idx, int desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);
	DUK_ASSERT(builtin_idx >= 0 && builtin_idx < DUK_NUM_BUILTINS);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);
	key = thr->strs[stridx];
	DUK_ASSERT(key != NULL);

	duk_push_hobject(ctx, thr->builtins[builtin_idx]);
	duk_hobject_define_property_internal(thr, obj, key, desc_flags);
	/* value popped by call */
}

/* This is a rare property helper; it sets the global thrower (E5 Section 13.2.3)
 * setter/getter into an object property.  Since there are so few places where
 * accessor properties are created (by the implementation), there are almost no
 * other API calls for creating such properties (except calling Object.defineProperty()
 * properly).
 *
 * This is needed by the 'arguments' object creation code.
 */

void duk_def_prop_stridx_thrower(duk_context *ctx, int obj_index, unsigned int stridx, int desc_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_hobject *thrower;
	int e_idx;
	int h_idx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);

	/*
	 *  Since we don't have an internal define function for creating
	 *  accessor values, first set the value to 'undefined', then get
	 *  the storage slot and update in-place to an accessor.
	 */

	duk_push_undefined(ctx);
	duk_def_prop_stridx(ctx, obj_index, stridx, desc_flags);

	obj = duk_require_hobject(ctx, obj_index);
	duk_hobject_find_existing_entry(obj, DUK_HTHREAD_GET_STRING(thr, stridx), &e_idx, &h_idx);
	DUK_ASSERT(e_idx >= 0 && e_idx < obj->e_used);

	/* no need to decref, as previous value is 'undefined' */
	thrower = thr->builtins[DUK_BIDX_TYPE_ERROR_THROWER];
	DUK_HOBJECT_E_SLOT_SET_ACCESSOR(obj, e_idx);
	DUK_HOBJECT_E_SET_VALUE_GETTER(obj, e_idx, thrower);
	DUK_HOBJECT_E_SET_VALUE_SETTER(obj, e_idx, thrower);
	DUK_HOBJECT_INCREF(thr, thrower);
	DUK_HOBJECT_INCREF(thr, thrower);  /* XXX: macro to increment a count directly */
}

/*
 *  Object related
 *
 *  Note: seal() and freeze() are accessible through Ecmascript bindings,
 *  and are not exposed through the API.
 */

void duk_compact(duk_context *ctx, int obj_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;

	DUK_ASSERT(ctx != NULL);

	obj = duk_require_hobject(ctx, obj_index);
	DUK_ASSERT(obj != NULL);

	/* Note: this may fail, caller should protect the call if necessary */
	duk_hobject_compact_props(thr, obj);
}

/* FIXME: the duk_hobject_enum.c stack APIs should be reworked */

void duk_enum(duk_context *ctx, int obj_index, int enum_flags) {
	DUK_ASSERT(ctx != NULL);

	duk_require_hobject(ctx, obj_index);
	duk_dup(ctx, obj_index);
	duk_hobject_enumerator_create(ctx, enum_flags);   /* [target] -> [target enum] */
	duk_remove(ctx, -2);                              /* [target enum] -> [enum] */
}

int duk_next(duk_context *ctx, int enum_index, int get_value) {
	duk_require_hobject(ctx, enum_index);
	duk_dup(ctx, enum_index);
	duk_hobject_enumerator_next(ctx);     /* [enum] -> [key] */

	/* FIXME: also get the value */

	return !duk_is_undefined(ctx, -1);
}


