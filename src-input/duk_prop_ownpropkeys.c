/*
 *  [[OwnPropertyKeys]]
 *
 *  ES2015+ [[OwnPropertyKeys]] ordering guarantees:
 *
 *    1. Array indices in ascending order, i.e. CanonicalNumericIndexStrings
 *       representing integers in [0, 0xfffffffe].
 *
 *    2. String keys in insertion order.  This includes non-array-index
 *       CanonicalNumericIndexStrings, e.g. ToString(0xffffffff), 'NaN',
 *       'Infinity'.
 *
 *    3. Symbol keys in insertion order.
 *
 *  Proxy 'ownKeys' trap does not have forced ordering at [[OwnPropertyKeys]]
 *  level.  Instead, they are sorted in EnumerableOwnPropertyNames() after
 *  filtering.  Proxy invariants for 'ownKeys' trap are complex and heavy.
 *
 *  Object property enumeration which includes inheritance has a separate
 *  algorithm, EnumerateObjectProperties:
 *  https://tc39.es/ecma262/#sec-enumerate-object-properties
 */

#include "duk_internal.h"

/* Append index keys of an array with an items part to output array.
 * Index keys are not (yet) string coerced.
 */
DUK_LOCAL duk_uarridx_t duk__prop_ownpropkeys_arrayitems(duk_hthread *thr,
                                                         duk_hobject *obj,
                                                         duk_harray *arr_out,
                                                         duk_uarridx_t idx_out) {
	duk_harray *a = (duk_harray *) obj;
	duk_tval *tv_out;
	duk_tval *tv_out_start;
	duk_tval *tv_base;
	duk_uint32_t i, n;

	/* Reserve space for maximum result size, actual size may be smaller
	 * due to input array gaps.
	 */
	n = DUK_HARRAY_GET_ITEMS_LENGTH(a);
	tv_out_start = duk_harray_append_reserve_items(thr, arr_out, idx_out, n);
	tv_out = tv_out_start;
	DUK_ASSERT(tv_out != NULL || (idx_out == 0 && n == 0));

	tv_base = DUK_HARRAY_GET_ITEMS(thr->heap, a);

	/* Array resize is side effect protected so assumptions
	 * should still hold.
	 */

	for (i = 0, n = DUK_HARRAY_GET_ITEMS_LENGTH(a); i < n; i++) {
		duk_tval *tv_val = tv_base + i;
		if (DUK_TVAL_IS_UNUSED(tv_val)) {
			/* Gaps are excluded from result. */
			continue;
		}
		DUK_TVAL_SET_U32(tv_out, (duk_uint32_t) i);
		tv_out++;
	}
	idx_out += (duk_uarridx_t) (tv_out - tv_out_start);
	DUK_HARRAY_SET_LENGTH(arr_out, idx_out);

	return idx_out;
}

/* Emit index keys without gaps for e.g. a string object.  Index keys are
 * not (yet) string coerced.
 */
DUK_LOCAL duk_uarridx_t duk__prop_ownpropkeys_append_indices(duk_hthread *thr,
                                                             duk_uint32_t n,
                                                             duk_harray *arr_out,
                                                             duk_uarridx_t idx_out) {
	duk_tval *tv_out;
	duk_tval *tv_out_start;
	duk_uint32_t i;

	tv_out_start = duk_harray_append_reserve_items(thr, arr_out, idx_out, n);
	tv_out = tv_out_start;
	DUK_ASSERT(tv_out != NULL || (idx_out == 0 && n == 0));

	for (i = 0; i < n; i++) {
		DUK_TVAL_SET_U32(tv_out, (duk_uint32_t) i);
		tv_out++;
	}
	idx_out += n;
	DUK_HARRAY_SET_LENGTH(arr_out, idx_out);

	return idx_out;
}

DUK_LOCAL duk_uarridx_t duk__prop_ownpropkeys_bufobj_indices(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_harray *arr_out,
                                                             duk_uarridx_t idx_out) {
	duk_hbufobj *h = (duk_hbufobj *) obj;
	duk_uarridx_t res;
	duk_uint32_t n;

	n = (duk_uint32_t) DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h);
	res = duk__prop_ownpropkeys_append_indices(thr, n, arr_out, idx_out);
	/* Side effects are possible, but they cannot alter the length of a
	 * typed array so 'n' should still be valid.
	 */
	DUK_ASSERT(DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h) == n);
	return res;
}

DUK_LOCAL duk_uarridx_t duk__prop_ownpropkeys_strobj_indices(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_harray *arr_out,
                                                             duk_uarridx_t idx_out) {
	duk_hstring *h;
	duk_uarridx_t res;
	duk_uint32_t n;

	h = duk_hobject_get_internal_value_string(thr->heap, obj);
	DUK_ASSERT(h != NULL);
	if (!h) {
		return idx_out;
	}

	n = duk_hstring_get_charlen(h);
	res = duk__prop_ownpropkeys_append_indices(thr, n, arr_out, idx_out);
	DUK_ASSERT(duk_hstring_get_charlen(h) == n);
	return res;
}

DUK_LOCAL duk_uarridx_t duk__prop_ownpropkeys_idxprops(duk_hthread *thr,
                                                       duk_hobject *obj,
                                                       duk_harray *arr_out,
                                                       duk_uarridx_t idx_out,
                                                       duk_uint_t ownpropkeys_flags) {
	duk_tval *tv_out_start;
	duk_tval *tv_out;
	duk_uint32_t i, n;
	duk_propvalue *val_base;
	duk_uarridx_t *key_base;
	duk_uint8_t *attr_base;

	n = obj->i_next;
	tv_out_start = duk_harray_append_reserve_items(thr, arr_out, idx_out, n);
	tv_out = tv_out_start;
	DUK_ASSERT(tv_out != NULL || (idx_out == 0 && n == 0));

	duk_hobject_get_idxprops_key_attr(thr->heap, obj, &val_base, &key_base, &attr_base);

	for (i = 0, n = obj->i_next; i < n; i++) {
		duk_uarridx_t idx;
		duk_uint8_t attrs;

		idx = key_base[i];
		if (idx == DUK_ARRIDX_NONE) {
			continue;
		}
		attrs = attr_base[i];
		if ((ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_REQUIRE_ENUMERABLE) && (attrs & DUK_PROPDESC_FLAG_ENUMERABLE) == 0) {
			continue;
		}

		DUK_TVAL_SET_U32(tv_out, (duk_uint32_t) idx);
		tv_out++;
	}
	idx_out += (duk_uarridx_t) (tv_out - tv_out_start);
	DUK_HARRAY_SET_LENGTH(arr_out, idx_out);

	return idx_out;
}

DUK_LOCAL duk_uarridx_t duk__prop_ownpropkeys_strprops(duk_hthread *thr,
                                                       duk_hobject *obj,
                                                       duk_harray *arr_out,
                                                       duk_uarridx_t idx_out,
                                                       duk_uint_t ownpropkeys_flags,
                                                       duk_bool_t symbol_phase,
                                                       duk_bool_t *out_found_symbols) {
	duk_tval *tv_out_start;
	duk_tval *tv_out;
	duk_uint32_t i, n;
	duk_propvalue *val_base;
	duk_hstring **key_base;
	duk_uint8_t *attr_base;
	duk_bool_t found_symbols = 0;

	n = duk_hobject_get_enext(obj);
	tv_out_start = duk_harray_append_reserve_items(thr, arr_out, idx_out, n);
	tv_out = tv_out_start;
	DUK_ASSERT(tv_out != NULL || (idx_out == 0 && n == 0));

	duk_hobject_get_strprops_key_attr(thr->heap, obj, &val_base, &key_base, &attr_base);

	for (i = 0, n = duk_hobject_get_enext(obj); i < n; i++) {
		duk_hstring *key;
		duk_uint8_t attrs;

		key = key_base[i];
		if (key == NULL) {
			continue;
		}
		attrs = attr_base[i];
		if ((ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_REQUIRE_ENUMERABLE) && (attrs & DUK_PROPDESC_FLAG_ENUMERABLE) == 0) {
			continue;
		}
		if (symbol_phase) {
			if (!DUK_HSTRING_HAS_SYMBOL(key) || DUK_HSTRING_HAS_HIDDEN(key)) {
				continue;
			}
		} else {
			if (DUK_HSTRING_HAS_SYMBOL(key)) {
				found_symbols = 1;
				continue;
			}
		}

		DUK_TVAL_SET_STRING_INCREF(thr, tv_out, key);
		tv_out++;
	}
	idx_out += (duk_uarridx_t) (tv_out - tv_out_start);
	DUK_HARRAY_SET_LENGTH(arr_out, idx_out);

	*out_found_symbols = found_symbols;
	return idx_out;
}

DUK_LOCAL duk_uarridx_t duk__prop_ownpropkeys_add_length(duk_hthread *thr, duk_uarridx_t idx_out) {
	duk_push_hstring_stridx(thr, DUK_STRIDX_LENGTH);
	duk_xdef_prop_index_wec(thr, -2, idx_out);
	idx_out++;

	return idx_out;
}

DUK_LOCAL void duk__prop_ownpropkeys_string_coerce(duk_hthread *thr, duk_harray *arr_out) {
	duk_uint32_t i, n;

	/* Arbitrary side effects are possible here.  However, 'arr_out' is
	 * not reachable to finalizers etc, so we can safely assume it won't
	 * be mutated.
	 */
	for (i = 0, n = DUK_HARRAY_GET_LENGTH(arr_out); i < n; i++) {
		duk_hstring *tmp;
		duk_tval *tv_slot;

		tv_slot = DUK_HARRAY_GET_ITEMS(thr->heap, arr_out) + i;
		duk_push_tval(thr, tv_slot);
		tmp = duk_to_hstring(thr, -1);
		DUK_ASSERT(tmp != NULL);
		tv_slot = DUK_HARRAY_GET_ITEMS(thr->heap, arr_out) + i;
		DUK_TVAL_SET_STRING(tv_slot, tmp); /* Steal refcount. */
		tv_slot = DUK_GET_TVAL_NEGIDX(thr, -1);
		DUK_TVAL_SET_UNDEFINED(tv_slot);
		thr->valstack_top--;
	}
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_uint32_t duk__prop_ownpropkeys_get_known_u32(duk_tval *tv) {
	duk_uint32_t res;

#if defined(DUK_USE_FASTINT)
	DUK_ASSERT(DUK_TVAL_IS_FASTINT(tv));
	res = DUK_TVAL_GET_FASTINT_U32(tv);
#else
	DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
	res = (duk_uint32_t) DUK_TVAL_GET_DOUBLE(tv);
#endif
	return res;
}

/* Sort index keys emitted so far.  This is needed when index keys
 * come from the index part, and are unordered.  Insertion sort for
 * now, should be quicksort or similar to handle large cases.
 */
DUK_LOCAL void duk__prop_ownpropkeys_sort_index_keys(duk_hthread *thr, duk_harray *arr_out) {
	duk_uint32_t i, n;
	duk_tval *val_base;

	DUK_UNREF(thr);

	n = DUK_HARRAY_GET_LENGTH(arr_out);
	if (n == 0) {
		return;
	}
	val_base = DUK_HARRAY_GET_ITEMS(thr->heap, arr_out);
	for (i = 1; i < n; i++) {
		duk_uint32_t curr_val;
		duk_uint32_t j;

		curr_val = duk__prop_ownpropkeys_get_known_u32(val_base + i);

		/* Insert curr_val at index i into sorted subarray [0,i[. */
		j = i;
		for (;;) {
			duk_uint32_t test_val;

			DUK_ASSERT(j >= 1U);
			j--;

			test_val = duk__prop_ownpropkeys_get_known_u32(val_base + j);
			if (curr_val >= test_val) { /* Equality not actually possible. */
				j++;
				break;
			}
			if (DUK_UNLIKELY(j == 0U)) {
				break;
			}
		}

		/* curr_val belongs at index j. */
		if (j < i) {
			duk_tval *copy_dst;
			duk_tval *copy_src;
			duk_tval *tv_i;
			duk_size_t copy_size;
			duk_tval tv_tmp;

			/*
			 *           j   i
			 *           |   |
			 *           v   v
			 *   A B C D F G E ...
			 *           ===
			 *
			 *   A B C D F F G ...
			 *             ===
			 *
			 *   A B C D E F G ...
			 *           =
			 */
			copy_src = val_base + j;
			copy_dst = copy_src + 1;
			copy_size = i - j;
			tv_i = val_base + i;
			DUK_TVAL_SET_TVAL(&tv_tmp, tv_i);
			duk_memmove((void *) copy_dst, (const void *) copy_src, (duk_size_t) (copy_size * sizeof(duk_tval)));
			DUK_TVAL_SET_TVAL(copy_src, &tv_tmp);
		}
	}
}

#if defined(DUK_USE_PROXY_POLICY)
/* Incorrect approximate implementation for now. */
DUK_LOCAL void duk__prop_ownpropkeys_proxy_policy(duk_hthread *thr, duk_hobject *obj, duk_uint_t ownpropkeys_flags) {
	duk_hobject *target;
	duk_uarridx_t i, len, out_idx = 0;
	duk_tval *tv_out;
	duk_bool_t target_extensible;

	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_PROXY);

	target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
	DUK_ASSERT(target != NULL);
	target_extensible = duk_js_isextensible(thr, target);
	DUK_UNREF(target_extensible);

	if (!duk_is_object(thr, -1)) {
		goto proxy_reject;
	}

	len = (duk_uarridx_t) duk_get_length(thr, -1);
	tv_out = duk_push_harray_with_size_outptr(thr, len);
	DUK_UNREF(tv_out);

	for (i = 0; i < len; i++) {
		duk_hstring *h;

		/* [ obj(stabilized) arr_out(ignored) trap_result res_arr ] */
		(void) duk_get_prop_index(thr, -2, i);

		h = duk_get_hstring(thr, -1);
		if (DUK_UNLIKELY(h == NULL)) {
			DUK_ERROR_TYPE_PROXY_REJECTED(thr);
			DUK_WO_NORETURN(return;);
		}

		if (DUK_UNLIKELY(DUK_HSTRING_HAS_SYMBOL(h))) {
			if (!(ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL)) {
				DUK_DDD(DUK_DDDPRINT("ignore symbol property: %!T", duk_get_tval(thr, -1)));
				goto skip_key;
			}
			if (DUK_HSTRING_HAS_HIDDEN(h) && !(ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_HIDDEN)) {
				DUK_DDD(DUK_DDDPRINT("ignore hidden symbol property: %!T", duk_get_tval(thr, -1)));
				goto skip_key;
			}
		} else {
			if (!(ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING)) {
				DUK_DDD(DUK_DDDPRINT("ignore string property: %!T", duk_get_tval(thr, -1)));
				goto skip_key;
			}
		}

		/* XXX: This is not correct, but approximate for now. */
		if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_REQUIRE_ENUMERABLE) {
			duk_small_int_t attrs = duk_prop_getownattr_obj_strkey(thr, target, h);
			if (attrs >= 0) {
				if (((duk_small_uint_t) attrs & DUK_PROPDESC_FLAG_ENUMERABLE) == 0) {
					DUK_DDD(DUK_DDDPRINT("ignore non-enumerable property: %!T", duk_get_tval(thr, -1)));
					goto skip_key;
				}
			} else {
				DUK_DDD(DUK_DDDPRINT("ignore non-existent property: %!T", duk_get_tval(thr, -1)));
				goto skip_key;
			}
		}

		/* [ obj trap_result res_arr propname ] */
		duk_xdef_prop_index_wec(thr, -2, out_idx++);
		continue;

	skip_key:
		duk_pop_known(thr);
		continue;
	}

	/* Truncate result array, it may be smaller than raw trap result. */
	duk_set_length(thr, -1, out_idx);

	duk_remove_m2(thr); /* Remove direct trap result. */
	return;

proxy_reject:
	DUK_ERROR_TYPE_PROXY_REJECTED(thr);
}
#endif

DUK_LOCAL duk_small_int_t duk__prop_ownpropkeys_proxy(duk_hthread *thr, duk_hobject *obj, duk_uint_t ownpropkeys_flags) {
	if (duk_proxy_trap_check_nokey(thr, (duk_hproxy *) obj, DUK_STRIDX_OWN_KEYS)) {
		duk_call_method(thr, 1); /* [ ... trap handler target ] -> [ ... result ] */

#if defined(DUK_USE_PROXY_POLICY)
		duk__prop_ownpropkeys_proxy_policy(thr, obj, ownpropkeys_flags);
#else
		DUK_DD(DUK_DDPRINT("proxy policy check for 'ownKeys' trap disabled in configuration"));
#endif

		return 1;
	} else {
		return -1;
	}
}

DUK_INTERNAL void duk_prop_ownpropkeys(duk_hthread *thr, duk_hobject *obj, duk_uint_t ownpropkeys_flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_small_uint_t htype;
	duk_uarridx_t idx_out = 0;
	duk_harray *arr_out;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	/* Stabilize 'obj' in case we need to traverse Proxy targets.  Keep
	 * the current object in this value stack slot (link to previous
	 * objects may be severed by side effects).
	 */
	duk_push_hobject(thr, obj);

	/* Output is always a duk_harray with a gap-free items part.  We can write
	 * out items directly, and they implicitly get WEC attributes which is
	 * compatible with Reflect.ownKeys() etc.
	 */
	arr_out = duk_push_harray(thr);

	/* [ ... obj arr_out ] */

retry_obj:
	htype = DUK_HOBJECT_GET_HTYPE(obj);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY:
	case DUK_HTYPE_ARGUMENTS:
		if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
			if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX) {
				idx_out = duk__prop_ownpropkeys_arrayitems(thr, obj, arr_out, idx_out);
			}
			goto skip_index_part;
		}
		break;
	case DUK_HTYPE_STRING_OBJECT:
		if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX) {
			idx_out = duk__prop_ownpropkeys_strobj_indices(thr, obj, arr_out, idx_out);
		}
		/* Indices >= .length are handled below as ordinary properties. */
		break;
	case DUK_HTYPE_PROXY: {
		duk_small_int_t proxy_rc;

		if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_NO_PROXY_BEHAVIOR) {
			DUK_DD(DUK_DDPRINT("caller request no proxy behavior for enumerating own property keys, skip proxy check"));
			break;
		}

		proxy_rc = duk__prop_ownpropkeys_proxy(thr, obj, ownpropkeys_flags);
		if (proxy_rc < 0) {
			/* Not found, continue to Proxy target. */
			duk_hproxy *h = (duk_hproxy *) obj;
			duk_hobject *next;

			next = duk_proxy_get_target_autothrow(thr, h);
			DUK_ASSERT(next != NULL);

			/* Replace stabilized object. */
			duk_push_hobject(thr, next);
			duk_replace(thr, -3);
			obj = next;

			goto retry_obj;
		} else {
			duk_remove_m2(thr); /* [ ... arr_out ] */
			goto success;
		}
		break;
	}
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
		if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX) {
			duk_hbufobj *h = (duk_hbufobj *) obj;
			if (!DUK_HBUFOBJ_IS_DETACHED(h)) {
				idx_out = duk__prop_ownpropkeys_bufobj_indices(thr, obj, arr_out, idx_out);
			} else {
				/* Detached: don't enumerate at all. */
			}
		}
		goto skip_index_part;
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	/* Index properties from idxprops part.  These need to be sorted
	 * because they're stored in a hash.
	 */
	if (obj->i_next > 0) {
		DUK_ASSERT(duk_hobject_get_idxprops(thr->heap, obj) != NULL);
		if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX) {
			idx_out = duk__prop_ownpropkeys_idxprops(thr, obj, arr_out, idx_out, ownpropkeys_flags);
			duk__prop_ownpropkeys_sort_index_keys(thr, arr_out);
		}
	}

skip_index_part:
	/* At this point there are sorted index keys in the result. */

	/* String coerce index keys inserted so far if necessary. */
	if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_STRING_COERCE) {
		duk__prop_ownpropkeys_string_coerce(thr, arr_out);
	}

	/* Virtual .length property? */
	if ((ownpropkeys_flags & (DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING | DUK_OWNPROPKEYS_FLAG_REQUIRE_ENUMERABLE)) ==
	    (DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING)) {
		htype = DUK_HOBJECT_GET_HTYPE(obj);
		DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

		switch (htype) {
		case DUK_HTYPE_ARRAY:
		case DUK_HTYPE_STRING_OBJECT:
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
		case DUK_HTYPE_INT8ARRAY:
		case DUK_HTYPE_UINT8ARRAY:
		case DUK_HTYPE_UINT8CLAMPEDARRAY:
		case DUK_HTYPE_INT16ARRAY:
		case DUK_HTYPE_UINT16ARRAY:
		case DUK_HTYPE_INT32ARRAY:
		case DUK_HTYPE_UINT32ARRAY:
		case DUK_HTYPE_FLOAT32ARRAY:
		case DUK_HTYPE_FLOAT64ARRAY:
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
			idx_out = duk__prop_ownpropkeys_add_length(thr, idx_out);
			break;
		default:
			break;
		}
	}

	/* At this point there are sorted index keys and maybe a few non-index
	 * keys (e.g. 'length') in the result after the index keys.
	 *
	 * String/Symbol properties in the string property part are in
	 * insertion order, but ES2015+ ordering requires that strings
	 * come before Symbols.  This could be achieved by first inserting
	 * the keys and then sorting them.
	 *
	 * We handle this here with two insertion passes now: first we
	 * insert all string properties and also keep track if any
	 * Symbols are encountered.  If Symbols were found, Symbols are
	 * added in the final step if desired.
	 */

	if (duk_hobject_get_strprops(thr->heap, obj) != NULL && duk_hobject_get_enext(obj) > 0) {
		duk_bool_t found_symbols = 1;

		if (ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING) {
			idx_out = duk__prop_ownpropkeys_strprops(thr,
			                                         obj,
			                                         arr_out,
			                                         idx_out,
			                                         ownpropkeys_flags,
			                                         0 /*symbol phase*/,
			                                         &found_symbols);
		}
		if ((ownpropkeys_flags & DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL) && found_symbols) {
			idx_out = duk__prop_ownpropkeys_strprops(thr,
			                                         obj,
			                                         arr_out,
			                                         idx_out,
			                                         ownpropkeys_flags,
			                                         1 /*symbol phase*/,
			                                         &found_symbols);
		}
	}

success:
	DUK_HARRAY_ASSERT_VALID(thr->heap, (duk_harray *) duk_require_hobject(thr, -1));

	duk_remove_m2(thr); /* Remove stabilized 'obj': [ ... obj arr_out ] -> [ ... arr_out ] */

	DUK_ASSERT(duk_get_top(thr) == entry_top + 1);
}
