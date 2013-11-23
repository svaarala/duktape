/*
 *  Tagged type definition (duk_tval) and accessor macros.
 *
 *  Access all fields through the accessor macros, as the representation
 *  is quite tricky.
 *
 *  There are two packed type alternatives: an 8-byte representation
 *  based on an IEEE double (preferred for compactness), and a 12-byte
 *  representation (portability).  The latter is needed also in e.g.
 *  64-bit environments (it may pad to 16 bytes per value).
 *
 *  Selecting the tagged type format involves many trade-offs (memory
 *  use, size and performance of generated code, portability, etc),
 *  see doc/types.txt for a detailed discussion (especially of how the
 *  IEEE double format is used to pack tagged values).
 *
 *  NB: because macro arguments are often expressions, macros should
 *  avoid evaluating their argument more than once.
 */

/*
 *  IEEE double format summary:
 *
 *    seeeeeee eeeeffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff
 *       A        B        C        D        E        F        G        H
 *
 *    s       sign bit
 *    eee...  exponent field
 *    fff...  fraction
 *
 *  See http://en.wikipedia.org/wiki/Double_precision_floating-point_format.
 *
 *  At least three memory layouts are relevant here:
 *
 *    A B C D E F G H    Big endian (e.g. 68k)           USE__BE_VARIANT
 *    H G F E D C B A    Little endian (e.g. x86)        USE__LE_VARIANT
 *    D C B A H G F E    Middle/cross endian (e.g. ARM)  USE__ME_VARIANT
 *
 *  ARM is a special case: ARM double values are in middle/cross endian
 *  format while ARM duk_uint64_t values are in standard little endian
 *  format (H G F E D C B A).  When a double is read as a duk_uint64_t
 *  from memory, the register will contain the (logical) value
 *  E F G H A B C D.  This requires some special handling below.
 *
 *  Indexes of various types (8-bit, 16-bit, 32-bit) in memory relative to
 *  the logical (big endian) order:
 *
 *  byte order      duk_uint8_t    duk_uint16_t     duk_uint32_t    
 *    BE             01234567         0123               01
 *    LE             76543210         3210               10
 *    ME (ARM)       32107654         1032               01
 */

#ifndef DUK_TVAL_H_INCLUDED
#define DUK_TVAL_H_INCLUDED

#ifdef DUK_USE_DOUBLE_LE
#define  USE__LE_VARIANT
#endif
#ifdef DUK_USE_DOUBLE_ME
#define  USE__ME_VARIANT
#endif
#ifdef DUK_USE_DOUBLE_BE
#define  USE__BE_VARIANT
#endif
#ifdef DUK_USE_64BIT_OPS
#define  USE__64BIT
#endif

/* sanity */
#if !defined(USE__LE_VARIANT) && !defined(USE__ME_VARIANT) && !defined(USE__BE_VARIANT)
#error unsupported: cannot determine byte order variant
#endif

#ifdef DUK_USE_PACKED_TVAL
/* ======================================================================== */

/*
 *  Packed 8-byte representation
 */

/* sanity */
#if !defined(DUK_USE_PACKED_TVAL_POSSIBLE)
#error packed representation not supported
#endif

/* use duk_double_union as duk_tval directly */
typedef union duk_double_union duk_tval;

/* tags */
#define  DUK_TAG_NORMALIZED_NAN    0x7ff8UL   /* the NaN variant we use */
/* avoid tag 0xfff0, no risk of confusion with negative infinity */
#define  DUK_TAG_UNDEFINED         0xfff1UL   /* embed: 0 or 1 (normal or unused) */
#define  DUK_TAG_NULL              0xfff2UL   /* embed: nothing */
#define  DUK_TAG_BOOLEAN           0xfff3UL   /* embed: 0 or 1 (false or true) */
/* DUK_TAG_NUMBER would logically go here, but it has multiple 'tags' */
#define  DUK_TAG_POINTER           0xfff4UL   /* embed: void ptr */
#define  DUK_TAG_STRING            0xfff5UL   /* embed: duk_hstring ptr */
#define  DUK_TAG_OBJECT            0xfff6UL   /* embed: duk_hobject ptr */
#define  DUK_TAG_BUFFER            0xfff7UL   /* embed: duk_hbuffer ptr */

/* for convenience */
#define  DUK_XTAG_UNDEFINED_ACTUAL 0xfff10000UL
#define  DUK_XTAG_UNDEFINED_UNUSED 0xfff10001UL
#define  DUK_XTAG_NULL             0xfff20000UL
#define  DUK_XTAG_BOOLEAN_FALSE    0xfff30000UL
#define  DUK_XTAG_BOOLEAN_TRUE     0xfff30001UL

/*
 *  The ME variant below is specifically for ARM byte order, which has the
 *  feature that while doubles have a mixed byte order (32107654), unsigned
 *  long long values has a little endian byte order (76543210).  When writing
 *  a logical double value through a ULL pointer, the 32-bit words need to be
 *  swapped; hence the #ifdefs below for ULL writes with USE__ME_VARIANT.
 *  This is not full ARM support but suffices for some environments.
 */

/* raw setters */
#ifdef USE__64BIT
#ifdef USE__ME_VARIANT
#define  DUK__TVAL_SET_HIGH32_FULL(v,x)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = (duk_uint64_t) (x); \
	} while (0)
#else
#define  DUK__TVAL_SET_HIGH32_FULL(v,x)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = ((duk_uint64_t) (x)) << 32; \
	} while (0)
#endif
#else  /* USE__64BIT */
#define  DUK__TVAL_SET_HIGH32_FULL(v,x)  do { \
		(v)->ui[DUK_DBL_IDX_UI0] = (duk_uint32_t) (x); \
		(v)->ui[DUK_DBL_IDX_UI1] = (duk_uint32_t) 0; \
	} while (0)
#endif  /* USE__64BIT */

#define  DUK__TVAL_SET_UNDEFINED_ACTUAL_FULL(v)  DUK__TVAL_SET_HIGH32((v), DUK_XTAG_UNDEFINED_ACTUAL)
#define  DUK__TVAL_SET_UNDEFINED_ACTUAL_NOTFULL(v)  do { \
		(v)->ui[DUK_DBL_IDX_UI0] = (duk_uint32_t) DUK_XTAG_UNDEFINED_ACTUAL; \
	} while (0)

#define  DUK__TVAL_SET_UNDEFINED_UNUSED_FULL(v)  DUK__TVAL_SET_HIGH32((v), DUK_XTAG_UNDEFINED_UNUSED)
#define  DUK__TVAL_SET_UNDEFINED_UNUSED_NOTFULL(v)  do { \
		(v)->ui[DUK_DBL_IDX_UI0] = (duk_uint32_t) DUK_XTAG_UNDEFINED_UNUSED; \
	} while (0)

/* Note: 16-bit initializer suffices (unlike for undefined/boolean) */
#define  DUK__TVAL_SET_NULL_FULL(v)  DUK__TVAL_SET_HIGH32((v), DUK_XTAG_NULL)
#define  DUK__TVAL_SET_NULL_NOTFULL(v)  do { \
		(v)->us[DUK_DBL_IDX_US0] = (duk_uint16_t) DUK_TAG_NULL; \
	} while (0)

#define  DUK__TVAL_SET_BOOLEAN_FULL(v,val)  DUK__TVAL_SET_HIGH32((v), (((duk_uint32_t) DUK_TAG_BOOLEAN) << 16) | ((duk_uint32_t) val))
#define  DUK__TVAL_SET_BOOLEAN_NOTFULL(v,val)  do { \
		DUK_ASSERT((val) == 0 || (val) == 1); \
		(v)->ui[DUK_DBL_IDX_UI0] = (((duk_uint32_t) DUK_TAG_BOOLEAN) << 16) | ((duk_uint32_t) (val)); \
	} while (0)

/* assumes that caller has normalized a possible NaN value of 'val', otherwise trouble ahead */
#define  DUK__TVAL_SET_NUMBER_FULL(v,val)  do { \
		(v)->d = (double) (val); \
	} while (0)

/* no notfull variant */
#define  DUK__TVAL_SET_NUMBER_NOTFULL(v,d)  DUK__TVAL_SET_NUMBER_FULL(v,d)

/* two casts to avoid gcc warning: "warning: cast from pointer to integer of different size [-Wpointer-to-int-cast]" */
#ifdef USE__64BIT
#ifdef USE__ME_VARIANT
#define  DUK__TVAL_SET_TAGGEDPOINTER(v,h,tag)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = (((duk_uint64_t) (tag)) << 16) | (((duk_uint64_t) (duk_uint32_t) (h)) << 32); \
	} while (0)
#else
#define  DUK__TVAL_SET_TAGGEDPOINTER(v,h,tag)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = (((duk_uint64_t) (tag)) << 48) | ((duk_uint64_t) (duk_uint32_t) (h)); \
	} while (0)
#endif
#else  /* USE__64BIT */
#define  DUK__TVAL_SET_TAGGEDPOINTER(v,h,tag)  do { \
		(v)->ui[DUK_DBL_IDX_UI0] = ((duk_uint32_t) (tag)) << 16; \
		(v)->ui[DUK_DBL_IDX_UI1] = (duk_uint32_t) (h); \
	} while (0)
#endif  /* USE__64BIT */

#ifdef USE__64BIT
#ifdef USE__ME_VARIANT
#define  DUK__TVAL_SET_NAN_FULL(v)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = 0x000000007ff80000ULL; \
	} while (0)
#else
#define  DUK__TVAL_SET_NAN_FULL(v)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = 0x7ff8000000000000ULL; \
	} while (0)
#endif
#else  /* USE__64BIT */
#define  DUK__TVAL_SET_NAN_FULL(v)  do { \
		(v)->ui[DUK_DBL_IDX_UI0] = (duk_uint32_t) 0x7ff80000UL; \
		(v)->ui[DUK_DBL_IDX_UI1] = (duk_uint32_t) 0x00000000UL; \
	} while (0)
#endif  /* USE__64BIT */

#define  DUK__TVAL_SET_NAN_NOTFULL(v)  do { \
		(v)->us[DUK_DBL_IDX_US0] = 0x7ff8UL; \
	} while (0)

#define  DUK__DOUBLE_SET_NAN_FULL(d)         DUK__TVAL_SET_NAN_FULL((duk_tval *)(d))
#define  DUK__DOUBLE_SET_NAN_NOTFULL(d)      DUK__TVAL_SET_NAN_NOTFULL((duk_tval *)(d))

/* select actual setters */
#ifdef DUK_USE_FULL_TVAL
#define  DUK_TVAL_SET_UNDEFINED_ACTUAL(v)    DUK__TVAL_SET_UNDEFINED_ACTUAL_FULL((v))
#define  DUK_TVAL_SET_UNDEFINED_UNUSED(v)    DUK__TVAL_SET_UNDEFINED_UNUSED_FULL((v))
#define  DUK_TVAL_SET_NULL(v)                DUK__TVAL_SET_NULL_FULL((v))
#define  DUK_TVAL_SET_BOOLEAN(v,i)           DUK__TVAL_SET_BOOLEAN_FULL((v),(i))
#define  DUK_TVAL_SET_NUMBER(v,d)            DUK__TVAL_SET_NUMBER_FULL((v),(d))
#define  DUK_TVAL_SET_NAN(v)                 DUK__TVAL_SET_NAN_FULL((v))
#define  DUK_DOUBLE_SET_NAN(d)               DUK__DOUBLE_SET_NAN_FULL((d))
#else
#define  DUK_TVAL_SET_UNDEFINED_ACTUAL(v)    DUK__TVAL_SET_UNDEFINED_ACTUAL_NOTFULL((v))
#define  DUK_TVAL_SET_UNDEFINED_UNUSED(v)    DUK__TVAL_SET_UNDEFINED_UNUSED_NOTFULL((v))
#define  DUK_TVAL_SET_NULL(v)                DUK__TVAL_SET_NULL_NOTFULL((v))
#define  DUK_TVAL_SET_BOOLEAN(v,i)           DUK__TVAL_SET_BOOLEAN_NOTFULL((v),(i))
#define  DUK_TVAL_SET_NUMBER(v,d)            DUK__TVAL_SET_NUMBER_NOTFULL((v),(d))
#define  DUK_TVAL_SET_NAN(v)                 DUK__TVAL_SET_NAN_NOTFULL((v))
#define  DUK_DOUBLE_SET_NAN(d)               DUK__DOUBLE_SET_NAN_NOTFULL((d))
#endif

#define  DUK_TVAL_SET_STRING(v,h)            DUK__TVAL_SET_TAGGEDPOINTER((v),(h),DUK_TAG_STRING)
#define  DUK_TVAL_SET_OBJECT(v,h)            DUK__TVAL_SET_TAGGEDPOINTER((v),(h),DUK_TAG_OBJECT)
#define  DUK_TVAL_SET_BUFFER(v,h)            DUK__TVAL_SET_TAGGEDPOINTER((v),(h),DUK_TAG_BUFFER)
#define  DUK_TVAL_SET_POINTER(v,p)           DUK__TVAL_SET_TAGGEDPOINTER((v),(p),DUK_TAG_POINTER)

#define  DUK_TVAL_SET_TVAL(v,x)              do { *(v) = *(x); } while (0)

/* getters */
#define  DUK_TVAL_GET_BOOLEAN(v)             ((int) (v)->us[DUK_DBL_IDX_US1])
#define  DUK_TVAL_GET_NUMBER(v)              ((v)->d)
#define  DUK_TVAL_GET_STRING(v)              ((duk_hstring *) (v)->vp[DUK_DBL_IDX_VP1])
#define  DUK_TVAL_GET_OBJECT(v)              ((duk_hobject *) (v)->vp[DUK_DBL_IDX_VP1])
#define  DUK_TVAL_GET_BUFFER(v)              ((duk_hbuffer *) (v)->vp[DUK_DBL_IDX_VP1])
#define  DUK_TVAL_GET_POINTER(v)             ((void *) (v)->vp[DUK_DBL_IDX_VP1])
#define  DUK_TVAL_GET_HEAPHDR(v)             ((duk_heaphdr *) (v)->vp[DUK_DBL_IDX_VP1])

/* decoding */
#define  DUK_TVAL_GET_TAG(v)                 ((int) (v)->us[DUK_DBL_IDX_US0])

#define  DUK_TVAL_IS_UNDEFINED(v)            (DUK_TVAL_GET_TAG((v)) == DUK_TAG_UNDEFINED)
#define  DUK_TVAL_IS_UNDEFINED_ACTUAL(v)     ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_UNDEFINED_ACTUAL)
#define  DUK_TVAL_IS_UNDEFINED_UNUSED(v)     ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_UNDEFINED_UNUSED)
#define  DUK_TVAL_IS_NULL(v)                 (DUK_TVAL_GET_TAG((v)) == DUK_TAG_NULL)
#define  DUK_TVAL_IS_BOOLEAN(v)              (DUK_TVAL_GET_TAG((v)) == DUK_TAG_BOOLEAN)
#define  DUK_TVAL_IS_BOOLEAN_TRUE(v)         ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_BOOLEAN_TRUE)
#define  DUK_TVAL_IS_BOOLEAN_FALSE(v)        ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_BOOLEAN_FALSE)
#define  DUK_TVAL_IS_STRING(v)               (DUK_TVAL_GET_TAG((v)) == DUK_TAG_STRING)
#define  DUK_TVAL_IS_OBJECT(v)               (DUK_TVAL_GET_TAG((v)) == DUK_TAG_OBJECT)
#define  DUK_TVAL_IS_BUFFER(v)               (DUK_TVAL_GET_TAG((v)) == DUK_TAG_BUFFER)
#define  DUK_TVAL_IS_POINTER(v)              (DUK_TVAL_GET_TAG((v)) == DUK_TAG_POINTER)
/* 0xfff0 is -Infinity */
#define  DUK_TVAL_IS_NUMBER(v)               (DUK_TVAL_GET_TAG((v)) <= 0xfff0UL)

#define  DUK_TVAL_IS_HEAP_ALLOCATED(v)       (DUK_TVAL_GET_TAG((v)) >= DUK_TAG_STRING)

/* misc */

/* Note: 
 *    - These take a double pointer as argument, so an explicit cast is needed
 *    - Full NaN check: exact determination whether argument is a NaN
 *    - Partial NaN check: works for all NaNs except those that begin with
 *      0x7ff0; since our normalized NaN begins with 0x7ff8, this check is
 *      reliable for normalized values
 */

/* XXX: reading duk_uint32_t instead of duk_uint16_t is one byte shorter on x86 :) */
#ifdef USE__64BIT
#ifdef USE__ME_VARIANT
#define  DUK__DOUBLE_IS_NAN_FULL(d) \
	/* E == 0x7ff, F != 0 => NaN */ \
	(((((duk_tval *)(d))->us[DUK_DBL_IDX_US0] & 0x7ff0UL) == 0x7ff0UL) && \
	 (((((duk_tval *)(d))->ull[DUK_DBL_IDX_ULL0]) & 0xffffffff000fffffULL) != 0))
#else
#define  DUK__DOUBLE_IS_NAN_FULL(d) \
	/* E == 0x7ff, F != 0 => NaN */ \
	(((((duk_tval *)(d))->us[DUK_DBL_IDX_US0] & 0x7ff0UL) == 0x7ff0UL) && \
	 (((((duk_tval *)(d))->ull[DUK_DBL_IDX_ULL0]) & 0x000fffffffffffffULL) != 0))
#endif
#else  /* USE__64BIT */
#define  DUK__DOUBLE_IS_NAN_FULL(d) \
	/* E == 0x7ff, F != 0 => NaN */ \
	(((((duk_tval *)(d))->ui[DUK_DBL_IDX_UI0] & 0x7ff00000UL) == 0x7ff00000UL) && \
	 ((((duk_tval *)(d))->ui[DUK_DBL_IDX_UI0] & 0x000fffffUL) != 0 || \
          ((duk_tval *)(d))->ui[DUK_DBL_IDX_UI1] != 0))
#endif  /* USE__64BIT */

/* XXX: avoid possible double read? */
#define  DUK__DOUBLE_IS_NAN_NOTFULL(d) \
	/* E == 0x7ff, topmost four bits of F != 0 => assume NaN */ \
	(((((duk_tval *)(d))->us[DUK_DBL_IDX_US0] & 0x7ff0UL) == 0x7ff0UL) && \
	 ((((duk_tval *)(d))->us[DUK_DBL_IDX_US0] & 0x000fUL) != 0x0000UL))

#define  DUK__DOUBLE_NORMALIZE_NAN_CHECK_FULL(d)  do { \
		if (DUK__DOUBLE_IS_NAN_FULL((d))) { \
			DUK__DOUBLE_SET_NAN_FULL((d)); \
		} \
	} while (0)

#define  DUK__DOUBLE_NORMALIZE_NAN_CHECK_NOTFULL(d)  do { \
		if (DUK__DOUBLE_IS_NAN_NOTFULL((d))) { \
			DUK__DOUBLE_SET_NAN_NOTFULL((d)); \
		} \
	} while (0)

#ifdef USE__64BIT
#ifdef USE__ME_VARIANT
#define  DUK__DOUBLE_IS_NORMALIZED_NAN_FULL(d) \
	(((duk_tval *)(d))->ull[DUK_DBL_IDX_ULL0] == 0x000000007ff80000ULL)
#else
#define  DUK__DOUBLE_IS_NORMALIZED_NAN_FULL(d) \
	(((duk_tval *)(d))->ull[DUK_DBL_IDX_ULL0] == 0x7ff8000000000000ULL)
#endif
#else  /* USE__64BIT */
#define  DUK__DOUBLE_IS_NORMALIZED_NAN_FULL(d) \
	((((duk_tval *)(d))->ui[DUK_DBL_IDX_UI0] == 0x7ff80000UL) && \
	 (((duk_tval *)(d))->ui[DUK_DBL_IDX_UI1] == 0x00000000UL))
#endif  /* USE__64BIT */

#define  DUK__DOUBLE_IS_NORMALIZED_NAN_NOTFULL(d) \
	/* E == 0x7ff, F == 8 => normalized NaN */ \
	(((duk_tval *)(d))->us[DUK_DBL_IDX_US0] == 0x7ff8UL)

#ifdef DUK_USE_FULL_TVAL
#define  DUK_DOUBLE_NORMALIZE_NAN_CHECK(d)  DUK__DOUBLE_NORMALIZE_NAN_CHECK_FULL((d))
#define  DUK_DOUBLE_IS_NAN(d)               DUK__DOUBLE_IS_NAN_FULL((d))
#define  DUK_DOUBLE_IS_NORMALIZED_NAN(d)    DUK__DOUBLE_IS_NORMALIZED_NAN_FULL((d))
#else
#define  DUK_DOUBLE_NORMALIZE_NAN_CHECK(d)  DUK__DOUBLE_NORMALIZE_NAN_CHECK_NOTFULL((d))
#define  DUK_DOUBLE_IS_NAN(d)               DUK__DOUBLE_IS_NAN_NOTFULL((d))
#define  DUK_DOUBLE_IS_NORMALIZED_NAN(d)    DUK__DOUBLE_IS_NORMALIZED_NAN_NOTFULL((d))
#endif

/* this is used for assertions - it checks that the NaN is properly normalized */
#define  DUK_DOUBLE_IS_NORMALIZED(d) \
	(!DUK_DOUBLE_IS_NAN((d)) ||  /* either not a NaN */ \
	 DUK_DOUBLE_IS_NORMALIZED_NAN((d)))  /* or is a normalized NaN */

#else  /* DUK_USE_PACKED_TVAL */
/* ======================================================================== */

/*
 *  Portable 12-byte representation
 */

#ifdef DUK_USE_FULL_TVAL
#error no 'full' tagged values in 12-byte representation
#endif

typedef struct duk_tval_struct duk_tval;

struct duk_tval_struct {
	int t;
	union {
		double d;
		int i;
		void *voidptr;
		duk_hstring *hstring;
		duk_hobject *hobject;
		duk_hcompiledfunction *hcompiledfunction;
		duk_hnativefunction *hnativefunction;
		duk_hthread *hthread;
		duk_hbuffer *hbuffer;
		duk_heaphdr *heaphdr;
	} v;
};

#define  DUK__TAG_NUMBER               0  /* not exposed */
#define  DUK_TAG_UNDEFINED             1
#define  DUK_TAG_NULL                  2
#define  DUK_TAG_BOOLEAN               3
#define  DUK_TAG_POINTER               4
#define  DUK_TAG_STRING                5
#define  DUK_TAG_OBJECT                6
#define  DUK_TAG_BUFFER                7

/* DUK__TAG_NUMBER is intentionally first, as it is the default clause in code
 * to support the 8-byte representation.  Further, it is a non-heap-allocated
 * type so it should come before DUK_TAG_STRING.  Finally, it should not break
 * the tag value ranges covered by case-clauses in a switch-case.
 */

/* setters */
#define  DUK_TVAL_SET_UNDEFINED_ACTUAL(tv)  do { \
		(tv)->t = DUK_TAG_UNDEFINED; \
		(tv)->v.i = 0; \
	} while (0)

#define  DUK_TVAL_SET_UNDEFINED_UNUSED(tv)  do { \
		(tv)->t = DUK_TAG_UNDEFINED; \
		(tv)->v.i = 1; \
	} while (0)

#define  DUK_TVAL_SET_NULL(tv)  do { \
		(tv)->t = DUK_TAG_NULL; \
	} while (0)

#define  DUK_TVAL_SET_BOOLEAN(tv,val)  do { \
		(tv)->t = DUK_TAG_BOOLEAN; \
		(tv)->v.i = (val); \
	} while (0)

#define  DUK_TVAL_SET_NUMBER(tv,val)  do { \
		(tv)->t = DUK__TAG_NUMBER; \
		(tv)->v.d = (val); \
	} while (0)

#define  DUK_TVAL_SET_STRING(tv,hptr)  do { \
		(tv)->t = DUK_TAG_STRING; \
		(tv)->v.hstring = (hptr); \
	} while (0)

#define  DUK_TVAL_SET_OBJECT(tv,hptr)  do { \
		(tv)->t = DUK_TAG_OBJECT; \
		(tv)->v.hobject = (hptr); \
	} while (0)

#define  DUK_TVAL_SET_BUFFER(tv,hptr)  do { \
		(tv)->t = DUK_TAG_BUFFER; \
		(tv)->v.hbuffer = (hptr); \
	} while (0)

#define  DUK_TVAL_SET_POINTER(tv,hptr)  do { \
		(tv)->t = DUK_TAG_POINTER; \
		(tv)->v.voidptr = (hptr); \
	} while (0)

#define  DUK_TVAL_SET_NAN(tv)  do { \
		/* in non-packed representation we don't care about which NaN is used */ \
		(tv)->t = DUK__TAG_NUMBER; \
		(tv)->v.d = DUK_DOUBLE_NAN; \
	} while (0)

#define  DUK_DOUBLE_SET_NAN(d)  do { \
		/* in non-packed representation we don't care about which NaN is used */ \
		/* assume d is 'double *' */ \
		(d)[0] = DUK_DOUBLE_NAN; \
	} while (0)

#define  DUK_TVAL_SET_TVAL(v,x)              do { *(v) = *(x); } while (0)

/* getters */
#define  DUK_TVAL_GET_BOOLEAN(tv)           ((tv)->v.i)
#define  DUK_TVAL_GET_NUMBER(tv)            ((tv)->v.d)
#define  DUK_TVAL_GET_STRING(tv)            ((tv)->v.hstring)
#define  DUK_TVAL_GET_OBJECT(tv)            ((tv)->v.hobject)
#define  DUK_TVAL_GET_BUFFER(tv)            ((tv)->v.hbuffer)
#define  DUK_TVAL_GET_POINTER(tv)           ((tv)->v.voidptr)
#define  DUK_TVAL_GET_HEAPHDR(tv)           ((tv)->v.heaphdr)

/* decoding */
#define  DUK_TVAL_GET_TAG(tv)               ((tv)->t)
#define  DUK_TVAL_IS_NUMBER(tv)             ((tv)->t == DUK__TAG_NUMBER)
#define  DUK_TVAL_IS_UNDEFINED(tv)          ((tv)->t == DUK_TAG_UNDEFINED)
#define  DUK_TVAL_IS_UNDEFINED_ACTUAL(tv)   (((tv)->t == DUK_TAG_UNDEFINED) && ((tv)->v.i == 0))
#define  DUK_TVAL_IS_UNDEFINED_UNUSED(tv)   (((tv)->t == DUK_TAG_UNDEFINED) && ((tv)->v.i != 0))
#define  DUK_TVAL_IS_NULL(tv)               ((tv)->t == DUK_TAG_NULL)
#define  DUK_TVAL_IS_BOOLEAN(tv)            ((tv)->t == DUK_TAG_BOOLEAN)
#define  DUK_TVAL_IS_BOOLEAN_TRUE(tv)       (((tv)->t == DUK_TAG_BOOLEAN) && ((tv)->v.i != 0))
#define  DUK_TVAL_IS_BOOLEAN_FALSE(tv)      (((tv)->t == DUK_TAG_BOOLEAN) && ((tv)->v.i == 0))
#define  DUK_TVAL_IS_STRING(tv)             ((tv)->t == DUK_TAG_STRING)
#define  DUK_TVAL_IS_OBJECT(tv)             ((tv)->t == DUK_TAG_OBJECT)
#define  DUK_TVAL_IS_BUFFER(tv)             ((tv)->t == DUK_TAG_BUFFER)
#define  DUK_TVAL_IS_POINTER(tv)            ((tv)->t == DUK_TAG_POINTER)

#define  DUK_TVAL_IS_HEAP_ALLOCATED(tv)     ((tv)->t >= DUK_TAG_STRING)

/* misc */
#define  DUK_DOUBLE_NORMALIZE_NAN_CHECK(d)  /* nop: no need to normalize */
#define  DUK_DOUBLE_IS_NAN(d)               (DUK_ISNAN(*(d)))
#define  DUK_DOUBLE_IS_NORMALIZED_NAN(d)    (DUK_ISNAN(*(d)))  /* all NaNs are considered normalized */
#define  DUK_DOUBLE_IS_NORMALIZED(d)        1  /* all doubles are considered normalized */

#endif  /* DUK_USE_PACKED_TVAL */

/*
 *  Convenience (independent of representation)
 */

#define  DUK_TVAL_SET_BOOLEAN_TRUE(v)        DUK_TVAL_SET_BOOLEAN(v, 1)
#define  DUK_TVAL_SET_BOOLEAN_FALSE(v)       DUK_TVAL_SET_BOOLEAN(v, 0)

/* undefine local defines */
#ifdef USE__LE_VARIANT
#undef USE__LE_VARIANT
#endif
#ifdef USE__ME_VARIANT
#undef USE__ME_VARIANT
#endif
#ifdef USE__BE_VARIANT
#undef USE__BE_VARIANT
#endif
#ifdef USE__64BIT
#undef USE__64BIT
#endif

#endif  /* DUK_TVAL_H_INCLUDED */

