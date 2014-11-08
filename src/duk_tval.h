/*
 *  Tagged type definition (duk_tval) and accessor macros.
 *
 *  Access all fields through the accessor macros, as the representation
 *  is quite tricky.
 *
 *  There are two packed type alternatives: an 8-byte representation
 *  based on an IEEE double (preferred for compactness), and a 12-byte
 *  representation (portability).  The latter is needed also in e.g.
 *  64-bit environments (it usually pads to 16 bytes per value).
 *
 *  Selecting the tagged type format involves many trade-offs (memory
 *  use, size and performance of generated code, portability, etc),
 *  see doc/types.txt for a detailed discussion (especially of how the
 *  IEEE double format is used to pack tagged values).
 *
 *  NB: because macro arguments are often expressions, macros should
 *  avoid evaluating their argument more than once.
 */

#ifndef DUK_TVAL_H_INCLUDED
#define DUK_TVAL_H_INCLUDED

/* sanity */
#if !defined(DUK_USE_DOUBLE_LE) && !defined(DUK_USE_DOUBLE_ME) && !defined(DUK_USE_DOUBLE_BE)
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
#define DUK_TAG_NORMALIZED_NAN    0x7ff8UL   /* the NaN variant we use */
/* avoid tag 0xfff0, no risk of confusion with negative infinity */
#define DUK_TAG_UNDEFINED         0xfff1UL   /* embed: 0 or 1 (normal or unused) */
#define DUK_TAG_NULL              0xfff2UL   /* embed: nothing */
#define DUK_TAG_BOOLEAN           0xfff3UL   /* embed: 0 or 1 (false or true) */
/* DUK_TAG_NUMBER would logically go here, but it has multiple 'tags' */
#define DUK_TAG_POINTER           0xfff4UL   /* embed: void ptr */
#define DUK_TAG_LIGHTFUNC         0xfff5UL   /* embed: func ptr */
#define DUK_TAG_STRING            0xfff6UL   /* embed: duk_hstring ptr */
#define DUK_TAG_OBJECT            0xfff7UL   /* embed: duk_hobject ptr */
#define DUK_TAG_BUFFER            0xfff8UL   /* embed: duk_hbuffer ptr */

/* for convenience */
#define DUK_XTAG_UNDEFINED_ACTUAL 0xfff10000UL
#define DUK_XTAG_UNDEFINED_UNUSED 0xfff10001UL
#define DUK_XTAG_NULL             0xfff20000UL
#define DUK_XTAG_BOOLEAN_FALSE    0xfff30000UL
#define DUK_XTAG_BOOLEAN_TRUE     0xfff30001UL

#define DUK__TVAL_SET_UNDEFINED_ACTUAL_FULL(v)      DUK_DBLUNION_SET_HIGH32_ZERO_LOW32((v), DUK_XTAG_UNDEFINED_ACTUAL)
#define DUK__TVAL_SET_UNDEFINED_ACTUAL_NOTFULL(v)   DUK_DBLUNION_SET_HIGH32((v), DUK_XTAG_UNDEFINED_ACTUAL)
#define DUK__TVAL_SET_UNDEFINED_UNUSED_FULL(v)      DUK_DBLUNION_SET_HIGH32_ZERO_LOW32((v), DUK_XTAG_UNDEFINED_UNUSED)
#define DUK__TVAL_SET_UNDEFINED_UNUSED_NOTFULL(v)   DUK_DBLUNION_SET_HIGH32((v), DUK_XTAG_UNDEFINED_UNUSED)

/* Note: 16-bit initializer suffices (unlike for undefined/boolean) */
#define DUK__TVAL_SET_NULL_FULL(v)     DUK_DBLUNION_SET_HIGH32_ZERO_LOW32((v), DUK_XTAG_NULL)
#define DUK__TVAL_SET_NULL_NOTFULL(v)  do { \
		(v)->us[DUK_DBL_IDX_US0] = (duk_uint16_t) DUK_TAG_NULL; \
	} while (0)

#define DUK__TVAL_SET_BOOLEAN_FULL(v,val)    DUK_DBLUNION_SET_HIGH32_ZERO_LOW32((v), (((duk_uint32_t) DUK_TAG_BOOLEAN) << 16) | ((duk_uint32_t) val))
#define DUK__TVAL_SET_BOOLEAN_NOTFULL(v,val) DUK_DBLUNION_SET_HIGH32((v), (((duk_uint32_t) DUK_TAG_BOOLEAN) << 16) | ((duk_uint32_t) (val)))

/* assumes that caller has normalized a possible NaN value of 'val', otherwise trouble ahead;
 * no notfull variant
 */
#define DUK__TVAL_SET_NUMBER_FULL(v,val)     DUK_DBLUNION_SET_DOUBLE((v), (val))
#define DUK__TVAL_SET_NUMBER_NOTFULL(v,val)  DUK_DBLUNION_SET_DOUBLE((v), (val))

/* two casts to avoid gcc warning: "warning: cast from pointer to integer of different size [-Wpointer-to-int-cast]" */
#ifdef DUK_USE_64BIT_OPS
#ifdef DUK_USE_DOUBLE_ME
#define DUK__TVAL_SET_TAGGEDPOINTER(v,h,tag)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = (((duk_uint64_t) (tag)) << 16) | (((duk_uint64_t) (duk_uint32_t) (h)) << 32); \
	} while (0)
#else
#define DUK__TVAL_SET_TAGGEDPOINTER(v,h,tag)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = (((duk_uint64_t) (tag)) << 48) | ((duk_uint64_t) (duk_uint32_t) (h)); \
	} while (0)
#endif
#else  /* DUK_USE_64BIT_OPS */
#define DUK__TVAL_SET_TAGGEDPOINTER(v,h,tag)  do { \
		(v)->ui[DUK_DBL_IDX_UI0] = ((duk_uint32_t) (tag)) << 16; \
		(v)->ui[DUK_DBL_IDX_UI1] = (duk_uint32_t) (h); \
	} while (0)
#endif  /* DUK_USE_64BIT_OPS */

#ifdef DUK_USE_64BIT_OPS
/* Double casting for pointer to avoid gcc warning (cast from pointer to integer of different size) */
#ifdef DUK_USE_DOUBLE_ME
#define DUK__TVAL_SET_LIGHTFUNC(v,fp,flags)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = (((duk_uint64_t) DUK_TAG_LIGHTFUNC) << 16) | \
		                             ((duk_uint64_t) (flags)) | \
		                             (((duk_uint64_t) (duk_uint32_t) (fp)) << 32); \
	} while (0)
#else
#define DUK__TVAL_SET_LIGHTFUNC(v,fp,flags)  do { \
		(v)->ull[DUK_DBL_IDX_ULL0] = (((duk_uint64_t) DUK_TAG_LIGHTFUNC) << 48) | \
		                             (((duk_uint64_t) (flags)) << 32) | \
		                             ((duk_uint64_t) (duk_uint32_t) (fp)); \
	} while (0)
#endif
#else  /* DUK_USE_64BIT_OPS */
#define DUK__TVAL_SET_LIGHTFUNC(v,fp,flags)  do { \
		(v)->ui[DUK_DBL_IDX_UI0] = (((duk_uint32_t) DUK_TAG_LIGHTFUNC) << 16) | ((duk_uint32_t) (flags)); \
		(v)->ui[DUK_DBL_IDX_UI1] = (duk_uint32_t) (fp); \
	} while (0)
#endif  /* DUK_USE_64BIT_OPS */

/* select actual setters */
#ifdef DUK_USE_FULL_TVAL
#define DUK_TVAL_SET_UNDEFINED_ACTUAL(v)    DUK__TVAL_SET_UNDEFINED_ACTUAL_FULL((v))
#define DUK_TVAL_SET_UNDEFINED_UNUSED(v)    DUK__TVAL_SET_UNDEFINED_UNUSED_FULL((v))
#define DUK_TVAL_SET_NULL(v)                DUK__TVAL_SET_NULL_FULL((v))
#define DUK_TVAL_SET_BOOLEAN(v,i)           DUK__TVAL_SET_BOOLEAN_FULL((v),(i))
#define DUK_TVAL_SET_NUMBER(v,d)            DUK__TVAL_SET_NUMBER_FULL((v),(d))
#define DUK_TVAL_SET_NAN(v)                 DUK__TVAL_SET_NAN_FULL((v))
#else
#define DUK_TVAL_SET_UNDEFINED_ACTUAL(v)    DUK__TVAL_SET_UNDEFINED_ACTUAL_NOTFULL((v))
#define DUK_TVAL_SET_UNDEFINED_UNUSED(v)    DUK__TVAL_SET_UNDEFINED_UNUSED_NOTFULL((v))
#define DUK_TVAL_SET_NULL(v)                DUK__TVAL_SET_NULL_NOTFULL((v))
#define DUK_TVAL_SET_BOOLEAN(v,i)           DUK__TVAL_SET_BOOLEAN_NOTFULL((v),(i))
#define DUK_TVAL_SET_NUMBER(v,d)            DUK__TVAL_SET_NUMBER_NOTFULL((v),(d))
#define DUK_TVAL_SET_NAN(v)                 DUK__TVAL_SET_NAN_NOTFULL((v))
#endif

#define DUK_TVAL_SET_LIGHTFUNC(v,fp,flags)  DUK__TVAL_SET_LIGHTFUNC((v),(fp),(flags))
#define DUK_TVAL_SET_STRING(v,h)            DUK__TVAL_SET_TAGGEDPOINTER((v),(h),DUK_TAG_STRING)
#define DUK_TVAL_SET_OBJECT(v,h)            DUK__TVAL_SET_TAGGEDPOINTER((v),(h),DUK_TAG_OBJECT)
#define DUK_TVAL_SET_BUFFER(v,h)            DUK__TVAL_SET_TAGGEDPOINTER((v),(h),DUK_TAG_BUFFER)
#define DUK_TVAL_SET_POINTER(v,p)           DUK__TVAL_SET_TAGGEDPOINTER((v),(p),DUK_TAG_POINTER)

#define DUK_TVAL_SET_TVAL(v,x)              do { *(v) = *(x); } while (0)

/* getters */
#define DUK_TVAL_GET_BOOLEAN(v)             ((int) (v)->us[DUK_DBL_IDX_US1])
#define DUK_TVAL_GET_NUMBER(v)              ((v)->d)
#define DUK_TVAL_GET_LIGHTFUNC(v,out_fp,out_flags)  do { \
		(out_flags) = (v)->ui[DUK_DBL_IDX_UI0] & 0xffffUL; \
		(out_fp) = (duk_c_function) (v)->ui[DUK_DBL_IDX_UI1]; \
	} while (0)
#define DUK_TVAL_GET_LIGHTFUNC_FUNCPTR(v)   ((duk_c_function) ((v)->ui[DUK_DBL_IDX_UI1]))
#define DUK_TVAL_GET_LIGHTFUNC_FLAGS(v)     (((int) (v)->ui[DUK_DBL_IDX_UI0]) & 0xffffUL)
#define DUK_TVAL_GET_STRING(v)              ((duk_hstring *) (v)->vp[DUK_DBL_IDX_VP1])
#define DUK_TVAL_GET_OBJECT(v)              ((duk_hobject *) (v)->vp[DUK_DBL_IDX_VP1])
#define DUK_TVAL_GET_BUFFER(v)              ((duk_hbuffer *) (v)->vp[DUK_DBL_IDX_VP1])
#define DUK_TVAL_GET_POINTER(v)             ((void *) (v)->vp[DUK_DBL_IDX_VP1])
#define DUK_TVAL_GET_HEAPHDR(v)             ((duk_heaphdr *) (v)->vp[DUK_DBL_IDX_VP1])

/* decoding */
#define DUK_TVAL_GET_TAG(v)                 ((duk_small_uint_t) (v)->us[DUK_DBL_IDX_US0])

#define DUK_TVAL_IS_UNDEFINED(v)            (DUK_TVAL_GET_TAG((v)) == DUK_TAG_UNDEFINED)
#define DUK_TVAL_IS_UNDEFINED_ACTUAL(v)     ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_UNDEFINED_ACTUAL)
#define DUK_TVAL_IS_UNDEFINED_UNUSED(v)     ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_UNDEFINED_UNUSED)
#define DUK_TVAL_IS_NULL(v)                 (DUK_TVAL_GET_TAG((v)) == DUK_TAG_NULL)
#define DUK_TVAL_IS_BOOLEAN(v)              (DUK_TVAL_GET_TAG((v)) == DUK_TAG_BOOLEAN)
#define DUK_TVAL_IS_BOOLEAN_TRUE(v)         ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_BOOLEAN_TRUE)
#define DUK_TVAL_IS_BOOLEAN_FALSE(v)        ((v)->ui[DUK_DBL_IDX_UI0] == DUK_XTAG_BOOLEAN_FALSE)
#define DUK_TVAL_IS_LIGHTFUNC(v)            (DUK_TVAL_GET_TAG((v)) == DUK_TAG_LIGHTFUNC)
#define DUK_TVAL_IS_STRING(v)               (DUK_TVAL_GET_TAG((v)) == DUK_TAG_STRING)
#define DUK_TVAL_IS_OBJECT(v)               (DUK_TVAL_GET_TAG((v)) == DUK_TAG_OBJECT)
#define DUK_TVAL_IS_BUFFER(v)               (DUK_TVAL_GET_TAG((v)) == DUK_TAG_BUFFER)
#define DUK_TVAL_IS_POINTER(v)              (DUK_TVAL_GET_TAG((v)) == DUK_TAG_POINTER)
/* 0xfff0 is -Infinity */
#define DUK_TVAL_IS_NUMBER(v)               (DUK_TVAL_GET_TAG((v)) <= 0xfff0UL)

#define DUK_TVAL_IS_HEAP_ALLOCATED(v)       (DUK_TVAL_GET_TAG((v)) >= DUK_TAG_STRING)

#else  /* DUK_USE_PACKED_TVAL */
/* ======================================================================== */

/*
 *  Portable 12-byte representation
 */

/* Note: not initializing all bytes is normally not an issue: Duktape won't
 * read or use the uninitialized bytes so valgrind won't issue warnings.
 * In some special cases a harmless valgrind warning may be issued though.
 * For example, the DumpHeap debugger command writes out a compiled function's
 * 'data' area as is, including any uninitialized bytes, which causes a
 * valgrind warning.
 */

#ifdef DUK_USE_FULL_TVAL
#error no 'full' tagged values in 12-byte representation
#endif

typedef struct duk_tval_struct duk_tval;

struct duk_tval_struct {
	duk_small_uint_t t;
	duk_small_uint_t v_extra;
	union {
		duk_double_t d;
		duk_small_int_t i;
		void *voidptr;
		duk_hstring *hstring;
		duk_hobject *hobject;
		duk_hcompiledfunction *hcompiledfunction;
		duk_hnativefunction *hnativefunction;
		duk_hthread *hthread;
		duk_hbuffer *hbuffer;
		duk_heaphdr *heaphdr;
		duk_c_function lightfunc;
	} v;
};

#define DUK__TAG_NUMBER               0  /* not exposed */
#define DUK_TAG_UNDEFINED             1
#define DUK_TAG_NULL                  2
#define DUK_TAG_BOOLEAN               3
#define DUK_TAG_POINTER               4
#define DUK_TAG_LIGHTFUNC             5
#define DUK_TAG_STRING                6
#define DUK_TAG_OBJECT                7
#define DUK_TAG_BUFFER                8

/* DUK__TAG_NUMBER is intentionally first, as it is the default clause in code
 * to support the 8-byte representation.  Further, it is a non-heap-allocated
 * type so it should come before DUK_TAG_STRING.  Finally, it should not break
 * the tag value ranges covered by case-clauses in a switch-case.
 */

/* setters */
#define DUK_TVAL_SET_UNDEFINED_ACTUAL(tv)  do { \
		(tv)->t = DUK_TAG_UNDEFINED; \
		(tv)->v.i = 0; \
	} while (0)

#define DUK_TVAL_SET_UNDEFINED_UNUSED(tv)  do { \
		(tv)->t = DUK_TAG_UNDEFINED; \
		(tv)->v.i = 1; \
	} while (0)

#define DUK_TVAL_SET_NULL(tv)  do { \
		(tv)->t = DUK_TAG_NULL; \
	} while (0)

#define DUK_TVAL_SET_BOOLEAN(tv,val)  do { \
		(tv)->t = DUK_TAG_BOOLEAN; \
		(tv)->v.i = (val); \
	} while (0)

#define DUK_TVAL_SET_NUMBER(tv,val)  do { \
		(tv)->t = DUK__TAG_NUMBER; \
		(tv)->v.d = (val); \
	} while (0)

#define DUK_TVAL_SET_POINTER(tv,hptr)  do { \
		(tv)->t = DUK_TAG_POINTER; \
		(tv)->v.voidptr = (hptr); \
	} while (0)

#define DUK_TVAL_SET_LIGHTFUNC(tv,fp,flags)  do { \
		(tv)->t = DUK_TAG_LIGHTFUNC; \
		(tv)->v_extra = (flags); \
		(tv)->v.lightfunc = (duk_c_function) (fp); \
	} while (0)

#define DUK_TVAL_SET_STRING(tv,hptr)  do { \
		(tv)->t = DUK_TAG_STRING; \
		(tv)->v.hstring = (hptr); \
	} while (0)

#define DUK_TVAL_SET_OBJECT(tv,hptr)  do { \
		(tv)->t = DUK_TAG_OBJECT; \
		(tv)->v.hobject = (hptr); \
	} while (0)

#define DUK_TVAL_SET_BUFFER(tv,hptr)  do { \
		(tv)->t = DUK_TAG_BUFFER; \
		(tv)->v.hbuffer = (hptr); \
	} while (0)

#define DUK_TVAL_SET_NAN(tv)  do { \
		/* in non-packed representation we don't care about which NaN is used */ \
		(tv)->t = DUK__TAG_NUMBER; \
		(tv)->v.d = DUK_DOUBLE_NAN; \
	} while (0)

#define DUK_TVAL_SET_TVAL(v,x)              do { *(v) = *(x); } while (0)

/* getters */
#define DUK_TVAL_GET_BOOLEAN(tv)           ((tv)->v.i)
#define DUK_TVAL_GET_NUMBER(tv)            ((tv)->v.d)
#define DUK_TVAL_GET_POINTER(tv)           ((tv)->v.voidptr)
#define DUK_TVAL_GET_LIGHTFUNC(tv,out_fp,out_flags)  do { \
		(out_flags) = (duk_uint32_t) (tv)->v_extra; \
		(out_fp) = (tv)->v.lightfunc; \
	} while (0)
#define DUK_TVAL_GET_LIGHTFUNC_FUNCPTR(tv) ((tv)->v.lightfunc)
#define DUK_TVAL_GET_LIGHTFUNC_FLAGS(tv)   ((duk_uint32_t) ((tv)->v_extra))
#define DUK_TVAL_GET_STRING(tv)            ((tv)->v.hstring)
#define DUK_TVAL_GET_OBJECT(tv)            ((tv)->v.hobject)
#define DUK_TVAL_GET_BUFFER(tv)            ((tv)->v.hbuffer)
#define DUK_TVAL_GET_HEAPHDR(tv)           ((tv)->v.heaphdr)

/* decoding */
#define DUK_TVAL_GET_TAG(tv)               ((tv)->t)
#define DUK_TVAL_IS_UNDEFINED(tv)          ((tv)->t == DUK_TAG_UNDEFINED)
#define DUK_TVAL_IS_UNDEFINED_ACTUAL(tv)   (((tv)->t == DUK_TAG_UNDEFINED) && ((tv)->v.i == 0))
#define DUK_TVAL_IS_UNDEFINED_UNUSED(tv)   (((tv)->t == DUK_TAG_UNDEFINED) && ((tv)->v.i != 0))
#define DUK_TVAL_IS_NULL(tv)               ((tv)->t == DUK_TAG_NULL)
#define DUK_TVAL_IS_BOOLEAN(tv)            ((tv)->t == DUK_TAG_BOOLEAN)
#define DUK_TVAL_IS_BOOLEAN_TRUE(tv)       (((tv)->t == DUK_TAG_BOOLEAN) && ((tv)->v.i != 0))
#define DUK_TVAL_IS_BOOLEAN_FALSE(tv)      (((tv)->t == DUK_TAG_BOOLEAN) && ((tv)->v.i == 0))
#define DUK_TVAL_IS_NUMBER(tv)             ((tv)->t == DUK__TAG_NUMBER)
#define DUK_TVAL_IS_POINTER(tv)            ((tv)->t == DUK_TAG_POINTER)
#define DUK_TVAL_IS_LIGHTFUNC(tv)          ((tv)->t == DUK_TAG_LIGHTFUNC)
#define DUK_TVAL_IS_STRING(tv)             ((tv)->t == DUK_TAG_STRING)
#define DUK_TVAL_IS_OBJECT(tv)             ((tv)->t == DUK_TAG_OBJECT)
#define DUK_TVAL_IS_BUFFER(tv)             ((tv)->t == DUK_TAG_BUFFER)

#define DUK_TVAL_IS_HEAP_ALLOCATED(tv)     ((tv)->t >= DUK_TAG_STRING)

#endif  /* DUK_USE_PACKED_TVAL */

/*
 *  Convenience (independent of representation)
 */

#define DUK_TVAL_SET_BOOLEAN_TRUE(v)        DUK_TVAL_SET_BOOLEAN(v, 1)
#define DUK_TVAL_SET_BOOLEAN_FALSE(v)       DUK_TVAL_SET_BOOLEAN(v, 0)

/* Lightfunc flags packing and unpacking. */
/* Sign extend: 0x0000##00 -> 0x##000000 -> sign extend to 0xssssss## */
#define DUK_LFUNC_FLAGS_GET_MAGIC(lf_flags) \
	((((duk_int32_t) (lf_flags)) << 16) >> 24)
#define DUK_LFUNC_FLAGS_GET_LENGTH(lf_flags) \
	(((lf_flags) >> 4) & 0x0f)
#define DUK_LFUNC_FLAGS_GET_NARGS(lf_flags) \
	((lf_flags) & 0x0f)
#define DUK_LFUNC_FLAGS_PACK(magic,length,nargs) \
	(((magic) & 0xff) << 8) | ((length) << 4) | (nargs)

#define DUK_LFUNC_NARGS_VARARGS             0x0f   /* varargs marker */
#define DUK_LFUNC_NARGS_MIN                 0x00
#define DUK_LFUNC_NARGS_MAX                 0x0e   /* max, excl. varargs marker */
#define DUK_LFUNC_LENGTH_MIN                0x00
#define DUK_LFUNC_LENGTH_MAX                0x0f
#define DUK_LFUNC_MAGIC_MIN                 (-0x80)
#define DUK_LFUNC_MAGIC_MAX                 0x7f

#endif  /* DUK_TVAL_H_INCLUDED */
