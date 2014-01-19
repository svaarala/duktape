/*
 *  Ecmascript specification algorithm and conversion helpers.
 *
 *  These helpers encapsulate the primitive Ecmascript operation
 *  semantics, and are used by the bytecode executor and the API
 *  (among other places).  Note that some primitives are only
 *  implemented as part of the API and have no "internal" helper.
 *  (This is the case when an internal helper would not really be
 *  useful; e.g. the operation is rare, uses value stack heavily,
 *  etc.)
 *
 *  The operation arguments depend on what is required to implement
 *  the operation:
 *
 *    - If an operation is simple and stateless, and has no side
 *      effects, it won't take an duk_hthread argument and its
 *      arguments may be duk_tval pointers (which are safe as long
 *      as no side effects take place).
 *
 *    - If complex coercions are required (e.g. a "ToNumber" coercion)
 *      or errors may be thrown, the operation takes an duk_hthread
 *      argument.  This also implies that the operation may have
 *      arbitrary side effects, invalidating any duk_tval pointers.
 *
 *    - For operations with potential side effects, arguments can be
 *      taken in several ways:
 *
 *      a) as duk_tval pointers, which makes sense if the "common case"
 *         can be resolved without side effects (e.g. coercion); the
 *         arguments are pushed to the valstack for coercion if
 *         necessary
 *
 *      b) as duk_tval values
 *
 *      c) implicitly on value stack top
 *
 *      d) as indices to the value stack
 *
 *  Future work:
 *
 *     - Argument styles may not be the most sensible in every case now.
 *
 *     - In-place coercions might be useful for several operations, if
 *       in-place coercion is OK for the bytecode executor and the API.
 */

#include "duk_internal.h"

/*
 *  [[DefaultValue]]  (E5 Section 8.12.8)
 *
 *  ==> implemented in the API.
 */

/*
 *  ToPrimitive()  (E5 Section 9.1)
 *
 *  ==> implemented in the API.
 */

/*
 *  ToBoolean()  (E5 Section 9.2)
 */

int duk_js_toboolean(duk_tval *tv) {
	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
		return 0;
	case DUK_TAG_BOOLEAN:
		return DUK_TVAL_GET_BOOLEAN(tv);
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);
		return (h->blen > 0 ? 1 : 0);
	}
	case DUK_TAG_OBJECT: {
		return 1;
	}
	case DUK_TAG_BUFFER: {
		/* mimic semantics for strings */
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_GET_SIZE(h) > 0 ? 1 : 0);
	}
	case DUK_TAG_POINTER: {
		void *p = DUK_TVAL_GET_POINTER(tv);
		return (p != NULL ? 1 : 0);
	}
	default: {
		/* number */
		int c;
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		c = DUK_FPCLASSIFY(DUK_TVAL_GET_NUMBER(tv));
		if (c == DUK_FP_ZERO || c == DUK_FP_NAN) {
			return 0;
		} else {
			return 1;
		}
	}
	}
	DUK_UNREACHABLE();
}

/*
 *  ToNumber()  (E5 Section 9.3)
 *
 *  Value to convert must be on stack top, and is popped before exit.
 *
 *  See: http://www.cs.indiana.edu/~burger/FP-Printing-PLDI96.pdf
 *       http://www.cs.indiana.edu/~burger/fp/index.html
 *
 *  Notes on the conversion:
 *
 *    - There are specific requirements on the accuracy of the conversion
 *      through a "Mathematical Value" (MV), so this conversion is not
 *      trivial.
 *
 *    - Quick rejects (e.g. based on first char) are difficult because
 *      the grammar allows leading and trailing white space.
 *
 *    - Quick reject based on string length is difficult even after
 *      accounting for white space; there may be arbitrarily many
 *      decimal digits.
 *
 *    - Standard grammar allows decimal values ("123"), hex values
 *      ("0x123") and infinities
 *
 *    - Unlike source code literals, ToNumber() coerces empty strings
 *      and strings with only whitespace to zero (not NaN).
 */	

/* E5 Section 9.3.1 */
static double tonumber_string_raw(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	int s2n_flags;
	double d;

	/* Quite lenient, e.g. allow empty as zero, but don't allow trailing
	 * garbage.
	 */
	s2n_flags = DUK_S2N_FLAG_TRIM_WHITE |
	            DUK_S2N_FLAG_ALLOW_EXP |
	            DUK_S2N_FLAG_ALLOW_PLUS |
	            DUK_S2N_FLAG_ALLOW_MINUS |
	            DUK_S2N_FLAG_ALLOW_INF |
	            DUK_S2N_FLAG_ALLOW_FRAC |
	            DUK_S2N_FLAG_ALLOW_NAKED_FRAC |
	            DUK_S2N_FLAG_ALLOW_EMPTY_FRAC |
	            DUK_S2N_FLAG_ALLOW_EMPTY_AS_ZERO |
	            DUK_S2N_FLAG_ALLOW_LEADING_ZERO |
	            DUK_S2N_FLAG_ALLOW_AUTO_HEX_INT;

	duk_numconv_parse(ctx, 10 /*radix*/, s2n_flags);
	d = duk_get_number(ctx, -1);
	duk_pop(ctx);

	return d;
}

double duk_js_tonumber(duk_hthread *thr, duk_tval *tv) {
	duk_context *ctx = (duk_hthread *) thr;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED: {
		/* return a specific NaN (although not strictly necessary) */
		duk_double_union du;
		DUK_DBLUNION_SET_NAN(&du);
		DUK_ASSERT(DUK_DBLUNION_IS_NORMALIZED(&du));
		return du.d;
	}
	case DUK_TAG_NULL: {
		/* +0.0 */
		return 0.0;
	}
	case DUK_TAG_BOOLEAN: {
		if (DUK_TVAL_IS_BOOLEAN_TRUE(tv)) {
			return 1.0;
		}
		return 0.0;
	}
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		duk_push_hstring(ctx, h);
		return tonumber_string_raw(thr);
	}
	case DUK_TAG_OBJECT: {
		/* Note: ToPrimitive(object,hint) == [[DefaultValue]](object,hint),
		 * so use [[DefaultValue]] directly.
		 */
		double d;
		duk_push_tval(ctx, tv);
		duk_to_defaultvalue(ctx, -1, DUK_HINT_NUMBER);  /* 'tv' becomes invalid */

		/* recursive call for a primitive value (guaranteed not to cause second
		 * recursion).
		 */
		d = duk_js_tonumber(thr, duk_require_tval(ctx, -1));

		duk_pop(ctx);
		return d;
	}
	case DUK_TAG_BUFFER: {
		/* Coerce like a string.  This makes sense because addition also treats
		 * buffers like strings.
		 */
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		duk_push_hbuffer(ctx, h);
		duk_to_string(ctx, -1);  /* XXX: expensive, but numconv now expects to see a string */
		return tonumber_string_raw(thr);
	}
	case DUK_TAG_POINTER: {
		/* Coerce like boolean.  This allows code to do something like:
		 *
		 *    if (ptr) { ... }
		 */
		void *p = DUK_TVAL_GET_POINTER(tv);
		return (p != NULL ? 1.0 : 0.0);
	}
	default: {
		/* number */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return DUK_TVAL_GET_NUMBER(tv);
	}
	}

	DUK_UNREACHABLE();
}

/*
 *  ToInteger()  (E5 Section 9.4)
 */

/* exposed, used by e.g. duk_bi_date.c */
double duk_js_tointeger_number(double x) {
	int c = DUK_FPCLASSIFY(x);

	if (c == DUK_FP_NAN) {
		return 0.0;
	} else if (c == DUK_FP_ZERO || c == DUK_FP_INFINITE) {
		/* FIXME: FP_ZERO check can be removed, the else clause handles it
		 * correctly (preserving sign).
		 */
		return x;
	} else {
		int s = DUK_SIGNBIT(x);
		x = floor(fabs(x));  /* truncate towards zero */
		if (s) {
			x = -x;
		}
		return x;
	}
}

double duk_js_tointeger(duk_hthread *thr, duk_tval *tv) {
	double d = duk_js_tonumber(thr, tv);  /* invalidates tv */
	return duk_js_tointeger_number(d);
}

/*
 *  ToInt32(), ToUint32(), ToUint16()  (E5 Sections 9.5, 9.6, 9.7)
 */

/* combined algorithm matching E5 Sections 9.5 and 9.6 */	
static double toint32_or_touint32_helper(double x, int is_toint32) {
	int c = DUK_FPCLASSIFY(x);
	int s;

	if (c == DUK_FP_NAN || c == DUK_FP_ZERO || c == DUK_FP_INFINITE) {
		return 0.0;
	}


	/* x = sign(x) * floor(abs(x)), i.e. truncate towards zero, keep sign */
	s = DUK_SIGNBIT(x);
	x = floor(fabs(x));
	if (s) {
		x = -x;
	}
	
	/* NOTE: fmod(x) result sign is same as sign of x, which
	 * differs from what Javascript wants (see Section 9.6).
	 */

	x = fmod(x, DUK_DOUBLE_2TO32);    /* -> x in ]-2**32, 2**32[ */

	if (x < 0.0) {
		x += DUK_DOUBLE_2TO32;
	}
	/* -> x in [0, 2**32[ */

	if (is_toint32) {
		if (x >= DUK_DOUBLE_2TO31) {
			/* x in [2**31, 2**32[ */

			x -= DUK_DOUBLE_2TO32;  /* -> x in [-2**31,2**31[ */
		}
	}

	return x;
}

duk_int32_t duk_js_toint32(duk_hthread *thr, duk_tval *tv) {
	double d = duk_js_tonumber(thr, tv);  /* invalidates tv */
	d = toint32_or_touint32_helper(d, 1);
	DUK_ASSERT(DUK_FPCLASSIFY(d) == DUK_FP_ZERO || DUK_FPCLASSIFY(d) == DUK_FP_NORMAL);
	DUK_ASSERT(d >= -2147483648.0 && d <= 2147483647.0);  /* [-0x80000000,0x7fffffff] */
	DUK_ASSERT(d == ((double) ((duk_int32_t) d)));  /* whole, won't clip */
	return (duk_int32_t) d;
}


duk_uint32_t duk_js_touint32(duk_hthread *thr, duk_tval *tv) {
	double d = duk_js_tonumber(thr, tv);  /* invalidates tv */
	d = toint32_or_touint32_helper(d, 0);
	DUK_ASSERT(DUK_FPCLASSIFY(d) == DUK_FP_ZERO || DUK_FPCLASSIFY(d) == DUK_FP_NORMAL);
	DUK_ASSERT(d >= 0.0 && d <= 4294967295.0);  /* [0x00000000, 0xffffffff] */
	DUK_ASSERT(d == ((double) ((duk_uint32_t) d)));  /* whole, won't clip */
	return (duk_uint32_t) d;

}

duk_uint16_t duk_js_touint16(duk_hthread *thr, duk_tval *tv) {
	/* should be a safe way to compute this */
	return (duk_uint16_t) (duk_js_touint32(thr, tv) & 0x0000ffffU);
}

/*
 *  ToString()  (E5 Section 9.8)
 *
 *  ==> implemented in the API.
 */

/*
 *  ToObject()  (E5 Section 9.9)
 *
 *  ==> implemented in the API.
 */

/*
 *  CheckObjectCoercible()  (E5 Section 9.10)
 *
 *  Note: no API equivalent now.
 */

void duk_js_checkobjectcoercible(duk_hthread *thr, duk_tval *tv_x) {
	int tag = DUK_TVAL_GET_TAG(tv_x);

	/* Note: this must match ToObject() behavior */

	if (tag == DUK_TAG_UNDEFINED ||
	    tag == DUK_TAG_NULL ||
	    tag == DUK_TAG_POINTER ||
	    tag == DUK_TAG_BUFFER) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not object coercible");
	}
}

/*
 *  IsCallable()  (E5 Section 9.11)
 *
 *  FIXME: API equivalent is a separate implementation now, and this has
 *  currently no callers.
 */

int duk_js_iscallable(duk_tval *tv_x) {
	duk_hobject *obj;

	if (!DUK_TVAL_IS_OBJECT(tv_x)) {
		return 0;
	}
	obj = DUK_TVAL_GET_OBJECT(tv_x);
	DUK_ASSERT(obj != NULL);

	return DUK_HOBJECT_IS_CALLABLE(obj);
}

/*
 *  Loose equality, strict equality, and SameValue (E5 Sections 11.9.1, 11.9.4,
 *  9.12).  These have much in common so they can share some helpers.
 *
 *  FIXME notes:
 *
 *    - Current implementation (and spec definition) has recursion; this should
 *      be fixed if possible.
 *
 *    - String-to-number coercion should be possible without going through the
 *      value stack (and be more compact) if a shared helper is invoked.
 *
 *    - Non-standard coercion rules for internal types?  For instance:
 *
 *      + Pointer -> convert to number? or string?
 *      + Buffer vs. string -> compare contents
 *      + Buffer vs. buffer -> compare contents?  (not for strict mode)
 *
 *    - The beginning of loose and strict equality are identical: if the type
 *      tags are the same, comparison logic is the same -> implement a single
 *      helper with a strictness flag?
 *
 *    - SameValue and strict equals are identical except that zero signs are
 *      significant for SameValue but not for strict equals, so it can also go
 *      into the same helper.
 */

/* Note that this is the same operation for strict and loose equality:
 *  - E5 Section 11.9.3, step 1.c (loose)
 *  - E5 Section 11.9.6, step 4 (strict)
 */

int duk_js_equals_number(double x, double y) {
	int cx = DUK_FPCLASSIFY(x);
	int cy = DUK_FPCLASSIFY(y);

	if (cx == DUK_FP_NAN || cy == DUK_FP_NAN) {
		return 0;
	}

	/* FIXME: optimize */

	if (cx == DUK_FP_ZERO && cy == DUK_FP_ZERO) {
		return 1;
	}

	if (x == y) {
		return 1;
	}

	return 0;
}

/* E5 Section 11.9.3. */
int duk_js_equals(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y) {
	duk_context *ctx = (duk_context *) thr;
	duk_tval *tv_tmp;

	/*
	 *  XXX: very direct translation now - should be made more efficient,
	 *  avoid recursion, etc.
	 */

	/*
	 *  Same type?
	 *
	 *  Note: since number values have no explicit tag in the 8-byte
	 *  representation, need the awkward if + switch.
	 */

	if (DUK_TVAL_IS_NUMBER(tv_x) && DUK_TVAL_IS_NUMBER(tv_y)) {
		return duk_js_equals_number(DUK_TVAL_GET_NUMBER(tv_x),
		                            DUK_TVAL_GET_NUMBER(tv_y));
	} else if (DUK_TVAL_GET_TAG(tv_x) == DUK_TVAL_GET_TAG(tv_y)) {
		switch (DUK_TVAL_GET_TAG(tv_x)) {
		case DUK_TAG_UNDEFINED: {
			return 1;
		}
		case DUK_TAG_NULL: {
			return 1;
		}
		case DUK_TAG_BOOLEAN: {
			return DUK_TVAL_GET_BOOLEAN(tv_x) == DUK_TVAL_GET_BOOLEAN(tv_y);
		}
		case DUK_TAG_POINTER: {
			return DUK_TVAL_GET_POINTER(tv_x) == DUK_TVAL_GET_POINTER(tv_y);
		}
		case DUK_TAG_STRING:
		case DUK_TAG_OBJECT: {
			/* heap pointer comparison suffices */
			return DUK_TVAL_GET_HEAPHDR(tv_x) == DUK_TVAL_GET_HEAPHDR(tv_y);
		}
		case DUK_TAG_BUFFER: {
			/* non-strict equality for buffers compares contents */
			duk_hbuffer *h_x = DUK_TVAL_GET_BUFFER(tv_x);
			duk_hbuffer *h_y = DUK_TVAL_GET_BUFFER(tv_y);
			size_t len_x = DUK_HBUFFER_GET_SIZE(h_x);
			size_t len_y = DUK_HBUFFER_GET_SIZE(h_y);
			void *buf_x;
			void *buf_y;
			if (len_x != len_y) {
				return 0;
			}
			buf_x = (void *) DUK_HBUFFER_GET_DATA_PTR(h_x);
			buf_y = (void *) DUK_HBUFFER_GET_DATA_PTR(h_y);
			/* if len_x == len_y == 0, buf_x and/or buf_y may
			 * be NULL, but that's OK.
			 */
			DUK_ASSERT(len_x == len_y);
			DUK_ASSERT(len_x == 0 || buf_x != NULL);
			DUK_ASSERT(len_y == 0 || buf_y != NULL);
			return (DUK_MEMCMP(buf_x, buf_y, len_x) == 0) ? 1 : 0;
		}
		default: {
			DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_x));
			DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_y));
			DUK_UNREACHABLE();
			return 0;
		}
		}
	}

	/*
	 *  Types are different; various cases
	 *
	 *  Since comparison is symmetric, we use a "swap trick" to reduce
	 *  code size.
	 */

	/* Undefined/null are considered equal (e.g. "null == undefined" -> true). */
	if ((DUK_TVAL_IS_UNDEFINED(tv_x) && DUK_TVAL_IS_NULL(tv_y)) ||
	    (DUK_TVAL_IS_NULL(tv_x) && DUK_TVAL_IS_UNDEFINED(tv_y))) {
		return 1;
	}

	/* Number/string-or-buffer -> coerce string to number (e.g. "'1.5' == 1.5" -> true). */
	if (DUK_TVAL_IS_NUMBER(tv_x) && (DUK_TVAL_IS_STRING(tv_y) || DUK_TVAL_IS_BUFFER(tv_y))) {
		/* the next 'if' is guaranteed to match after swap */
		tv_tmp = tv_x;
		tv_x = tv_y;
		tv_y = tv_tmp;
	}
	if ((DUK_TVAL_IS_STRING(tv_x) || DUK_TVAL_IS_BUFFER(tv_x)) && DUK_TVAL_IS_NUMBER(tv_y)) {
		/* FIXME: this is possible without resorting to the value stack */
		double d1, d2;
		d2 = DUK_TVAL_GET_NUMBER(tv_y);
		duk_push_tval(ctx, tv_x);
		duk_to_string(ctx, -1);  /* buffer values are coerced first to string here */
		duk_to_number(ctx, -1);
		d1 = duk_require_number(ctx, -1);
		duk_pop(ctx);
		return duk_js_equals_number(d1, d2);
	}

	/* Buffer/string -> compare contents. */
	if (DUK_TVAL_IS_BUFFER(tv_x) && DUK_TVAL_IS_STRING(tv_y)) {
		tv_tmp = tv_x;
		tv_x = tv_y;
		tv_y = tv_tmp;
	}
	if (DUK_TVAL_IS_STRING(tv_x) && DUK_TVAL_IS_BUFFER(tv_y)) {
		duk_hstring *h_x = DUK_TVAL_GET_STRING(tv_x);
		duk_hbuffer *h_y = DUK_TVAL_GET_BUFFER(tv_y);
		size_t len_x = DUK_HSTRING_GET_BYTELEN(h_x);
		size_t len_y = DUK_HBUFFER_GET_SIZE(h_y);
		void *buf_x;
		void *buf_y;
		if (len_x != len_y) {
			return 0;
		}
		buf_x = (void *) DUK_HSTRING_GET_DATA(h_x);
		buf_y = (void *) DUK_HBUFFER_GET_DATA_PTR(h_y);
		/* if len_x == len_y == 0, buf_x and/or buf_y may
		 * be NULL, but that's OK.
		 */
		DUK_ASSERT(len_x == len_y);
		DUK_ASSERT(len_x == 0 || buf_x != NULL);
		DUK_ASSERT(len_y == 0 || buf_y != NULL);
		return (DUK_MEMCMP(buf_x, buf_y, len_x) == 0) ? 1 : 0;
	}

	/* Boolean/any -> coerce boolean to number and try again.  If boolean is
	 * compared to a pointer, the final comparison after coercion now always
	 * yields false (as pointer vs. number compares to false), but this is
	 * not special cased.
	 */
	if (DUK_TVAL_IS_BOOLEAN(tv_x)) {
		tv_tmp = tv_x;
		tv_x = tv_y;
		tv_y = tv_tmp;
	}
	if (DUK_TVAL_IS_BOOLEAN(tv_y)) {
		/* ToNumber(bool) is +1.0 or 0.0.  Tagged boolean value is always 0 or 1. */
		int rc;
		DUK_ASSERT(DUK_TVAL_GET_BOOLEAN(tv_y) == 0 || DUK_TVAL_GET_BOOLEAN(tv_y) == 1);
		duk_push_tval(ctx, tv_x);
		duk_push_int(ctx, DUK_TVAL_GET_BOOLEAN(tv_y));
		rc = duk_js_equals(thr, duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
		duk_pop_2(ctx);
		return rc;
	}

	/* String-number-buffer/object -> coerce object to primitive (apparently without hint), then try again. */
	if ((DUK_TVAL_IS_STRING(tv_x) || DUK_TVAL_IS_NUMBER(tv_x) || DUK_TVAL_IS_BUFFER(tv_x)) &&
	    DUK_TVAL_IS_OBJECT(tv_y)) {
		tv_tmp = tv_x;
		tv_x = tv_y;
		tv_y = tv_tmp;
	}
	if (DUK_TVAL_IS_OBJECT(tv_x) &&
	    (DUK_TVAL_IS_STRING(tv_y) || DUK_TVAL_IS_NUMBER(tv_y) || DUK_TVAL_IS_BUFFER(tv_y))) {
		int rc;
		duk_push_tval(ctx, tv_x);
		duk_push_tval(ctx, tv_y);
		duk_to_primitive(ctx, -2, DUK_HINT_NONE);  /* apparently no hint? */
		rc = duk_js_equals(thr, duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
		duk_pop_2(ctx);
		return rc;
	}

	/* Nothing worked -> not equal. */
	return 0;
}

/* E5 Section 11.9.4 */
int duk_js_strict_equals(duk_tval *tv_x, duk_tval *tv_y) {
	/*
	 *  Same type?
	 *
	 *  Note: since number values have no explicit tag, need the awkward
	 *  if + switch.
	 */

	if (DUK_TVAL_IS_NUMBER(tv_x) && DUK_TVAL_IS_NUMBER(tv_y)) {
		/* Note: comparison of numbers is identical for loose and strict
		 * equality, hence use loose helper.
		 */
		return duk_js_equals_number(DUK_TVAL_GET_NUMBER(tv_x),
		                            DUK_TVAL_GET_NUMBER(tv_y));
	} else if (DUK_TVAL_GET_TAG(tv_x) == DUK_TVAL_GET_TAG(tv_y)) {
		switch (DUK_TVAL_GET_TAG(tv_x)) {
		case DUK_TAG_UNDEFINED: {
			return 1;
		}
		case DUK_TAG_NULL: {
			return 1;
		}
		case DUK_TAG_BOOLEAN: {
			return DUK_TVAL_GET_BOOLEAN(tv_x) == DUK_TVAL_GET_BOOLEAN(tv_y);
		}
		case DUK_TAG_POINTER: {
			return DUK_TVAL_GET_POINTER(tv_x) == DUK_TVAL_GET_POINTER(tv_y);
		}
		case DUK_TAG_STRING:
		case DUK_TAG_OBJECT:
		case DUK_TAG_BUFFER: {
			/* heap pointer comparison suffices */
			return DUK_TVAL_GET_HEAPHDR(tv_x) == DUK_TVAL_GET_HEAPHDR(tv_y);
		}
		default: {
			DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_x));
			DUK_UNREACHABLE();
			return 0;
		}
		}
	}

	/*
	 *  Types are different -> strict equality always false
	 */

	return 0;
}

/* E5 Section 9.12 */
int duk_js_samevalue_number(double x, double y) {
	int cx = DUK_FPCLASSIFY(x);
	int cy = DUK_FPCLASSIFY(y);

	if (cx == DUK_FP_NAN && cy == DUK_FP_NAN) {
		/* SameValue(NaN, NaN) = true, regardless of NaN sign or extra bits */
		return 1;
	}

	if (cx == DUK_FP_ZERO && cy == DUK_FP_ZERO) {
		/* Note: cannot assume that a non-zero return value of signbit() would
		 * always be the same -- hence cannot (portably) use something like:
		 *
		 *     signbit(x) == signbit(y)
		 */

		int sx = (DUK_SIGNBIT(x) ? 1 : 0);
		int sy = (DUK_SIGNBIT(y) ? 1 : 0);

		return (sx == sy);
	}

	/* normal comparison; known:
	 *   - both x and y are not NaNs (but one of them can be)
	 *   - both x and y are not zero (but one of them can be)
	 *   - x and y may be denormal or infinite
	 */

	return (x == y);
}

int duk_js_samevalue(duk_tval *tv_x, duk_tval *tv_y) {
	/*
	 *  Same type?
	 *
	 *  Note: since number values have no explicit tag, need the awkward
	 *  if + switch.
	 */

	if (DUK_TVAL_IS_NUMBER(tv_x) && DUK_TVAL_IS_NUMBER(tv_y)) {
		return duk_js_samevalue_number(DUK_TVAL_GET_NUMBER(tv_x),
		                               DUK_TVAL_GET_NUMBER(tv_y));
	} else if (DUK_TVAL_GET_TAG(tv_x) == DUK_TVAL_GET_TAG(tv_y)) {
		switch (DUK_TVAL_GET_TAG(tv_x)) {
		case DUK_TAG_UNDEFINED: {
			return 1;
		}
		case DUK_TAG_NULL: {
			return 1;
		}
		case DUK_TAG_BOOLEAN: {
			return DUK_TVAL_GET_BOOLEAN(tv_x) == DUK_TVAL_GET_BOOLEAN(tv_y);
		}
		case DUK_TAG_POINTER: {
			return DUK_TVAL_GET_POINTER(tv_x) == DUK_TVAL_GET_POINTER(tv_y);
		}
		case DUK_TAG_STRING:
		case DUK_TAG_OBJECT:
		case DUK_TAG_BUFFER: {
			/* heap pointer comparison suffices */
			return DUK_TVAL_GET_HEAPHDR(tv_x) == DUK_TVAL_GET_HEAPHDR(tv_y);
		}
		default: {
			DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_x));
			DUK_UNREACHABLE();
			return 0;
		}
		}
	}

	/*
	 *  Types are different -> strict equality always false
	 */

	return 0;
}

/*
 *  Comparisons (x >= y, x > y, x <= y, x < y)
 *
 *  E5 Section 11.8.5: implement 'x < y' and then use negate and eval_left_first
 *  flags to get the rest.
 */

/* FIXME: join flags into one integer argument? */

/* FIXME: this should probably just operate on the stack top, because it
 * needs to push stuff on the stack anyway...
 */

int duk_js_string_compare(duk_hstring *h1, duk_hstring *h2) {
	/*
	 *  String comparison (E5 Section 11.8.5, step 4), which
	 *  needs to compare codepoint by codepoint.
	 *
	 *  However, UTF-8 allows us to use strcmp directly: the shared
	 *  prefix will be encoded identically (UTF-8 has unique encoding)
	 *  and the first differing character can be compared with a simple
	 *  unsigned byte comparison (which strcmp does).
	 *
	 *  This will not work properly for non-xutf-8 strings, but this
	 *  is not an issue for compliance.
	 */

	size_t h1_len, h2_len, prefix_len;
	int rc;

	DUK_ASSERT(h1 != NULL);
	DUK_ASSERT(h2 != NULL);
	h1_len = DUK_HSTRING_GET_BYTELEN(h1);
	h2_len = DUK_HSTRING_GET_BYTELEN(h2);
	prefix_len = (h1_len <= h2_len ? h1_len : h2_len);

	/* FIXME: this special case can now be removed with DUK_MEMCMP */
	/* memcmp() should return zero (equal) for zero length, but avoid
	 * it because there are some platform specific bugs.  Don't use
	 * strncmp() because it stops comparing at a NUL.
	 */

	if (prefix_len == 0) {
		rc = 0;
	} else {
		rc = DUK_MEMCMP((const char *) DUK_HSTRING_GET_DATA(h1),
		                (const char *) DUK_HSTRING_GET_DATA(h2),
		                prefix_len);
	}

	if (rc < 0) {
		return -1;
	} else if (rc > 0) {
		return 1;
	}

	/* prefix matches, lengths matter now */
	if (h1_len < h2_len) {
		/* e.g. "x" < "xx" */
		return -1;
	} else if (h1_len > h2_len) {
		return 1;
	}

	return 0;
}

int duk_js_compare_helper(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y, int eval_left_first, int negate) {
	duk_context *ctx = (duk_context *) thr;
	double d1, d2;
	int c1, c2;
	int s1, s2;
	int rc;
	int retval;

	duk_push_tval(ctx, tv_x);
	duk_push_tval(ctx, tv_y);

	if (eval_left_first) {
		duk_to_primitive(ctx, -2, DUK_HINT_NUMBER);
		duk_to_primitive(ctx, -1, DUK_HINT_NUMBER);
	} else {
		duk_to_primitive(ctx, -1, DUK_HINT_NUMBER);
		duk_to_primitive(ctx, -2, DUK_HINT_NUMBER);
	}

	/* Note: reuse variables */
	tv_x = duk_get_tval(ctx, -2);
	tv_y = duk_get_tval(ctx, -1);

	if (DUK_TVAL_IS_STRING(tv_x) && DUK_TVAL_IS_STRING(tv_y)) {
		duk_hstring *h1 = DUK_TVAL_GET_STRING(tv_x);
		duk_hstring *h2 = DUK_TVAL_GET_STRING(tv_y);
		DUK_ASSERT(h1 != NULL);
		DUK_ASSERT(h2 != NULL);

		rc = duk_js_string_compare(h1, h2);
		if (rc < 0) {
			goto lt_true;
		} else {
			goto lt_false;
		}
	} else {
		/* Ordering should not matter (E5 Section 11.8.5, step 3.a) but
		 * preserve it just in case.
		 */

		if (eval_left_first) {
			d1 = duk_to_number(ctx, -2);
			d2 = duk_to_number(ctx, -1);
		} else {
			d2 = duk_to_number(ctx, -1);
			d1 = duk_to_number(ctx, -2);
		}

		c1 = DUK_FPCLASSIFY(d1);
		s1 = DUK_SIGNBIT(d1);
		c2 = DUK_FPCLASSIFY(d2);
		s2 = DUK_SIGNBIT(d2);

		if (c1 == DUK_FP_NAN || c2 == DUK_FP_NAN) {
			goto lt_undefined;
		}

		if (c1 == DUK_FP_ZERO && c2 == DUK_FP_ZERO) {
			/* For all combinations: +0 < +0, +0 < -0, -0 < +0, -0 < -0,
			 * steps e, f, and g.
			 */
			goto lt_false;
		}

		if (d1 == d2) {
			goto lt_false;
		}

		if (c1 == DUK_FP_INFINITE && s1 == 0) {
			/* x == +Infinity */
			goto lt_false;
		}

		if (c2 == DUK_FP_INFINITE && s2 == 0) {
			/* y == +Infinity */
			goto lt_true;
		}

		if (c2 == DUK_FP_INFINITE && s2 != 0) {
			/* y == -Infinity */
			goto lt_false;
		}

		if (c1 == DUK_FP_INFINITE && s1 != 0) {
			/* x == -Infinity */
			goto lt_true;
		}

		if (d1 < d2) {
			goto lt_true;
		}

		goto lt_false;
	}

 lt_undefined:
	/* Note: undefined from Section 11.8.5 always results in false
	 * return (see e.g. Section 11.8.3) - hence special treatment here.
	 */
	retval = 0;
	goto cleanup;

 lt_true:
	if (negate) {
		retval = 0;
		goto cleanup;
	} else {
		retval = 1;
		goto cleanup;
	}
	/* never here */

 lt_false:
	if (negate) {
		retval = 1;
		goto cleanup;
	} else {
		retval = 0;
		goto cleanup;
	}
	/* never here */

 cleanup:
	duk_pop_2(ctx);
	return retval;
}

/* FIXME: remove these? or make them macros? */
#if 0  /* unused */
/* E5 Sections 11.8.1, 11.8.5 */
int duk_js_lessthan(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y) {
	/* x < y */
	return duk_js_compare_helper(thr, tv_x, tv_y, 1, 0);
}

/* E5 Sections 11.8.2, 11.8.5 */
int duk_js_greaterthan(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y) {
	/* x > y  -->  y < x */
	return duk_js_compare_helper(thr, tv_y, tv_x, 0, 0);
}

/* E5 Sections 11.8.3, 11.8.5 */
int duk_js_lessthanorequal(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y) {
	/* x <= y  -->  not (x > y)  -->  not (y < x) */
	return duk_js_compare_helper(thr, tv_y, tv_x, 0, 1);
}

/* E5 Sections 11.8.4, 11.8.5 */
int duk_js_greaterthanorequal(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y) {
	/* x >= y  -->  not (x < y) */
	return duk_js_compare_helper(thr, tv_x, tv_y, 1, 1);
}
#endif

/*
 *  instanceof
 */

/*
 *  E5 Section 11.8.6 describes the main algorithm, which uses
 *  [[HasInstance]].  [[HasInstance]] is defined for only
 *  function objects:
 *
 *    - Normal functions:
 *      E5 Section 15.3.5.3
 *    - Functions established with Function.prototype.bind():
 *      E5 Section 15.3.4.5.3
 *
 *  For other objects, a TypeError is thrown.
 *
 *  FIXME: TypeError descriptions are bad (automatic from API).
 *
 */

/* FIXME: refactoring -> helper to extract duk_hobject * from a tval would
 * be useful here; that function should throw TypeErrors if type expectations
 * are incorrect.
 */

int duk_js_instanceof(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y) {
	duk_context *ctx = (duk_context *) thr;
	duk_hobject *func;
	duk_hobject *val;
	duk_hobject *proto;
	duk_uint32_t sanity;

	/*
	 *  Get the values onto the stack first.  It would be possible to cover
	 *  some normal cases without resorting to the value stack.
	 */

	duk_push_tval(ctx, tv_x);
	duk_push_tval(ctx, tv_y);
	func = duk_require_hobject(ctx, -1);

	/*
	 *  For bound objects, [[HasInstance]] just calls the target function
	 *  [[HasInstance]].  If that is again a bound object, repeat until
	 *  we find a non-bound Function object.
	 */

	/* FIXME: this bound function resolution also happens elsewhere,
	 * move into a shared helper.
	 */

	sanity = DUK_HOBJECT_BOUND_CHAIN_SANITY;
	do {
		DUK_ASSERT(DUK_HOBJECT_IS_CALLABLE(func));

		/* check func supports [[HasInstance]] (this is checked for every function
		 * in the bound chain, including the final one)
		 */

		if (!DUK_HOBJECT_IS_CALLABLE(func)) {
			/*
		 	 *  Note: of native Ecmascript objects, only Function instances
			 *  have a [[HasInstance]] internal property.  Custom objects might
			 *  also have it, but not in current implementation.
			 *
			 *  XXX: add a separate flag, DUK_HOBJECT_FLAG_ALLOW_INSTANCEOF?
			 */
			DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "instanceof rval does not support [[HasInstance]]");
		}

		if (!DUK_HOBJECT_HAS_BOUND(func)) {
			break;
		}

		/* [ ... lval rval ] */

		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_TARGET);         /* -> [ ... lval rval new_rval ] */
		duk_replace(ctx, -1);                                        /* -> [ ... lval new_rval ] */
		func = duk_require_hobject(ctx, -1);

		/* func support for [[HasInstance]] checked in the beginning of the loop */
	} while (--sanity > 0);

	if (sanity == 0) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "instanceof bound chain sanity exceeded");
	}

	/*
	 *  'func' is now a non-bound object which supports [[HasInstance]]
	 *  (which here just means DUK_HOBJECT_FLAG_CALLABLE).  Move on
	 *  to execute E5 Section 15.3.5.3.
	 */

	DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(func));
	DUK_ASSERT(DUK_HOBJECT_IS_CALLABLE(func));

	/* [ ... lval rval(func) ] */

	val = duk_get_hobject(ctx, -2);
	if (!val) {
		goto pop_and_false;
	}

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_PROTOTYPE);  /* -> [ ... lval rval rval.prototype ] */
	proto = duk_require_hobject(ctx, -1);
	duk_pop(ctx);  /* -> [ ... lval rval ] */

	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	do {
		/*
		 *  Note: prototype chain is followed BEFORE first comparison.  This
		 *  means that the instanceof lval is never itself compared to the
		 *  rval.prototype property.  This is apparently intentional, see E5
		 *  Section 15.3.5.3, step 4.a.
		 *
		 *  Also note:
		 *
		 *      js> (function() {}) instanceof Function
		 *      true
		 *      js> Function instanceof Function
		 *      true
		 *
		 *  For the latter, h_proto will be Function.prototype, which is the
		 *  built-in Function prototype.  Because Function.[[Prototype]] is
		 *  also the built-in Function prototype, the result is true.
		 */

		val = val->prototype;

		if (!val) {
			goto pop_and_false;
		} else if (val == proto) {
			goto pop_and_true;
		}

		/* follow prototype chain */
	} while (--sanity > 0);

	if (sanity == 0) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "instanceof prototype chain sanity exceeded");
	}
	DUK_UNREACHABLE();

 pop_and_false:
	duk_pop_2(ctx);
	return 0;

 pop_and_true:
	duk_pop_2(ctx);
	return 1;
}

/*
 *  in
 */

/*
 *  E5 Sections 11.8.7, 8.12.6.
 *
 *  Basically just a property existence check using [[HasProperty]].
 *
 *  FIXME: TypeError descriptions are bad (automatic from API).
 */
	
int duk_js_in(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y) {
	duk_context *ctx = (duk_context *) thr;
	int retval;

	/*
	 *  Get the values onto the stack first.  It would be possible to cover
	 *  some normal cases without resorting to the value stack (e.g. if
	 *  lval is already a string).
	 */

	duk_push_tval(ctx, tv_x);
	duk_push_tval(ctx, tv_y);
	(void) duk_require_hobject(ctx, -1);  /* TypeError if rval not object */
	duk_to_string(ctx, -2);               /* coerce lval with ToString() */

	retval = duk_hobject_hasprop(thr, duk_get_tval(ctx, -1), duk_get_tval(ctx, -2));

	duk_pop_2(ctx);
	return retval;
}

/*
 *  typeof
 *
 *  E5 Section 11.4.3.
 *
 *  Very straightforward.  The only question is what to return for our
 *  non-standard tag / object types.
 *
 *  There is an unfortunate string constant define naming problem with
 *  typeof return values for e.g. "Object" and "object"; careful with
 *  the built-in string defines.  The LC_XXX defines are used for the
 *  lowercase variants now.
 */

duk_hstring *duk_js_typeof(duk_hthread *thr, duk_tval *tv_x) {
	int idx = 0;

	switch (DUK_TVAL_GET_TAG(tv_x)) {
	case DUK_TAG_UNDEFINED: {
		idx = DUK_STRIDX_UNDEFINED;
		break;
	}
	case DUK_TAG_NULL: {
		/* Note: not a typo, "object" is returned for a null value */
		idx = DUK_STRIDX_LC_OBJECT;
		break;
	}
	case DUK_TAG_BOOLEAN: {
		idx = DUK_STRIDX_LC_BOOLEAN;
		break;
	}
	case DUK_TAG_POINTER: {
		/* implementation specific */
		idx = DUK_STRIDX_LC_POINTER;
		break;
	}
	case DUK_TAG_STRING: {
		idx = DUK_STRIDX_LC_STRING;
		break;
	}
	case DUK_TAG_OBJECT: {
		duk_hobject *obj = DUK_TVAL_GET_OBJECT(tv_x);
		DUK_ASSERT(obj != NULL);
		if (DUK_HOBJECT_IS_CALLABLE(obj)) {
			idx = DUK_STRIDX_LC_FUNCTION;
		} else {
			idx = DUK_STRIDX_LC_OBJECT;
		}
		break;
	}
	case DUK_TAG_BUFFER: {
		/* implementation specific */
		idx = DUK_STRIDX_LC_BUFFER;
		break;
	}
	default: {
		/* number */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_x));
		idx = DUK_STRIDX_LC_NUMBER;
		break;
	}
	}

	DUK_ASSERT(idx >= 0 && idx < DUK_HEAP_NUM_STRINGS);
	return thr->strs[idx];
}

/*
 *  Array index and length
 *
 *  Array index: E5 Section 15.4
 *  Array length: E5 Section 15.4.5.1 steps 3.c - 3.d (array length write)
 *
 *  The DUK_HSTRING_GET_ARRIDX_SLOW() and DUK_HSTRING_GET_ARRIDX_FAST() macros
 *  call duk_js_to_arrayindex_string_helper().
 */

duk_small_int_t duk_js_to_arrayindex_raw_string(duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t *out_idx) {
	duk_uint32_t res, new_res;

	if (blen == 0 || blen > 10) {
		goto parse_fail;
	}
	if (str[0] == (duk_uint8_t) '0' && blen > 1) {
		goto parse_fail;
	}

	/* Accept 32-bit decimal integers, no leading zeroes, signs, etc.
	 * Leading zeroes are not accepted (zero index "0" is an exception
	 * handled above).
	 */

	res = 0;
	while (blen-- > 0) {
		duk_uint8_t c = *str++;
		if (c >= (duk_uint8_t) '0' && c <= (duk_uint8_t) '9') {
			new_res = res * 10 + (duk_uint32_t) (c - (duk_uint8_t) '0');
			if (new_res < res) {
				/* overflow, more than 32 bits -> not an array index */
				goto parse_fail;
			}
			res = new_res;
		} else {
			goto parse_fail;
		}
	}

	*out_idx = res;
	return 1;

 parse_fail:
	*out_idx = DUK_HSTRING_NO_ARRAY_INDEX;
	return 0;
}	

/* Called by duk_hstring.h macros */
duk_uint32_t duk_js_to_arrayindex_string_helper(duk_hstring *h) {
	duk_uint32_t res;
	duk_small_int_t rc;

	if (!DUK_HSTRING_HAS_ARRIDX(h)) {
		return DUK_HSTRING_NO_ARRAY_INDEX;
	}

	rc = duk_js_to_arrayindex_raw_string(DUK_HSTRING_GET_DATA(h),
	                                     DUK_HSTRING_GET_BYTELEN(h),
	                                     &res);
	DUK_UNREF(rc);
	DUK_ASSERT(rc != 0);
	return res;
}

