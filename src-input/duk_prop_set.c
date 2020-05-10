/*
 *  [[Set]] and PutValue() for properties
 *
 *  The specification [[Set]] algorithm is tail recursive, progressing
 *  from target to target up the inheritance chain.  Except for special
 *  cases like getters and Proxies, once an inherited property is found
 *  or the inheritance chain ends, the final property write goes to the
 *  original receiver.
 *
 *  To avoid a mandatory dependence on the compiler optimizing tail calls
 *  (which is not guaranteed), the algorithm here is reworked into a "check"
 *  phase doing the prototype walk without recursion, and a "final" phase
 *  doing the actual property write on the receiver when the [[Set]]
 *  algorithm terminates.  Some cases are handled inline in the "check" phase.
 *  A tail recursive implementation might be preferable because it avoids
 *  the overhead of returning and checking a return code for each step, but
 *  it would need to depend on a config option.
 *
 *  As for [[Get]], side effects are difficult to handle correctly, especially
 *  for Proxies and Arguments objects.
 */

#include "duk_internal.h"

/* Outcome for the "check" phase of [[Set]]: property found (= proceed),
 * not found (= continue lookup), handled inline as success or failure,
 * or special handling (Proxy and Arguments).
 */
#define DUK__SETCHECK_NOTFOUND           0  /* property not found (continue prototype walk) */
#define DUK__SETCHECK_FOUND              1  /* property found and is writable */
#define DUK__SETCHECK_DONE_FAILURE       2  /* handled inline, failure result */
#define DUK__SETCHECK_DONE_SUCCESS       3  /* handled inline, success result */
#define DUK__SETCHECK_HANDLE_SPECIAL     4  /* special handling for Proxy and Arguments in "check" loop */

DUK_LOCAL_DECL duk_bool_t duk__prop_set_str_safe(duk_hthread *thr, duk_hobject *target, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag);
DUK_LOCAL_DECL duk_bool_t duk__prop_set_str_unsafe(duk_hthread *thr, duk_hobject *target, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag);
DUK_LOCAL_DECL duk_bool_t duk__prop_set_idx_safe(duk_hthread *thr, duk_hobject *target, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag);
DUK_LOCAL_DECL duk_bool_t duk__prop_set_idx_unsafe(duk_hthread *thr, duk_hobject *target, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag);
DUK_LOCAL_DECL duk_bool_t duk__setfinal_own_prop_idxkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val);
DUK_LOCAL_DECL duk_bool_t duk__setfinal_own_prop_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val);
DUK_LOCAL_DECL duk_bool_t duk__setcheck_own_prop_idxkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag);

DUK_NORETURN(DUK_LOCAL duk_bool_t duk__prop_set_error_objidx_str(duk_hthread *thr, duk_idx_t idx_obj, duk_hstring *key, duk_bool_t throw_flag));
DUK_NORETURN(DUK_LOCAL duk_bool_t duk__prop_set_error_objidx_idx(duk_hthread *thr, duk_idx_t idx_obj, duk_uarridx_t idx, duk_bool_t throw_flag));
DUK_NORETURN(DUK_LOCAL duk_bool_t duk__prop_set_error_objidx_tvkey(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key, duk_bool_t throw_flag));

#if defined(DUK_USE_PARANOID_ERRORS)
DUK_LOCAL duk_bool_t duk__prop_set_error_shared(duk_hthread *thr, duk_idx_t idx_obj, duk_bool_t throw_flag) {
	if (throw_flag) {
		const char *str1 = duk_get_type_name(thr, idx_obj);
		DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "cannot write property of %s", str1);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_set_error_objidx_str(duk_hthread *thr, duk_idx_t idx_obj, duk_hstring *key, duk_bool_t throw_flag) {
	DUK_UNREF(key);
	return duk__prop_set_error_shared(thr, idx_obj, throw_flag);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_set_error_objidx_idx(duk_hthread *thr, duk_idx_t idx_obj, duk_uarridx_t idx, duk_bool_t throw_flag) {
	DUK_UNREF(idx);
	return duk__prop_set_error_shared(thr, idx_obj, throw_flag);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_set_error_objidx_tvkey(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key, duk_bool_t throw_flag) {
	DUK_UNREF(tv_key);
	return duk__prop_set_error_shared(thr, idx_obj, throw_flag);
}
#elif defined(DUK_USE_VERBOSE_ERRORS)
DUK_LOCAL duk_bool_t duk__prop_set_error_objidx_str(duk_hthread *thr, duk_idx_t idx_obj, duk_hstring *key, duk_bool_t throw_flag) {
	if (throw_flag) {
		const char *str1 = duk_push_readable_idx(thr, idx_obj);
		const char *str2 = duk_push_readable_hstring(thr, key);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot write property %s of %s", str2, str1);
	}
	return 0;
}
DUK_LOCAL duk_bool_t duk__prop_set_error_objidx_idx(duk_hthread *thr, duk_idx_t idx_obj, duk_uarridx_t idx, duk_bool_t throw_flag) {
	if (throw_flag) {
		const char *str1 = duk_push_readable_idx(thr, idx_obj);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot write property %lu of %s", (unsigned long) idx, str1);
	}
	return 0;
}
DUK_LOCAL duk_bool_t duk__prop_set_error_objidx_tvkey(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key, duk_bool_t throw_flag) {
	if (throw_flag) {
		const char *str1 = duk_push_readable_idx(thr, idx_obj);
		const char *str2 = duk_push_readable_tval(thr, tv_key);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot write property %s of %s", str2, str1);
	}
	return 0;
}
#else
DUK_LOCAL duk_bool_t duk__prop_set_error_shared(duk_hthread *thr, duk_idx_t idx_obj, duk_bool_t throw_flag) {
	if (throw_flag) {
		DUK_ERROR_TYPE(thr, DUK_STR_CANNOT_WRITE_PROPERTY);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_set_error_objidx_str(duk_hthread *thr, duk_idx_t idx_obj, duk_hstring *key, duk_bool_t throw_flag) {
	DUK_UNREF(key);
	return duk__prop_set_error_shared(thr, idx_obj, throw_flag);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_set_error_objidx_idx(duk_hthread *thr, duk_idx_t idx_obj, duk_uarridx_t idx, duk_bool_t throw_flag) {
	DUK_UNREF(idx);
	return duk__prop_set_error_shared(thr, idx_obj, throw_flag);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_set_error_objidx_tvkey(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key, duk_bool_t throw_flag) {
	DUK_UNREF(tv_key);
	return duk__prop_set_error_shared(thr, idx_obj, throw_flag);
}
#endif  /* error model */

DUK_LOCAL void duk__prop_set_write_tval(duk_hthread *thr, duk_idx_t idx_val, duk_tval *tv_slot) {
	duk_tval *tv_val;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(tv_slot != NULL);

	tv_val = thr->valstack_bottom + idx_val;
	DUK_TVAL_SET_TVAL_UPDREF(thr, tv_slot, tv_val);
}

/* Return true if calling code can assume Array instance has no inherited
 * array index properties.
 */
DUK_LOCAL duk_bool_t duk__prop_assume_no_inherited_array_indices(duk_hthread *thr, duk_hobject *obj) {
#if defined(DUK_USE_ARRAY_FASTPATH)
	/* With array fast path, always make the assumption.  This is not
	 * compliant but often very useful.
	 */
	DUK_UNREF(thr);
	DUK_UNREF(obj);
	return 1;
#else
	/* Without array fast path, never make the assumption. */
	DUK_UNREF(thr);
	DUK_UNREF(obj);
	return 0;
#endif
	/* As future work, it would be nice to detect the condition safely and
	 * quickly.  A safe condition would be something like:
	 *
	 *   1. The object's direct prototype is (the original) Array.prototype.
	 *   2. The inheritance chain of Array.prototype is untouched (i.e.
	 *      it inherits from (the original) Object.prototype which then
	 *      inherits from nothing).
	 *   3. Neither Array.prototype nor Object.prototype have index keys.
	 *   4. None of the objects involved are Proxies.
	 *
	 * Checking for index keys is now cheap because they are stored in a
	 * separate property table.  But even so, the safe check involves
	 * multiple branches and it may not necessarily be worth it.
	 *
	 * An alternative would be to use some sort of 'dirty' flag for
	 * Array.prototype, set when establishing index keys in either
	 * Object.prototype or Array.prototype, or when modifying the
	 * inheritance chain of either.  Mark-and-sweep could maybe recheck
	 * the flag periodically to see if it can be cleared.  With this the
	 * necessary check would be:
	 *
	 *   1. Check that the object inherits directly from the original
	 *      Array.prototype (or null).
	 *   2. Check that Array.prototype 'array index dirty' flag is false.
	 *      This flag could actually live in duk_hthread to avoid the
	 *      prototype lookup.
	 */
}

/* Return true if 'obj' is the original receiver of the operation.  This
 * allows some common case shortcuts.
 */
DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__prop_recv_direct(duk_hthread *thr, duk_idx_t idx_recv, duk_hobject *obj) {
	duk_hobject *recv;
	duk_tval *tv_recv;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(obj != NULL);

	tv_recv = DUK_GET_TVAL_POSIDX(thr, idx_recv);
	if (DUK_TVAL_IS_OBJECT(tv_recv)) {
		recv = DUK_TVAL_GET_OBJECT(tv_recv);
		return (recv == obj);
	}
	return 0;
}

/* Final [[Set]] processing for an index write to an Array with no items
 * part, in essence Array exotic [[DefineOwnProperty]] for index keys.
 */
DUK_LOCAL duk_bool_t duk__setfinal_write_array_abandoned_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val) {
	duk_harray *a = (duk_harray *) obj;
	duk_uint32_t old_len;
	duk_uint32_t new_len;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_ARRAY);
	DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));

	/* Fail if length update is needed and length is not writable.
	 * This must happen before the ordinary index write.
	 */
	old_len = DUK_HARRAY_GET_LENGTH(a);
	if (idx >= old_len) {
		if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a))) {
			goto fail_length_not_writable;
		}
		/* Ordinary write code checks for extensibility, so we don't
		 * need to check it here.
		 */
#if 0
		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
			goto fail_not_extensible;
		}
#endif
		new_len = idx + 1;
		DUK_ASSERT(new_len > idx);
		DUK_ASSERT(new_len >= 1U);
	} else {
		new_len = 0U;  /* Marker: no length update. */
	}

	/* Ordinary [[DefineOwnProperty]] for index part.  May fail if item
	 * exists and is write protected.  See notes on side effects below.
	 */
	rc = duk__setfinal_own_prop_idxkey_ordinary(thr, obj, idx, idx_val);

	/* Update array length if written idx extends array and the ordinary
	 * [[DefineOwnProperty]] was successful.
	 *
	 * It's critical that there are no visible side effects between the
	 * property write and the length update.  There are two major cases
	 * here:
	 *
	 * 1. 'idx' being written doesn't extend the array.  There may be
	 *    arbitrary side effects by e.g. an existing setter, a finalizer
	 *    triggered by a normal property write, etc.  There's no need
	 *    to update .length.
	 *
	 * 2. 'idx' extends the array and there is no previous property.
	 *    There can be no side effects from a previous property, but
	 *    property table resize can have side effects.  The resize code
	 *    avoids dangerous side effects like finalizers so we should be
	 *    able to update .length safely.
	 */
	if (new_len > 0U) {
		DUK_ASSERT(DUK_HARRAY_GET_LENGTH(a) == old_len);
		if (rc) {
			DUK_HARRAY_SET_LENGTH(a, new_len);
		}
	}

	DUK_HARRAY_ASSERT_VALID(thr->heap, a);
	return rc;

 fail_length_not_writable:
 fail_not_extensible:
	return 0;
}

/* Final [[Set]] processing for an Array with items part, in essence Array
 * exotic [[DefineOwnProperty]].
 *
 * Returns:
 *   0 = failure
 *   1 = success
 *  -1 = abandoned, caller must move on to index part
 */
DUK_LOCAL duk_small_int_t duk__setfinal_write_array_arrayitems_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val) {
	duk_harray *a = (duk_harray *) obj;
	duk_tval *tv_slot;
	duk_tval *tv_val;
	duk_uint32_t old_len;
	duk_uint32_t new_len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_ARRAY);
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));

	/* Length check before any mutation. */
	old_len = DUK_HARRAY_GET_LENGTH(a);
	if (idx >= old_len) {
		if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a))) {
			goto fail_length_not_writable;
		}
		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
			/* Don't try to extend if we're going to fail anyway. */
			goto fail_not_extensible;
		}
		new_len = idx + 1;
		DUK_ASSERT(new_len > idx);
		DUK_ASSERT(new_len >= 1U);
	} else {
		new_len = 0U;  /* Marker: no length update. */
	}

	/* Obtain a new array items slot, or abandon.  Possible resize is
	 * free of dangerous side effects.
	 *
	 * The helper does not check for extensibility (assumes we already did).
	 */
	tv_slot = duk_hobject_obtain_arridx_slot(thr, idx, obj);
	if (DUK_UNLIKELY(tv_slot == NULL)) {
		/* Failed to extend array items part, array is now abandoned.
		 * Handle in abandoned path (idx part).
		 */
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		return -1;
	}
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT(DUK_HARRAY_GET_LENGTH(a) == old_len);

	/* Write and update length.  As with the abandoned case, there are two
	 * important cases with regards to side effects:
	 *
	 * 1. 'idx' doesn't extend the array: setters are not possible (not
	 *    allowed with array items part) but finalizers are possible.
	 *    No length update is needed.
	 *
	 * 2. 'idx' extends the array.  A possible property table resize is
	 *    free of dangerous side effects so a .length update should be
	 *    safe.
	 */
	tv_val = thr->valstack_bottom + idx_val;
	if (DUK_TVAL_IS_UNUSED(tv_slot)) {
		/* This is duplicating the extensibility check above; it's
		 * needed for gappy arrays even when length is not extended.
		 */
		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
			goto fail_not_extensible;
		}
		DUK_TVAL_SET_TVAL_INCREF(thr, tv_slot, tv_val);
		if (new_len > 0U) {
			DUK_ASSERT(DUK_HARRAY_GET_LENGTH(a) == old_len);
			DUK_HARRAY_SET_LENGTH(a, new_len);
		}
	} else {
		/* Existing entry: no need to check extensibility and
		 * there will definitely be no length update.
		 */
		DUK_ASSERT(new_len == 0U);
		DUK_TVAL_SET_TVAL_UPDREF(thr, tv_slot, tv_val);  /* side effects */
	}
	DUK_HARRAY_ASSERT_VALID(thr->heap, a);
	return 1;

 fail_not_extensible:
 fail_length_not_writable:
	return 0;
}

/* Final [[Set]] processing for an Arguments with items part.  This just
 * handles writing to the array items part (or abandoning it), the Arguments
 * exotic [[DefineOwnProperty]] has already been handled in the check phase.
 */
DUK_LOCAL duk_small_int_t duk__setfinal_write_arguments_arrayitems_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val) {
	duk_harray *a = (duk_harray *) obj;
	duk_tval *tv_slot;
	duk_tval *tv_val;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_ARGUMENTS);
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_UNREF(a);

	/* Obtain a new array items slot, or abandon. */
	tv_slot = duk_hobject_obtain_arridx_slot(thr, idx, obj);
	if (DUK_UNLIKELY(tv_slot == NULL)) {
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		return -1;
	}
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));

	/* Write. */
	tv_val = thr->valstack_bottom + idx_val;
	if (DUK_TVAL_IS_UNUSED(tv_slot)) {
		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
			goto fail_not_extensible;
		}
		DUK_TVAL_SET_TVAL_INCREF(thr, tv_slot, tv_val);
	} else {
		/* Existing entry, no need to check extensibility. */
		DUK_TVAL_SET_TVAL_UPDREF(thr, tv_slot, tv_val);  /* side effects */
	}
	return 1;

 fail_not_extensible:
	return 0;
}

/* Helper for handling setter calls.
 * Return value: 1=found and handled, 0=not found, fail write.
 * Setter may also throw.
 */
DUK_LOCAL duk_bool_t duk__setcheck_own_prop_found_setter_helper(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_propvalue *pv, duk_uint8_t attrs, duk_bool_t use_key) {
	duk_propaccessor *pa;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(pv != NULL);
	DUK_ASSERT_VALSTACK_SPACE(thr, DUK_HOBJECT_PROP_VALSTACK_SPACE);
	DUK_ASSERT(attrs & DUK_PROPDESC_FLAG_ACCESSOR);
	DUK_ASSERT((use_key && key != NULL) || (!use_key && key == NULL));
	DUK_UNREF(obj);
	DUK_UNREF(attrs);

	pa = &pv->a;
	if (DUK_LIKELY(pa->set != NULL)) {
		duk_push_hobject(thr, pa->set);
		duk_dup(thr, idx_recv);          /* receiver; original uncoerced base */
		duk_dup(thr, idx_val);
#if defined(DUK_USE_NONSTD_SETTER_KEY_ARGUMENT)
		if (use_key) {
			DUK_ASSERT(key != NULL);
			duk_push_hstring(thr, key);
		} else {
			(void) duk_push_u32_tostring(thr, idx);
		}
		duk_call_method(thr, 2);         /* [ setter receiver(= this) val key ] -> [ retval ] */
#else
		DUK_UNREF(key);
		DUK_UNREF(idx);
		DUK_UNREF(use_key);
		duk_call_method(thr, 1);         /* [ setter receiver(= this) val ] -> [ retval ] */
#endif
		duk_pop_unsafe(thr);
		return 1;
	} else {
		/* If setter is missing, fail write. */
		return 0;
	}
}

#if defined(DUK_USE_NONSTD_SETTER_KEY_ARGUMENT)
DUK_LOCAL DUK_COLD DUK_NOINLINE duk_bool_t duk__setcheck_own_prop_found_setter_withkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_propvalue *pv, duk_uint8_t attrs) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(pv != NULL);

	return duk__setcheck_own_prop_found_setter_helper(thr, obj, key, 0, idx_val, idx_recv, pv, attrs, 1 /*use_key*/);
}
DUK_LOCAL DUK_COLD DUK_NOINLINE duk_bool_t duk__setcheck_own_prop_found_setter_withidx(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_propvalue *pv, duk_uint8_t attrs) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(pv != NULL);

	return duk__setcheck_own_prop_found_setter_helper(thr, obj, NULL, idx, idx_val, idx_recv, pv, attrs, 0 /*use_key*/);
}
#else
DUK_LOCAL DUK_COLD DUK_NOINLINE duk_bool_t duk__setcheck_own_prop_found_setter_nokey(duk_hthread *thr, duk_hobject *obj, duk_idx_t idx_val, duk_idx_t idx_recv, duk_propvalue *pv, duk_uint8_t attrs) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(pv != NULL);

	return duk__setcheck_own_prop_found_setter_helper(thr, obj, NULL, 0, idx_val, idx_recv, pv, attrs, 0 /*use_key*/);
}
#endif

DUK_LOCAL duk_bool_t duk__setcheck_own_prop_strkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	duk_propvalue *pv;
	duk_uint8_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);
	DUK_UNREF(throw_flag);

	if (duk_hobject_lookup_strprop_val_attrs(thr, obj, key, &pv, &attrs) != 0) {
		/* Fast path for write allowed case, single branch. */
		if (DUK_LIKELY((attrs & (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_ACCESSOR)) ==
		               DUK_PROPDESC_FLAG_WRITABLE)) {
			/* Not an accessor, writable property.  Fast path direct writes
			 * which are relatively common.
			 */
#if !defined(DUK_USE_PREFER_SIZE)
			if (duk__prop_recv_direct(thr, idx_recv, obj)) {
				duk__prop_set_write_tval(thr, idx_val, &pv->v);
				return DUK__SETCHECK_DONE_SUCCESS;
			}
#endif
			return DUK__SETCHECK_FOUND;
		} else {
			/* Accessor can be handled inline also for inherited case. */
			if ((attrs & DUK_PROPDESC_FLAG_ACCESSOR) != 0) {
#if defined(DUK_USE_NONSTD_SETTER_KEY_ARGUMENT)
				if (duk__setcheck_own_prop_found_setter_withkey(thr, obj, key, idx_val, idx_recv, pv, attrs) == 0) {
					goto fail_not_writable;
				}
#else
				if (duk__setcheck_own_prop_found_setter_nokey(thr, obj, idx_val, idx_val, pv, attrs) == 0) {
					goto fail_not_writable;
				}
#endif
				return DUK__SETCHECK_DONE_SUCCESS;
			} else {
				DUK_ASSERT((attrs & DUK_PROPDESC_FLAG_WRITABLE) == 0);
				goto fail_not_writable;
			}
		}
	} else {
		return DUK__SETCHECK_NOTFOUND;
	}

 fail_not_writable:
	return DUK__SETCHECK_DONE_FAILURE;
}

DUK_LOCAL duk_bool_t duk__setcheck_own_prop_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY: {
		duk_harray *a = (duk_harray *) obj;
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a))) {
				goto fail_not_writable;
			}
			/* For arrays the common case is a direct non-inherited
			 * write.  Writing to .length is common but usually not
			 * performance critical, so no fast path yet.
			 */
			return DUK__SETCHECK_FOUND;
		}
		break;
	}
	case DUK_HTYPE_ARGUMENTS:
		/* Special arguments behavior only triggers for index keys,
		 * so no special behavior here.
		 */
		break;
	case DUK_HTYPE_STRING_OBJECT:
		/* String objects don't have exotic [[Set]] behavior; the
		 * ordinary [[Set]] algorithm uses [[GetOwnProperty]].
		 * The String [[GetOwnProperty]] uses an ordinary property
		 * lookup, and if not found, then checks for String .length
		 * and indices.
		 *
		 * In practice it's easiest to handle it as exotic behavior
		 * for valid indices and .length before the ordinary property
		 * lookup as it shouldn't be possible to establish conflicting
		 * ordinary properties.
		 */
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			goto fail_not_writable;
		}
		break;
	case DUK_HTYPE_PROXY:
		/* Handled by the caller in a special way to allow target
		 * reference stabilization, see comments in duk_prop_get.c.
		 */
		return DUK__SETCHECK_HANDLE_SPECIAL;
	case DUK_HTYPE_COMPFUNC:
	case DUK_HTYPE_NATFUNC:
	case DUK_HTYPE_BOUNDFUNC:
		/* No exotic [[Set]] or [[GetOwnProperty]] behavior. */
		break;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
		/* ArrayBuffer and DataView .byteLength is an accessor,
		 * handle without fast path.
		 */
		break;
	case DUK_HTYPE_INT8ARRAY:
	case DUK_HTYPE_UINT8ARRAY:
	case DUK_HTYPE_UINT8CLAMPEDARRAY:
	case DUK_HTYPE_INT16ARRAY:
	case DUK_HTYPE_UINT16ARRAY:
	case DUK_HTYPE_INT32ARRAY:
	case DUK_HTYPE_UINT32ARRAY:
	case DUK_HTYPE_FLOAT32ARRAY:
	case DUK_HTYPE_FLOAT64ARRAY: {
		/* Typed array .length is an inherited accessor but we present a
		 * virtual own property.  Fail a write attempt (even with SameValue()
		 * compatible value).
		 *
		 * Special [[Set]] behavior for CanonicalNumericIndexStrings
		 * when in direct receiver case.
		 */

		duk_hbufobj *h = (duk_hbufobj *) obj;

		if (DUK_HSTRING_HAS_LENGTH_OR_CANNUM(key)) {
			if (DUK_HSTRING_HAS_CANNUM(key)) {
				/* Specification bug, exotic [[Set]] behavior should only
				 * trigger for direct receiver case (ES2016+ don't have a
				 * receiver check but it is implemented in practice).
				 */
				if (duk__prop_recv_direct(thr, idx_recv, obj)) {
					if (DUK_HBUFOBJ_IS_DETACHED(h)) {
						/* If detached, unconditional TypeError for any canonical
						 * numerix index string (even if out of range).
						 */
						duk__prop_set_error_objidx_str(thr, idx_recv, key, 1 /*throw_flag*/);
					}
					/* Else fall through to shared error. */
				} else {
					/* Not direct receiver, no exotic behavior.
					 * Ordinary lookup would return not found because
					 * we don't allow canonical numeric index string
					 * properties to be established, so just shortcut
					 * it here.
					 */
					return DUK__SETCHECK_NOTFOUND;
				}
			} else {
				DUK_ASSERT(DUK_HSTRING_HAS_LENGTH(key));
			}
			goto fail_not_writable;
		}
		break;
	}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	return duk__setcheck_own_prop_strkey_ordinary(thr, obj, key, idx_val, idx_recv, throw_flag);

 fail_not_writable:
	return DUK__SETCHECK_DONE_FAILURE;
}

DUK_LOCAL DUK_COLD DUK_NOINLINE duk_bool_t duk__setcheck_own_prop_idxkey_arguments(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag, duk_bool_t check_only) {
	duk_harray *a = (duk_harray *) obj;
	duk_hstring *varname;
	duk_hobject *map;
	duk_hobject *env;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);
	DUK_ASSERT(check_only == 0 || check_only == 1);

	/* Arguments exotic [[Set]] is in principle quite simple but difficult
	 * in practice because of side effects which need careful handling.
	 *
	 * Arguments exotic [[Set]] performs an arguments map write if:
	 *
	 *   1. the Arguments object is a direct receiver, and
	 *   2. the property key exists in the Arguments map.
	 *
	 * If the exotic behavior is triggered, the Arguments map write is
	 * done followed by an ordinary [[Set]] for the Arguments object.
	 *
	 * In the common case the ordinary [[Set]] will terminate because
	 * the property exists in the Arguments object (it would not be
	 * mapped otherwise) and will be updated.
	 *
	 * However, the side effects from the Arguments map write are arbitrary
	 * (due to a previous value being freed and possibly triggering
	 * finalization) and may include operating on the Arguments object
	 * or stranding the current target 'obj' before we resume the ordinary
	 * [[Set]].  For example, side effects can delete the property (and the
	 * mapping), so that an ordinary [[Set]] would then continue through
	 * the inheritance chain and might be observed e.g. if it hits a getter.
	 *
	 * Here we force stabilization of the 'target' object to protect
	 * against inheritance changes stranding the target, and allow side
	 * effects between the map write and the normal property write.  The
	 * stabilization happens using a special return code.
	 *
	 * There are many other possibilities, for example we could ensure
	 * that the Arguments map write side effect happens last, when the
	 * operation is otherwise complete.  To do so we would carefully handle
	 * the write order:
	 *
	 *    1. Push old Arguments property value on the value stack.
	 *    2. Write Arguments property value; this triggers no side
	 *       effects because no refcount can drop to zero.
	 *    3. Perform the Arguments map write.  This may have side
	 *       effects.
	 *    4. Pop the value stack temporary.  This may also have
	 *       side effects.
	 */

	/* Arguments [[Set]] special behavior is limited to direct receiver case. */
	if (!duk__prop_recv_direct(thr, idx_recv, obj)) {
		DUK_DD(DUK_DDPRINT("arguments [[Set]] not direct, no map check"));
		goto ordinary_set_check;
	}

	/* Check if index is mapped to a variable. */
	varname = duk_prop_arguments_map_prep_idxkey(thr, obj, idx, &map, &env);
	if (varname == NULL) {
		DUK_DD(DUK_DDPRINT("arguments [[Set]], not mapped"));
		goto ordinary_set_check;
	}

	/* Index is mapped.  Handle with a special return code so that
	 * 'target' can be stabilized in the [[Set]] check loop and
	 * the operation is then retried.
	 */
	if (check_only) {
		/* We're in unstabilized path, re-run after stabilizing. */
		DUK_DD(DUK_DDPRINT("arguments [[Set]], return special rc"));
		return DUK__SETCHECK_HANDLE_SPECIAL;
	}

	/* With check_only == 0, 'target' is stabilized so we can do the
	 * Arguments map write, followed by an ordinary [[Set]] check.
	 * Side effects may have altered the Arguments object here, but
	 * that's OK.
	 */

	/* Arguments map write. */
	duk_dup(thr, idx_val);
	duk_js_putvar_envrec(thr, env, varname, DUK_GET_TVAL_NEGIDX(thr, -1), throw_flag);
	duk_pop_unsafe(thr);

	/* Then normal [[Set]] check which may not hit a property, may hit
	 * a setter, etc, due to side effects from above.
	 */

 ordinary_set_check:
	DUK_DD(DUK_DDPRINT("arguments [[Set]], ordinary [[Set]] check"));
	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
		if (idx < DUK_HARRAY_GET_ITEMS_LENGTH(a)) {
			duk_tval *tv_val = DUK_HARRAY_GET_ITEMS(thr->heap, a) + idx;
			if (DUK_TVAL_IS_UNUSED(tv_val)) {
				return DUK__SETCHECK_NOTFOUND;
			} else {
				return DUK__SETCHECK_FOUND;
			}
		} else {
			return DUK__SETCHECK_NOTFOUND;
		}
	}

	return duk__setcheck_own_prop_idxkey_ordinary(thr, obj, idx, idx_val, idx_recv, throw_flag);
}

DUK_LOCAL duk_bool_t duk__setcheck_own_prop_idxkey_typedarray(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	duk_hbufobj *h = (duk_hbufobj *) obj;
	duk_size_t byte_off;
	duk_small_uint_t elem_size;
	duk_uint8_t *data;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);
	DUK_UNREF(throw_flag);

	/* In ES2015 the typed array [[Set]] would only trigger exotic behavior
	 * if the key was a canonical number index string and the typed array
	 * was the receiver.  In ES2016+ the direct receiver check has been
	 * omitted for both [[Get]] and [[Set]], i.e. the typed array should
	 * capture even an inherited read or write.
	 *
	 * - ES2015: https://www.ecma-international.org/ecma-262/6.0/#sec-integer-indexed-exotic-objects-set-p-v-receiver
	 * - ES2016: https://www.ecma-international.org/ecma-262/7.0/#sec-integer-indexed-exotic-objects-set-p-v-receiver
	 *
	 * This is apparently a specification bug for [[Set]], but there is
	 * not yet a clear resolution:
	 *
	 * - https://github.com/tc39/ecma262/issues/1541
	 *
	 * Best behavior for now is to add back the receiver check from ES2015
	 * for [[Set]], but omit it from [[Get]].
	 */

	/* [[Set]] fails for detached buffers both in direct and inherited
	 * case for canonical numeric index keys.  However, value conversion
	 * may have side effects changing the detached status so it can't be
	 * handled here reliably.
	 */

	/* Don't expose exotic [[Set]] behavior if obj is not a direct
	 * receiver.  This differs from [[Get]] which does allow indices
	 * to be read via inheritance (in ES2016+).
	 */
	if (DUK_UNLIKELY(!duk__prop_recv_direct(thr, idx_recv, obj))) {
		/* Not direct receiver, use OrdinarySet() which does a
		 * [[GetOwnProperty]].  We can assume we don't ever
		 * establish keys for indices beyond typed array length
		 * so we can skip a normal property table check also here.
		 * Index properties are always writable.
		 */
		if (DUK_UNLIKELY(DUK_HBUFOBJ_IS_DETACHED(h))) {
			goto fail_detached;
		}
		if (DUK_LIKELY(idx < DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h))) {
			return DUK__SETCHECK_FOUND;
		}
		return DUK__SETCHECK_NOTFOUND;
	}

	/* ToNumber() coercion may have side effects that operate on 'obj'
	 * or the inheritance path between receiver and 'obj', so we must
	 * stabilize 'obj' for the generic case.  Side effects may also
	 * affect 'obj' detached status.
	 */
	duk_push_hobject(thr, obj);
	duk_dup(thr, idx_val);
	(void) duk_to_number_m1(thr);

	/* Careful with wrapping: idx upshift may easily wrap, whereas
	 * length downshift won't.  Here we assume that logical length
	 * is controlled so that upshifting a valid index won't wrap.
	 */
	if (DUK_LIKELY(idx < DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h))) {
		byte_off = idx << h->shift;  /* no wrap assuming h_bufobj->length is valid */
		elem_size = (duk_small_uint_t) (1U << h->shift);

		if (DUK_LIKELY(h->buf != NULL && DUK_HBUFOBJ_VALID_BYTEOFFSET_EXCL(h, byte_off + elem_size))) {
			DUK_ASSERT(!DUK_HBUFOBJ_IS_DETACHED(h));
			data = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h->buf) + h->offset + byte_off;
			duk_hbufobj_validated_write(thr, h, data, elem_size);
		} else {
			/* Handle detached buffers here (uncovered case gets same treatment). */
			goto fail_detached;
		}
		rc = DUK__SETCHECK_DONE_SUCCESS;
	} else {
		rc = DUK__SETCHECK_DONE_FAILURE;
	}
	duk_pop_2_unsafe(thr);
	return rc;  /* Never continue lookup. */

 fail_detached:
	/* If detached, unconditional TypeError for any canonical
	 * numerix index string (even if out of range).
	 */
	duk__prop_set_error_objidx_idx(thr, idx_recv, idx, 1 /*throw_flag*/);
	DUK_WO_NORETURN(return 0;);
}

DUK_LOCAL duk_small_int_t duk__setcheck_own_prop_idxkey_array(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	duk_harray *a = (duk_harray *) obj;

	DUK_UNREF(throw_flag);

	/* Abandoned, generic handling. */
	if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj))) {
		goto abandoned;
	}

#if defined(DUK_USE_ARRAY_PROP_FASTPATH)
	/* Limited fast path for direct receiver case. */
	if (DUK_LIKELY(duk__prop_recv_direct(thr, idx_recv, obj))) {
		duk_tval *tv_slot;
		duk_tval *tv_val;
		duk_uint32_t old_len;
		duk_uint32_t new_len;

		/* See comments on side effects in the Array setfinal path. */

		old_len = DUK_HARRAY_GET_LENGTH(a);
		if (idx >= old_len) {
			if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a))) {
				goto fail_length_not_writable;
			}
			if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
				/* Don't try to extend if we're going to fail anyway. */
				goto fail_not_extensible;
			}
			new_len = idx + 1;
			DUK_ASSERT(new_len > idx);
			DUK_ASSERT(new_len >= 1U);
		} else {
			new_len = 0U;  /* Marker: no length update. */
		}

		/* Obtain a new array items slot, or abandon. */
		tv_slot = duk_hobject_obtain_arridx_slot(thr, idx, obj);
		if (DUK_UNLIKELY(tv_slot == NULL)) {
			/* Failed to extend array items part, array is now abandoned.
			 * Handle in abandoned path (idx part).
			 */
			DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
			goto abandoned;
		}
		DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		DUK_ASSERT(DUK_HARRAY_GET_LENGTH(a) == old_len);

		/* Write and update length. */
		tv_val = thr->valstack_bottom + idx_val;
		if (DUK_TVAL_IS_UNUSED(tv_slot)) {
			if (!duk__prop_assume_no_inherited_array_indices(thr, obj)) {
				return DUK__SETCHECK_NOTFOUND;
			}
			if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
				goto fail_not_extensible;
			}
			DUK_TVAL_SET_TVAL_INCREF(thr, tv_slot, tv_val);
			if (new_len > 0U) {
				DUK_ASSERT(DUK_HARRAY_GET_LENGTH(a) == old_len);
				DUK_HARRAY_SET_LENGTH(a, new_len);
			}
			return DUK__SETCHECK_DONE_SUCCESS;
		} else {
			/* Existing entry: no need to check extensibility and
			 * there will definitely be no length update.
			 */
			DUK_TVAL_SET_TVAL_UPDREF(thr, tv_slot, tv_val);  /* side effects */
			return DUK__SETCHECK_DONE_SUCCESS;
		}
	}
#endif  /* DUK_USE_ARRAY_PROP_FASTPATH */

	/* Not a direct receiver or no fastpath.  Just check existence. */
	if (idx < DUK_HARRAY_GET_LENGTH(a)) {
		duk_tval *tv_val = DUK_HARRAY_GET_ITEMS(thr->heap, a) + idx;
		if (DUK_TVAL_IS_UNUSED(tv_val)) {
			return DUK__SETCHECK_NOTFOUND;
		} else {
			return DUK__SETCHECK_FOUND;
		}
	} else {
		return DUK__SETCHECK_NOTFOUND;
	}

	/* Never here. */
	DUK_ASSERT(0);
	return DUK__SETCHECK_NOTFOUND;

 abandoned:
	return -1;

 fail_not_extensible:
 fail_length_not_writable:
	return DUK__SETCHECK_DONE_FAILURE;
}

DUK_LOCAL duk_bool_t duk__setcheck_own_prop_idxkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	duk_propvalue *pv;
	duk_uint8_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);
	DUK_UNREF(throw_flag);

	if (duk_hobject_lookup_idxprop_val_attrs(thr, obj, idx, &pv, &attrs) != 0) {
		/* Fast path for write allowed case, single branch. */
		if (DUK_LIKELY((attrs & (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_ACCESSOR)) ==
		               DUK_PROPDESC_FLAG_WRITABLE)) {
			/* Not an accessor, writable property.  Fast path direct writes
			 * which are relatively common.
			 */
#if !defined(DUK_USE_PREFER_SIZE)
			if (duk__prop_recv_direct(thr, idx_recv, obj)) {
				duk__prop_set_write_tval(thr, idx_val, &pv->v);
				return DUK__SETCHECK_DONE_SUCCESS;
			}
#endif
			return DUK__SETCHECK_FOUND;
		} else {
			/* Accessor can be handled inline also for inherited case. */
			if ((attrs & DUK_PROPDESC_FLAG_ACCESSOR) != 0) {
#if defined(DUK_USE_NONSTD_SETTER_KEY_ARGUMENT)
				if (duk__setcheck_own_prop_found_setter_withidx(thr, obj, idx, idx_val, idx_recv, pv, attrs) == 0) {
					goto fail_not_writable;
				}
#else
				if (duk__setcheck_own_prop_found_setter_nokey(thr, obj, idx_val, idx_val, pv, attrs) == 0) {
					goto fail_not_writable;
				}
#endif
				return DUK__SETCHECK_DONE_SUCCESS;
			} else {
				DUK_ASSERT((attrs & DUK_PROPDESC_FLAG_WRITABLE) == 0);
				goto fail_not_writable;
			}
		}
	}

	return DUK__SETCHECK_NOTFOUND;

 fail_not_writable:
	return DUK__SETCHECK_DONE_FAILURE;
}

DUK_LOCAL duk_bool_t duk__setcheck_own_prop_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY: {
		duk_small_int_t set_rc;

		set_rc = duk__setcheck_own_prop_idxkey_array(thr, obj, idx, idx_val, idx_recv, throw_flag);
		if (DUK_LIKELY(set_rc >= 0)) {
			return (duk_bool_t) set_rc;
		}
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		break;  /* Check from index part if abandoned. */
	}
	case DUK_HTYPE_ARGUMENTS:
		/* Arguments exotic [[Set]] is tricky, see detailed comments
		 * in the helper.
		 */
		return duk__setcheck_own_prop_idxkey_arguments(thr, obj, idx, idx_val, idx_recv, throw_flag, 1 /*check_only*/);
	case DUK_HTYPE_STRING_OBJECT: {
		/* String objects don't have exotic [[Set]] behavior; the
		 * ordinary [[Set]] algorithm uses [[GetOwnProperty]].
		 * In practice it's easiest to handle it as exotic
		 * behavior for valid indices and .length.
		 */
		duk_hstring *h;

		h = duk_hobject_lookup_intvalue_hstring(thr, obj);
		if (h != NULL && idx < DUK_HSTRING_GET_CHARLEN(h)) {
			goto fail_not_writable;
		}
		/* Out of bounds, go to normal property table. */
		break;
	}
	case DUK_HTYPE_PROXY:
		/* Handled by the caller in the NULL prototype path, see
		 * comments in duk_prop_get.c.
		 */
		return DUK__SETCHECK_NOTFOUND;
	case DUK_HTYPE_COMPFUNC:
	case DUK_HTYPE_NATFUNC:
	case DUK_HTYPE_BOUNDFUNC:
		/* No exotic [[Set]] or [[GetOwnProperty]] behavior. */
		break;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
		break;
	case DUK_HTYPE_INT8ARRAY:
	case DUK_HTYPE_UINT8ARRAY:
	case DUK_HTYPE_UINT8CLAMPEDARRAY:
	case DUK_HTYPE_INT16ARRAY:
	case DUK_HTYPE_UINT16ARRAY:
	case DUK_HTYPE_INT32ARRAY:
	case DUK_HTYPE_UINT32ARRAY:
	case DUK_HTYPE_FLOAT32ARRAY:
	case DUK_HTYPE_FLOAT64ARRAY:
		return duk__setcheck_own_prop_idxkey_typedarray(thr, obj, idx, idx_val, idx_recv, throw_flag);
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	return duk__setcheck_own_prop_idxkey_ordinary(thr, obj, idx, idx_val, idx_recv, throw_flag);

 fail_not_writable:
	return DUK__SETCHECK_DONE_FAILURE;
}

DUK_LOCAL duk_bool_t duk__setfinal_own_prop_strkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_val) {
	duk_propvalue *pv;
	duk_uint8_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));

	if (duk_hobject_lookup_strprop_val_attrs(thr, obj, key, &pv, &attrs) != 0) {
		/* The specification algorithm handles accessors curiously in
		 * the final step: if the receiver has an own accessor property,
		 * it is NOT invoked but causes a write failure.  This is
		 * probably intended to standardize behavior for cases where
		 * the prototype chain or objects are modified by side effect
		 * during handling of [[Set]].
		 *
		 * This case can only happen if we first (1) pass through the
		 * receiver object without encountering any property, and (2)
		 * then a side effect modifies the receiver or prototype chain
		 * before the [[Set]] prototype search terminates.
		 *
		 * Hitting a setter won't trigger this because then we don't
		 * back up to the receiver at all.  The same applies to a Proxy
		 * 'set' trap.
		 *
		 * However, it's possible for a 'set' trap lookup to fail to
		 * find a property but have side effects, e.g. if the handler
		 * object is a Proxy or has a 'set' getter.
		 */
		if (DUK_UNLIKELY((attrs & (DUK_PROPDESC_FLAG_ACCESSOR | DUK_PROPDESC_FLAG_WRITABLE)) != DUK_PROPDESC_FLAG_WRITABLE)) {
#if defined(DUK_USE_DEBUG)
			if (attrs & DUK_PROPDESC_FLAG_ACCESSOR) {
				DUK_D(DUK_DPRINT("receiver property is an accessor in final step of [[Set]], fail property write"));
			}
#endif
			goto fail_not_writable;
		}

		duk__prop_set_write_tval(thr, idx_val, &pv->v);
	} else {
		duk_uint_fast32_t ent_idx;
		duk_propvalue *val_base;
		duk_hstring **key_base;
		duk_uint8_t *attr_base;
		duk_tval *tv_dst;
		duk_tval *tv_val;

		/* Create new property. */

		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
			goto fail_not_extensible;
		}

		/* Entry allocation updates hash part and increases the key
		 * refcount; may need a props allocation resize but doesn't
		 * 'recheck' the valstack and won't have side effects.
		 */
		ent_idx = (duk_uint_fast32_t) duk_hobject_alloc_strentry_checked(thr, obj, key);
		DUK_ASSERT(ent_idx >= 0);

		val_base = DUK_HOBJECT_E_GET_VALUE_BASE(thr->heap, obj);
		key_base = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, obj);
		attr_base = DUK_HOBJECT_E_GET_FLAGS_BASE(thr->heap, obj);
		DUK_UNREF(key_base);

		DUK_ASSERT(key_base[ent_idx] == key);
		pv = val_base + ent_idx;
		tv_dst = &pv->v;
		tv_val = thr->valstack_bottom + idx_val;
		/* Previous value is garbage, so no DECREF. */
		DUK_TVAL_SET_TVAL_INCREF(thr, tv_dst, tv_val);
		attr_base[ent_idx] = DUK_PROPDESC_FLAGS_WEC;
	}
	return 1;

 fail_not_extensible:
 fail_not_writable:
	return 0;
}

/* Handle successful write to 'obj' (receiver).  This is actually the
 * property write part of the [[Set]] algorithm(s), and specification
 * handles it as [[DefineOwnProperty]] (possibly via CreateDataProperty),
 * so all [[DefineOwnProperty]] exotic behaviors apply.
 */
DUK_LOCAL duk_bool_t duk__setfinal_own_prop_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_val) {
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));

	/* Here 'obj' is always the receiver, and is stabilized automatically
	 * by the duk_tval at 'idx_recv'.
	 */

	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_UNLIKELY(DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj))) {
		goto fail_not_writable;
	}
#endif

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		/* Array does not have exotic [[Set]] behavior.  Conceptually
		 * ordinary [[Set]] finds Array .length descriptor and checks
		 * its writability.  If writable, [[DefineOwnProperty]] is
		 * invoked.  If not writable, [[DefineOwnProperty]] is not
		 * invoked at all (even if value would be SameValue to existing).
		 */
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			duk_harray *a = (duk_harray *) obj;
			duk_uint32_t new_len;

			/* Recheck in case of side effects mutating object. */
			if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a))) {
				goto fail_not_writable;
			}

			/* 'obj' is stabilized in value stack, so length coercion
			 * side effects are OK here.
			 */
			new_len = duk_harray_to_array_length_checked(thr, DUK_GET_TVAL_POSIDX(thr, idx_val));
			return duk_harray_put_array_length_u32(thr, obj, new_len, 0 /*force_flag*/);
		}
		break;
	case DUK_HTYPE_ARGUMENTS:
		/* Parameter map only contains arridx keys, so the
		 * Arguments [[DefineOwnProperty]] simplifies to
		 * OrdinaryDefineOwnProperty().
		 */
		break;
	case DUK_HTYPE_STRING_OBJECT:
		/* We should have already rejected a .length write in the
		 * "check" phase.
		 */
		DUK_ASSERT(!DUK_HSTRING_HAS_LENGTH(key));
		if (DUK_UNLIKELY(DUK_HSTRING_HAS_LENGTH(key))) {
			/* Fail safely. */
			goto fail_not_writable;
		}
		break;
	case DUK_HTYPE_PROXY:
		/* We should not come here: Proxy [[Set]] either continues
		 * to the Proxy target if no trap is found, or the trap is
		 * invoked and the [[Set]] process finishes there.  Cause
		 * an internal error for safety here.
		 */
		goto fail_internal;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
		break;
	case DUK_HTYPE_INT8ARRAY:
	case DUK_HTYPE_UINT8ARRAY:
	case DUK_HTYPE_UINT8CLAMPEDARRAY:
	case DUK_HTYPE_INT16ARRAY:
	case DUK_HTYPE_UINT16ARRAY:
	case DUK_HTYPE_INT32ARRAY:
	case DUK_HTYPE_UINT32ARRAY:
	case DUK_HTYPE_FLOAT32ARRAY:
	case DUK_HTYPE_FLOAT64ARRAY: {
		/* We should normally not come here: the .length
		 * or canonical numeric index string case has
		 * already been checked and rejected in the 'check'
		 * phase for this object (as direct receiver).
		 */
		DUK_ASSERT(!DUK_HSTRING_HAS_LENGTH(key));
		DUK_ASSERT(!DUK_HSTRING_HAS_CANNUM(key));
		if (DUK_UNLIKELY(DUK_HSTRING_HAS_LENGTH_OR_CANNUM(key))) {
			/* Fail safely. */
			goto fail_not_writable;
		}
		break;
	}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	return duk__setfinal_own_prop_strkey_ordinary(thr, obj, key, idx_val);

 fail_not_writable:
 fail_internal:
	return 0;
}

DUK_LOCAL duk_bool_t duk__setfinal_own_prop_idxkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val) {
	duk_propvalue *pv;
	duk_uint8_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));

	if (duk_hobject_lookup_idxprop_val_attrs(thr, obj, idx, &pv, &attrs) != 0) {
		if (DUK_LIKELY((attrs & (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_ACCESSOR)) ==
		               DUK_PROPDESC_FLAG_WRITABLE)) {
#if defined(DUK_USE_DEBUG)
			if (attrs & DUK_PROPDESC_FLAG_ACCESSOR) {
				DUK_D(DUK_DPRINT("receiver property is an accessor in final step of [[Set]] => fail property write"));
			}
#endif
			goto fail_not_writable;
		}
		duk__prop_set_write_tval(thr, idx_val, &pv->v);
		DUK_GC_TORTURE(thr->heap);
	} else {
		duk_uint_fast32_t ent_idx;
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;
		duk_uint8_t *attr_base;
		duk_tval *tv_dst;
		duk_tval *tv_src;

		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj))) {
			goto fail_not_extensible;
		}

		ent_idx = (duk_uint_fast32_t) duk_hobject_alloc_idxentry_checked(thr, obj, idx);

		val_base = (duk_propvalue *) (void *) obj->idx_props;
		key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);
		attr_base = (duk_uint8_t *) (void *) (key_base + obj->i_size);

		DUK_ASSERT(key_base[ent_idx] == idx);
		attr_base[ent_idx] = DUK_PROPDESC_FLAGS_WEC;
		tv_dst = &(val_base + ent_idx)->v;
		tv_src = thr->valstack_bottom + idx_val;
		DUK_TVAL_SET_TVAL_INCREF(thr, tv_dst, tv_src);
		DUK_GC_TORTURE(thr->heap);
	}
	return 1;

 fail_not_extensible:
 fail_not_writable:
	return 0;
}

DUK_LOCAL duk_bool_t duk__setfinal_own_prop_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val) {
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));

	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_UNLIKELY(DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj))) {
		goto fail_not_writable;
	}
#endif

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		/* Array does not have exotic [[Set]] behavior.  Conceptually
		 * ordinary [[Set]] finds Array index descriptors and checks
		 * their writability.  If writable, [[DefineOwnProperty]] is
		 * invoked (which then has some exotic behaviors).  If not
		 * writable, [[DefineOwnProperty]] is not invoked at all,
		 * even if value would be SameValue to existing.
		 */
		if (DUK_LIKELY(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj))) {
			duk_small_int_t setfin_rc;

			setfin_rc = duk__setfinal_write_array_arrayitems_idxkey(thr, obj, idx, idx_val);
			if (setfin_rc > 0) {
				DUK_ASSERT(setfin_rc == 1);
				DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
				return (duk_bool_t) setfin_rc;
			} else if (setfin_rc == 0) {
				goto fail_array;
			} else {
				DUK_ASSERT(setfin_rc == -1);
				DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
			}
		}
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		return duk__setfinal_write_array_abandoned_idxkey(thr, obj, idx, idx_val);
	case DUK_HTYPE_ARGUMENTS:
		/* Similar to Array case, but no exotic 'length'.  Array magic
		 * arguments map behavior was already applied in the 'check'
		 * phase.
		 */
		if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
			duk_small_int_t setfin_rc;

			setfin_rc = duk__setfinal_write_arguments_arrayitems_idxkey(thr, obj, idx, idx_val);
			if (setfin_rc > 0) {
				DUK_ASSERT(setfin_rc == 1);
				DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
				return (duk_bool_t) setfin_rc;
			} else if (setfin_rc == 0) {
				goto fail_array;
			} else {
				DUK_ASSERT(setfin_rc < 0);
				DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
			}
			DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		}
		break;  /* No array items, handle in shared index part path. */
	case DUK_HTYPE_STRING_OBJECT:
		/* "Check" path should have rejected writes to all exotic
		 * index properties (0 <= idx < .length) and .length.  So
		 * here only ordinary behavior should remain.
		 */
		break;
	case DUK_HTYPE_PROXY:
		/* We should not come here: Proxy [[Set]] either continues
		 * to the Proxy target if no trap is found, or the trap is
		 * invoked and the [[Set]] process finishes there.  Fail
		 * safely.
		 */
		DUK_D(DUK_DPRINT("Proxy final [[Set]], should not happen"));
		DUK_ASSERT(0);
		goto fail_internal;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
		break;
	case DUK_HTYPE_INT8ARRAY:
	case DUK_HTYPE_UINT8ARRAY:
	case DUK_HTYPE_UINT8CLAMPEDARRAY:
	case DUK_HTYPE_INT16ARRAY:
	case DUK_HTYPE_UINT16ARRAY:
	case DUK_HTYPE_INT32ARRAY:
	case DUK_HTYPE_UINT32ARRAY:
	case DUK_HTYPE_FLOAT32ARRAY:
	case DUK_HTYPE_FLOAT64ARRAY: {
		/* If [[Set]] is applied to a typed array with a valid
		 * CanonicalNumericIndexString (all arridx are such strings,
		 * but also many other strings like 'NaN'), the [[Set]]
		 * operation won't return to the original receiver call site,
		 * i.e. it terminates in the check phase.
		 *
		 * While typed arrays have a custom [[DefineOwnProperty]] we
		 * should never come here.  Side effects during [[Set]] lookup
		 * (e.g. Proxy trap checks) can modify the typed array but
		 * cannot change its [[Set]] capturing nature.
		 */
		DUK_D(DUK_DPRINT("typed array final [[Set]], should not happen"));
		DUK_ASSERT(0);
		goto fail_internal;
	}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	return duk__setfinal_own_prop_idxkey_ordinary(thr, obj, idx, idx_val);

#if defined(DUK_USE_ROM_OBJECTS)
 fail_not_writable:
#endif
 fail_internal:
 fail_array:
	return 0;
}

DUK_LOCAL duk_bool_t duk__prop_set_proxy_tail(duk_hthread *thr, duk_idx_t idx_val, duk_idx_t idx_recv) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));

	duk_dup(thr, idx_val);
	duk_dup(thr, idx_recv);
	duk_call_method(thr, 4);  /* [ ... trap handler target key val receiver ] -> [ ... result ] */
	duk_pop_unsafe(thr);

#if 0
	/* XXX: proxy policy check */
#endif

	return 1;
}

DUK_LOCAL DUK_NOINLINE duk_bool_t duk__setcheck_strkey_proxy(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);
	DUK_UNREF(throw_flag);

	if (duk_proxy_trap_check_strkey(thr, (duk_hproxy *) obj, key, DUK_STRIDX_SET)) {
		duk_push_hstring(thr, key);
		return duk__prop_set_proxy_tail(thr, idx_val, idx_recv);
	} else {
		return 0;
	}
}

DUK_LOCAL DUK_NOINLINE duk_bool_t duk__setcheck_idxkey_proxy(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);
	DUK_UNREF(throw_flag);

	if (duk_proxy_trap_check_idxkey(thr, (duk_hproxy *) obj, idx, DUK_STRIDX_SET)) {
		(void) duk_push_u32_tostring(thr, idx);
		return duk__prop_set_proxy_tail(thr, idx_val, idx_recv);
	} else {
		return 0;
	}
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__prop_set_stroridx_helper(duk_hthread *thr, duk_hobject *target, duk_hstring *key, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag, duk_bool_t use_key, duk_bool_t side_effect_safe) {
	duk_bool_t rc;
	duk_small_uint_t sanity;
	duk_tval *tv_recv;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(target != NULL);
	if (use_key) {
		DUK_ASSERT(key != NULL);
		DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	} else {
		DUK_ASSERT_ARRIDX_VALID(idx);
	}
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	if (side_effect_safe) {
		duk_push_hobject(thr, target);
	}

	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	do {
		duk_hobject *next;

#if defined(DUK_USE_ASSERTIONS)
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) target) > 0);
		if (side_effect_safe && DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) target) == 1) {
			DUK_D(DUK_DPRINT("'target' is only reachable via stabilized value stack slot"));
		}
#endif

		DUK_GC_TORTURE(thr->heap);
		if (use_key) {
			rc = duk__setcheck_own_prop_strkey(thr, target, key, idx_val, idx_recv, throw_flag);
		} else {
			rc = duk__setcheck_own_prop_idxkey(thr, target, idx, idx_val, idx_recv, throw_flag);
		}
		DUK_GC_TORTURE(thr->heap);

	 recheck_rc:
		if (rc == DUK__SETCHECK_NOTFOUND) {
			/* Not found, continue lookup with ordinary [[Set]]. */
		} else if (rc == DUK__SETCHECK_FOUND) {
			/* Found, write allowed. */
			goto allow_write;
		} else if (rc == DUK__SETCHECK_DONE_SUCCESS) {
			/* Found and handled inline, present success. */
			goto success;
		} else if (rc == DUK__SETCHECK_DONE_FAILURE) {
			/* Found and handled inline, present failure. */
			goto fail;
		} else {
			DUK_ASSERT(rc == DUK__SETCHECK_HANDLE_SPECIAL);
			DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(target) == DUK_HTYPE_PROXY || DUK_HOBJECT_GET_HTYPE(target) == DUK_HTYPE_ARGUMENTS);

			/* Proxy or Arguments: special handling, require safe path.
			 * This awkward handling is done so that we can avoid
			 * unnecessary target stabilization unless we find a Proxy
			 * an Arguments object.
			 */

			if (side_effect_safe) {
				if (DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(target)) {
					DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(target) == DUK_HTYPE_PROXY);
					if (use_key) {
						rc = duk__setcheck_strkey_proxy(thr, target, key, idx_val, idx_recv, throw_flag);
					} else {
						rc = duk__setcheck_idxkey_proxy(thr, target, idx, idx_val, idx_recv, throw_flag);
					}
					DUK_ASSERT(rc == 0 || rc == 1);
					if (rc) {
						DUK_ASSERT(rc == 1);
						goto success;
					} else {
						DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(target));
						next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) target);
						DUK_ASSERT(next != NULL);
						goto go_next;
					}
				} else {
					DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(target) == DUK_HTYPE_ARGUMENTS);
					if (use_key) {
						DUK_ASSERT(0);
					} else {
						rc = duk__setcheck_own_prop_idxkey_arguments(thr, target, idx, idx_val, idx_recv, throw_flag, 0 /*check_only*/);
						goto recheck_rc;
					}
				}
			} else {
				goto switch_to_safe;
			}
		}

#if defined(DUK_USE_ASSERTIONS)
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) target) > 0);
		if (side_effect_safe && DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) target) == 1) {
			DUK_D(DUK_DPRINT("'target' is only reachable via stabilized value stack slot"));
		}
#endif

		next = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, target);
		if (next == NULL) {
			goto allow_write;
		} else {
			DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(target) != DUK_HTYPE_PROXY);
		}

	 go_next:
		DUK_ASSERT(next != NULL);
		if (side_effect_safe) {
			duk_tval *tv_target;

			tv_target = thr->valstack_top - 1;
			DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv_target));
			DUK_ASSERT(DUK_TVAL_GET_OBJECT(tv_target) == target);
			DUK_HOBJECT_INCREF(thr, next);
			DUK_TVAL_UPDATE_OBJECT(tv_target, next);
			DUK_HOBJECT_DECREF(thr, target);
		}
		target = next;
	} while (--sanity > 0);

	DUK_ERROR_RANGE_PROTO_SANITY(thr);
	DUK_WO_NORETURN(return 0;);

 allow_write:
	/* Property (a) not found in inheritance chain, or (b) found a data
	 * property which doesn't prevent a write.  Getters and proxies are
	 * handled when they are found and we don't come back here.  Receiver
	 * may be primitive here so we may still fail to write, and side
	 * effects may have altered the receiver during the prototype walk.
	 */
	tv_recv = thr->valstack_bottom + idx_recv;
	if (DUK_LIKELY(DUK_TVAL_IS_OBJECT(tv_recv))) {
		duk_hobject *recv = DUK_TVAL_GET_OBJECT(tv_recv);
		target = NULL;
		if (key) {
			rc = duk__setfinal_own_prop_strkey(thr, recv, key, idx_val);
		} else {
			rc = duk__setfinal_own_prop_idxkey(thr, recv, idx, idx_val);
		}
		if (rc == 0) {
			goto fail;
		}
		goto success;
	} else {
		goto fail;
	}

 success:
	if (side_effect_safe) {
		duk_pop_unsafe(thr);
	}
	return 1;

 fail:
	/* This pop is needed even on error, because throw_flag may be 0
	 * in which case we don't actually throw.
	 */
	if (side_effect_safe) {
		duk_pop_unsafe(thr);
	}
	if (use_key) {
		return duk__prop_set_error_objidx_str(thr, idx_recv, key, throw_flag);
	} else {
		return duk__prop_set_error_objidx_idx(thr, idx_recv, idx, throw_flag);
	}

 switch_to_safe:
	if (use_key) {
		return duk__prop_set_str_safe(thr, target, key, idx_val, idx_recv, throw_flag);
	} else {
		return duk__prop_set_idx_safe(thr, target, idx, idx_val, idx_recv, throw_flag);
	}
}

DUK_LOCAL DUK_NOINLINE duk_bool_t duk__prop_set_str_safe(duk_hthread *thr, duk_hobject *target, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	return duk__prop_set_stroridx_helper(thr, target, key, 0, idx_val, idx_recv, throw_flag, 1 /*use_key*/, 1 /*side_effect_safe*/);
}

DUK_LOCAL duk_bool_t duk__prop_set_str_unsafe(duk_hthread *thr, duk_hobject *target, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_set_str_safe(thr, target, key, 0, idx_val, idx_recv, throw_flag);
#else
	return duk__prop_set_stroridx_helper(thr, target, key, 0, idx_val, idx_recv, throw_flag, 1 /*use_key*/, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL DUK_NOINLINE duk_bool_t duk__prop_set_idx_safe(duk_hthread *thr, duk_hobject *target, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	return duk__prop_set_stroridx_helper(thr, target, NULL, idx, idx_val, idx_recv, throw_flag, 0 /*use_key*/, 1 /*side_effect_safe*/);
}

DUK_LOCAL duk_bool_t duk__prop_set_idx_unsafe(duk_hthread *thr, duk_hobject *target, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_set_idx_safe(thr, target, NULL, idx, idx_val, idx_recv, throw_flag);
#else
	return duk__prop_set_stroridx_helper(thr, target, NULL, idx, idx_val, idx_recv, throw_flag, 0 /*use_key*/, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL duk_bool_t duk__prop_set_str(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	DUK_STATS_INC(thr->heap, stats_set_strkey_count);

	return duk__prop_set_str_unsafe(thr, obj, key, idx_val, idx_recv, throw_flag);
}

DUK_LOCAL duk_bool_t duk__prop_set_idx(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val, duk_idx_t idx_recv, duk_bool_t throw_flag) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	DUK_STATS_INC(thr->heap, stats_set_idxkey_count);

	return duk__prop_set_idx_unsafe(thr, obj, idx, idx_val, idx_recv, throw_flag);
}

DUK_LOCAL duk_bool_t duk__prop_putvalue_idx_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_uarridx_t idx, duk_idx_t idx_val, duk_bool_t throw_flag) {
	duk_hobject *next;
	duk_small_uint_t next_bidx;
	duk_tval *tv_recv;
	duk_small_uint_t tag_recv;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	DUK_STATS_INC(thr->heap, stats_putvalue_idxkey_count);

	tv_recv = thr->valstack_bottom + idx_recv;
	DUK_ASSERT(DUK_HTHREAD_TVAL_IN_VSFRAME(thr, tv_recv));

	tag_recv = DUK_TVAL_GET_TAG(tv_recv);

#if 1
	if (tag_recv == DUK_TAG_OBJECT) {
		/* Typical case. */
		next = DUK_TVAL_GET_OBJECT(tv_recv);
		goto go_next;
	}
#endif

	switch (DUK_TVAL_GET_TAG(tv_recv)) {
	case DUK_TAG_UNUSED:
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
		return duk__prop_set_error_objidx_idx(thr, idx_recv, idx, 1 /*unconditional*/);
	case DUK_TAG_STRING: {
		/* For arridx in string valid range a TypeError is required:
		 * [[GetOwnProperty]] for ToObject coerced value (String
		 * object) would return a non-writable descriptor, and
		 * ordinary [[Set]] would reject the write regardless of
		 * the value (even for SameValue() compatible value).
		 *
		 * For arridx outside the valid range the [[Set]] walk should
		 * walk the inheritance chain.  Assuming no property is found,
		 * the final write will fail due to a primitive base value.
		 * However, if a getter is found, it must be invoked, so we
		 * can't terminate the search here for out-of-bounds indices.
		 * A proxy could also be hit, although only if prototype
		 * objects have been replaced.
		 */
		duk_hstring *h = DUK_TVAL_GET_STRING(tv_recv);

		if (DUK_LIKELY(!DUK_HSTRING_HAS_SYMBOL(h))) {
			if (idx < DUK_HSTRING_GET_CHARLEN(h)) {
				goto fail_not_writable;
			}
			next_bidx = DUK_BIDX_STRING_PROTOTYPE;
		} else {
			next_bidx = DUK_BIDX_SYMBOL_PROTOTYPE;
		}
		break;
	}
	case DUK_TAG_OBJECT:
#if 0
		/* Typical case. */
		next = DUK_TVAL_GET_OBJECT(tv_recv);
		goto go_next;
#else
		/* Never here. */
		return 0;
#endif
	case DUK_TAG_BUFFER: {
		/* Uint8Array like all typed arrays have an exotic [[Set]]
		 * algorithm capturing all indices.  Unlike with standard
		 * primitive values, here we behave as if the plain buffer
		 * was actually an Uint8Array object, allowing the write.
		 * Out-of-bounds indices cause a failure with no inheritance.
		 */
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv_recv);
		duk_tval *tv_val;
		duk_uint32_t coerced_val;

		/* Coerce value before index check because coercion may have
		 * side effects; this is unnecessary if the write fails, but
		 * don't optimize for out-of-bounds.  Side effects should not
		 * affect 'h' because it's stabilized by the idx_recv slot.
		 */
		tv_val = DUK_GET_TVAL_POSIDX(thr, idx_val);
#if 1
		if (DUK_LIKELY(DUK_TVAL_IS_FASTINT(tv_val))) {
			coerced_val = DUK_TVAL_GET_FASTINT_U32(tv_val);
		} else
#endif
		{
			coerced_val = duk_to_uint32(thr, idx_val);  /* arbitrary side effects */
		}

		if (DUK_LIKELY(idx < DUK_HBUFFER_GET_SIZE(h))) {
			duk_uint8_t *buf = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h);
			buf[idx] = (duk_uint8_t) (coerced_val & 0xffU);
			return 1;
		} else {
			goto fail_not_writable;
		}
		break;
	}
	case DUK_TAG_BOOLEAN:
		next_bidx = DUK_BIDX_BOOLEAN_PROTOTYPE;
		break;
	case DUK_TAG_POINTER:
		next_bidx = DUK_BIDX_POINTER_PROTOTYPE;
		break;
	case DUK_TAG_LIGHTFUNC:
		/* No virtual index properties so just continue lookup. */
		next_bidx = DUK_BIDX_NATIVE_FUNCTION_PROTOTYPE;
		break;
	default:
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_recv));
		next_bidx = DUK_BIDX_NUMBER_PROTOTYPE;
	}

	next = thr->builtins[next_bidx];
	/* fall thru */
 go_next:
	return duk__prop_set_idx(thr, next, idx, idx_val, idx_recv, throw_flag);

 fail_not_writable:
	return duk__prop_set_error_objidx_idx(thr, idx_recv, idx, throw_flag);
}

DUK_LOCAL duk_bool_t duk__prop_putvalue_str_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_hstring *key, duk_idx_t idx_val, duk_bool_t throw_flag) {
	duk_hobject *next;
	duk_small_uint_t next_bidx;
	duk_tval *tv_recv;
	duk_small_uint_t tag_recv;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_index(thr, idx_recv));
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	DUK_ASSERT(duk_is_valid_index(thr, idx_val));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	DUK_STATS_INC(thr->heap, stats_putvalue_strkey_count);

	tv_recv = thr->valstack_bottom + idx_recv;
	DUK_ASSERT(DUK_HTHREAD_TVAL_IN_VSFRAME(thr, tv_recv));

	tag_recv = DUK_TVAL_GET_TAG(tv_recv);
#if 1
	if (tag_recv == DUK_TAG_OBJECT) {
		/* Typical case. */
		next = DUK_TVAL_GET_OBJECT(tv_recv);
		goto go_next;
	}
#endif

	/* Conceptually receiver is ToObject() coerced, and then [[Set]] is
	 * attempted on the result.  For non-object values, the coerced types
	 * are: String, Uint8Array, Boolean, Pointer, Function, Number.  Only
	 * Uint8Array has a special [[Set]] algorithm.  For standard [[Set]]
	 * we check whether an own property exists, and if so, is it writable
	 * (setter etc is also in principle possible).
	 *
	 * Ultimately, even if the property exists and is writable, the final
	 * attempt to write a property to a primitive value fails with a
	 * TypeError.  So all we need to check is if an own property exists,
	 * and if so (assuming it's not a setter), error out.
	 */
	switch (tag_recv) {
	case DUK_TAG_UNUSED:
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
		return duk__prop_set_error_objidx_str(thr, idx_recv, key, 1 /*unconditional*/);
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv_recv);

		if (DUK_LIKELY(!DUK_HSTRING_HAS_SYMBOL(h))) {
			if (DUK_HSTRING_HAS_LENGTH(key)) {
				goto fail_not_writable;
			}
			next_bidx = DUK_BIDX_STRING_PROTOTYPE;
		} else {
			next_bidx = DUK_BIDX_SYMBOL_PROTOTYPE;
		}
		break;
	}
	case DUK_TAG_OBJECT:
#if 0
		/* Typical case. */
		next = DUK_TVAL_GET_OBJECT(tv_recv);
		goto go_next;
#else
		/* Never here. */
		return 0;
#endif
	case DUK_TAG_BUFFER:
		/* Plain buffers are a custom type so there are no mandatory
		 * behaviors.  But we want to mimic Uint8Array here; for them,
		 * there's no standard own 'length' property but an inherited
		 * setter, but we currently present a non-writable own property
		 * 'length' for Uint8Arrays.
		 *
		 * Non-arridx CanonicalNumericIndexStrings come here, and they
		 * should fail with no further inheritance checks.  Other keys
		 * should walk the prototype chain to allow side effects.
		 */
		if (DUK_HSTRING_HAS_LENGTH_OR_CANNUM(key)) {
			goto fail_not_writable;
		}
		next_bidx = DUK_BIDX_UINT8ARRAY_PROTOTYPE;
		break;
	case DUK_TAG_BOOLEAN:
		next_bidx = DUK_BIDX_BOOLEAN_PROTOTYPE;
		break;
	case DUK_TAG_POINTER:
		next_bidx = DUK_BIDX_POINTER_PROTOTYPE;
		break;
	case DUK_TAG_LIGHTFUNC:
		/* Lightfuncs have no own properties so just continue lookup. */
		next_bidx = DUK_BIDX_NATIVE_FUNCTION_PROTOTYPE;
		break;
	default:
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_recv));
		next_bidx = DUK_BIDX_NUMBER_PROTOTYPE;
	}

	next = thr->builtins[next_bidx];
	/* fall thru */
 go_next:
	return duk__prop_set_str(thr, next, key, idx_val, idx_recv, throw_flag);

 fail_not_writable:
	return duk__prop_set_error_objidx_str(thr, idx_recv, key, throw_flag);
}

DUK_INTERNAL duk_bool_t duk_prop_putvalue_idx_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_uarridx_t idx, duk_idx_t idx_val, duk_bool_t throw_flag) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	if (DUK_LIKELY(idx <= DUK_ARRIDX_MAX)) {
		return duk__prop_putvalue_idx_inidx(thr, idx_recv, idx, idx_val, throw_flag);
	} else {
		duk_bool_t rc;
		duk_hstring *key;

		DUK_D(DUK_DPRINT("corner case, input idx 0xffffffff is not an arridx, must coerce to string"));
		key = duk_push_u32_tohstring(thr, idx);
		rc = duk__prop_putvalue_str_inidx(thr, idx_recv, key, idx_val, throw_flag);
		duk_pop_unsafe(thr);
		return rc;
	}
}

DUK_INTERNAL duk_bool_t duk_prop_putvalue_str_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_hstring *key, duk_idx_t idx_val, duk_bool_t throw_flag) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
		return duk__prop_putvalue_idx_inidx(thr, idx_recv, DUK_HSTRING_GET_ARRIDX_FAST_KNOWN(key), idx_val, throw_flag);
	} else {
		return duk__prop_putvalue_str_inidx(thr, idx_recv, key, idx_val, throw_flag);
	}
}

DUK_INTERNAL duk_bool_t duk_prop_putvalue_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_tval *tv_key, duk_idx_t idx_val, duk_bool_t throw_flag) {
	duk_bool_t rc;
	duk_hstring *key;
	duk_uarridx_t idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_recv));
	DUK_ASSERT(tv_key != NULL);
	/* tv_key may not be in value stack but it must be reachable and
	 * remain reachable despite arbitrary side effects (e.g. function
	 * constant table).
	 */
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_val));
	/* 'idx_val' is in value stack but we're not allowed to mutate it
	 * because it might be a VM register source.
	 */
	DUK_ASSERT(throw_flag == 0 || throw_flag == 1);

	switch (DUK_TVAL_GET_TAG(tv_key)) {
	case DUK_TAG_STRING:
		key = DUK_TVAL_GET_STRING(tv_key);
		if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
			idx = DUK_HSTRING_GET_ARRIDX_FAST_KNOWN(key);
			goto use_idx;
		} else {
			goto use_str;
		}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT: {
		duk_int64_t fi = DUK_TVAL_GET_FASTINT(tv_key);
		if (fi >= 0 && fi <= (duk_int64_t) DUK_ARRIDX_MAX) {
			idx = (duk_uarridx_t) fi;
			goto use_idx;
		}
		break;
	}
#endif
#if !defined(DUK_USE_PACKED_TVAL)
	case DUK_TAG_NUMBER: {
		duk_double_t d = DUK_TVAL_GET_DOUBLE(tv_key);
		if (duk_prop_double_idx_check(d, &idx)) {
			goto use_idx;
		}
		break;
	}
#endif
	case DUK_TAG_UNUSED:
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
	case DUK_TAG_BOOLEAN:
	case DUK_TAG_POINTER:
	case DUK_TAG_LIGHTFUNC:
	case DUK_TAG_OBJECT:
	case DUK_TAG_BUFFER:
		break;
	default: {
#if defined(DUK_USE_PACKED_TVAL)
		duk_double_t d;
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_key));
		DUK_ASSERT(!DUK_TVAL_IS_FASTINT(tv_key));
		d = DUK_TVAL_GET_DOUBLE(tv_key);
		if (duk_prop_double_idx_check(d, &idx)) {
			goto use_idx;
		}
#endif
		break;
	}
	}

	if (duk_is_null_or_undefined(thr, idx_recv)) {
		/* Ensure ToObject() coercion error happens before key coercion
		 * side effects.
		 */
		return duk__prop_set_error_objidx_tvkey(thr, idx_recv, tv_key, 1 /*unconditional*/);
	}

	duk_push_tval(thr, tv_key);
	tv_key = NULL;
	key = duk_to_property_key_hstring(thr, -1);
	rc = duk_prop_putvalue_str_inidx(thr, idx_recv, key, idx_val, throw_flag);
	duk_pop_unsafe(thr);
	return rc;

 use_idx:
	DUK_ASSERT_ARRIDX_VALID(idx);
	return duk__prop_putvalue_idx_inidx(thr, idx_recv, idx, idx_val, throw_flag);

 use_str:
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	return duk__prop_putvalue_str_inidx(thr, idx_recv, key, idx_val, throw_flag);
}
