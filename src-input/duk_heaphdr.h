/*
 *  Heap header definition and assorted macros, including ref counting.
 *  Access all fields through the accessor macros.
 *
 *  All heap objects share the same flags and refcount fields.  Objects other
 *  than strings also need to have a single or double linked list pointers
 *  for insertion into the "heap allocated" list.  Strings have single linked
 *  list pointers for string table chaining.
 *
 *  Technically, 'h_refcount' must be wide enough to guarantee that it cannot
 *  wrap; otherwise objects might be freed incorrectly after wrapping.  The
 *  default refcount field is 32 bits even on 64-bit systems: while that's in
 *  theory incorrect, the Duktape heap needs to be larger than 64GB for the
 *  count to actually wrap (assuming 16-byte duk_tvals).  This is very unlikely
 *  to ever be an issue, but if it is, disabling DUK_USE_REFCOUNT32 causes
 *  Duktape to use size_t for refcounts which should always be safe.
 *
 *  Heap header size on 32-bit platforms: 8 bytes without reference counting,
 *  16 bytes with reference counting.
 *
 *  Note that 'raw' macros such as DUK_HEAPHDR_GET_REFCOUNT() are not
 *  defined without DUK_USE_REFERENCE_COUNTING, so caller must #if defined()
 *  around them.
 */

#if !defined(DUK_HEAPHDR_H_INCLUDED)
#define DUK_HEAPHDR_H_INCLUDED

/* Flags allocation:
 *
 *   xxxxxxxx xxxxxxxx xxxxxXXX XXHHHHHH
 *
 *   H = HTYPE field
 *   X = heaphdr level bits
 *   x = reserved for subtypes
 */

#define DUK_HEAPHDR_FLAGS_TYPE_MASK     0x0000003fUL
#define DUK_HEAPHDR_FLAGS_FLAG_MASK     (~DUK_HEAPHDR_FLAGS_TYPE_MASK)
#define DUK_HEAPHDR_FLAGS_USER_START    11 /* 21 user flags */
#define DUK_HEAPHDR_USER_FLAG_NUMBER(n) (DUK_HEAPHDR_FLAGS_USER_START + (n))
#define DUK_HEAPHDR_USER_FLAG(n)        (1UL << (DUK_HEAPHDR_FLAGS_USER_START + (n)))

#define DUK_HEAPHDR_FLAG_TEMPROOT    0x0040UL /* mark-and-sweep: children not processed */
#define DUK_HEAPHDR_FLAG_REACHABLE   0x0080UL /* mark-and-sweep: reachable */
#define DUK_HEAPHDR_FLAG_FINALIZABLE 0x0100UL /* mark-and-sweep: finalizable (on current pass) */
#define DUK_HEAPHDR_FLAG_FINALIZED   0x0200UL /* mark-and-sweep: finalized (on previous pass) */
#define DUK_HEAPHDR_FLAG_READONLY    0x0400UL /* read-only object, in code section */

#define DUK_HEAPHDR_GET_FLAGS_RAW(h) ((h)->h_flags)
#define DUK_HEAPHDR_GET_FLAGS(h)     ((h)->h_flags & DUK_HEAPHDR_FLAGS_FLAG_MASK)

#define DUK_HEAPHDR_SET_FLAGS_RAW(h, val) \
	do { \
		(h)->h_flags = (val); \
	} while (0)
#define DUK_HEAPHDR_SET_FLAGS(h, val) \
	do { \
		(h)->h_flags = ((h)->h_flags & ~(DUK_HEAPHDR_FLAGS_FLAG_MASK)) | (val); \
	} while (0)

#define DUK_HEAPHDR_GET_HTYPE(h) ((h)->h_flags & 0x3fUL)
#define DUK_HEAPHDR_SET_HTYPE(h, val) \
	do { \
		(h)->h_flags = ((h)->h_flags & ~(DUK_HEAPHDR_FLAGS_TYPE_MASK)) | (val); \
	} while (0)

#define DUK_HEAPHDR_SET_HTYPE_AND_FLAGS(h, tval, fval) \
	do { \
		(h)->h_flags = ((tval) &DUK_HEAPHDR_FLAGS_TYPE_MASK) | ((fval) &DUK_HEAPHDR_FLAGS_FLAG_MASK); \
	} while (0)

#define DUK_HEAPHDR_HTYPE_AS_FLAGS(htype) ((duk_uint32_t) (htype))

/* HTYPE allocation is critical for performance and footprint.  Each HTYPE
 * determines a particular struct subtype (e.g. duk_hstring or duk_hcompfunc).
 * It also maps to an ECMAScript internal class.
 *
 * Because HTYPE is 6 bits, we can use a class mask for type detection when
 * 64-bit types are available.  With 32-bit types only this can be worked
 * around by using lookups based on HTYPE (AND, indexed MOV, TEST).
 *
 * Tests for HTYPE bit patterns:
 *   (1) Single bit set or clear => TEST
 *   (2) Multiple bits, >= 1 set => TEST
 *   (3) Multiple bits, all clear => TEST
 *   (4) Multiple bits, all set => AND + CMP
 *   (5) Multiple bits, arbitrary pattern => AND + CMP
 *   (6) Comparison using <, >, or == => CMP
 *
 * An internal object requires a HTYPE for various reasons:
 *   - If the object has a specific C struct layout (e.g. duk_hproxy) it
 *     should have a HTYPE.
 *   - If the object has exotic property behaviors, in which case HTYPE
 *     can be used to look up a handler function.
 *   - If the object type needs to be checked compactly in internals, e.g.
 *     Date code checking for a Date instance.  An alternative is to use a
 *     hidden Symbol for that purpose (for example).
 *   - To provide an ECMAScript internal class (for Object.prototype.toString).
 *     An alternative is to use a hidden Symbol to provide the (rarely needed)
 *     class instead.
 *
 * On the other hand each HTYPE increases the size of several switch clauses
 * which deal with all HTYPEs, for example in reference counting and
 * mark-and-sweep.
 */

#define DUK_HTYPE_IS_ANY_STRING(htype) ((htype) <= 0x03U)
#define DUK_HEAPHDR_IS_ANY_STRING(h)   (((h)->h_flags & 0x3fU) <= 0x03U)

#define DUK_HTYPE_IS_ANY_BUFFER(htype) (((htype) &0x3cU) == 0x04U)
#define DUK_HEAPHDR_IS_ANY_BUFFER(h)   (((h)->h_flags & 0x3cU) == 0x04U)

#define DUK_HTYPE_IS_ANY_BUFOBJ(htype) ((htype) >= 0x30U)
#define DUK_HEAPHDR_IS_ANY_BUFOBJ(h)   (((h)->h_flags & 0x3fU) >= 0x30U)

#define DUK_HTYPE_IS_ANY_OBJECT(htype) ((htype) >= 0x08U)
#define DUK_HEAPHDR_IS_ANY_OBJECT(h)   (((h)->h_flags & 0x3fU) >= 0x08U)

#define DUK_HTYPE_IS_ARRAY(htype) ((htype) == DUK_HTYPE_ARRAY)
#define DUK_HEAPHDR_IS_ARRAY(h)   (DUK_HEAPHDR_GET_HTYPE((h)) == DUK_HTYPE_ARRAY)

/* 0b0000xx: duk_hstring and subclasses; API type is DUK_TYPE_STRING */
#define DUK_HTYPE_MIN               0
#define DUK_HTYPE_STRING_INTERNAL   0 /* 0b000000 */
#define DUK_HTYPE_STRING_EXTERNAL   1 /* 0b000001 */
#define DUK_HTYPE_RESERVED2         2
#define DUK_HTYPE_RESERVED3         3
/* 0b0001xx: duk_hbuffer and subclasses; API type is DUK_TYPE_BUFFER */
#define DUK_HTYPE_BUFFER_FIXED      4 /* 0b000100 */
#define DUK_HTYPE_BUFFER_DYNAMIC    5 /* 0b000101 */
#define DUK_HTYPE_BUFFER_EXTERNAL   6 /* 0b000110 */
#define DUK_HTYPE_RESERVED7         7
/* 0b00100x: duk_harray */
#define DUK_HTYPE_ARRAY             8 /* 0b001000 */
#define DUK_HTYPE_ARGUMENTS         9 /* 0b001001 */
/* 0b00101x: misc */
#define DUK_HTYPE_OBJECT            10 /* 0b001010 */
#define DUK_HTYPE_RESERVED11        11
/* 0b0011xx: functions */
#define DUK_HTYPE_COMPFUNC          12
#define DUK_HTYPE_NATFUNC           13
#define DUK_HTYPE_BOUNDFUNC         14
#define DUK_HTYPE_RESERVED15        15
/* 0b01xxxx: built-ins etc */
#define DUK_HTYPE_BOOLEAN_OBJECT    16
#define DUK_HTYPE_DATE              17
#define DUK_HTYPE_ERROR             18
#define DUK_HTYPE_JSON              19
#define DUK_HTYPE_MATH              20
#define DUK_HTYPE_NUMBER_OBJECT     21
#define DUK_HTYPE_REGEXP            22
#define DUK_HTYPE_STRING_OBJECT     23
#define DUK_HTYPE_GLOBAL            24
#define DUK_HTYPE_SYMBOL_OBJECT     25
#define DUK_HTYPE_OBJENV            26
#define DUK_HTYPE_DECENV            27
#define DUK_HTYPE_POINTER_OBJECT    28
#define DUK_HTYPE_THREAD            29
#define DUK_HTYPE_PROXY             30
#define DUK_HTYPE_RESERVED31        31
/* 0b10xxxx: built-ins etc */
#define DUK_HTYPE_NONE              32
#define DUK_HTYPE_RESERVED33        33
#define DUK_HTYPE_RESERVED34        34
#define DUK_HTYPE_RESERVED35        35
#define DUK_HTYPE_RESERVED36        36
#define DUK_HTYPE_RESERVED37        37
#define DUK_HTYPE_RESERVED38        38
#define DUK_HTYPE_RESERVED39        39
#define DUK_HTYPE_RESERVED40        40
#define DUK_HTYPE_RESERVED41        41
#define DUK_HTYPE_RESERVED42        42
#define DUK_HTYPE_RESERVED43        43
#define DUK_HTYPE_RESERVED44        44
#define DUK_HTYPE_RESERVED45        45
#define DUK_HTYPE_RESERVED46        46
#define DUK_HTYPE_RESERVED47        47
/* 0b11xxxx: buffer objects */
#define DUK_HTYPE_ARRAYBUFFER       48
#define DUK_HTYPE_RESERVED49        49
#define DUK_HTYPE_DATAVIEW          50
#define DUK_HTYPE_INT8ARRAY         51
#define DUK_HTYPE_UINT8ARRAY        52
#define DUK_HTYPE_UINT8CLAMPEDARRAY 53
#define DUK_HTYPE_INT16ARRAY        54
#define DUK_HTYPE_UINT16ARRAY       55
#define DUK_HTYPE_INT32ARRAY        56
#define DUK_HTYPE_UINT32ARRAY       57
#define DUK_HTYPE_FLOAT32ARRAY      58
#define DUK_HTYPE_FLOAT64ARRAY      59
#define DUK_HTYPE_RESERVED60        60
#define DUK_HTYPE_RESERVED61        61
#define DUK_HTYPE_RESERVED62        62
#define DUK_HTYPE_RESERVED63        63
#define DUK_HTYPE_MAX               60
#define DUK_HTYPE_BUFOBJ_MIN        48
#define DUK_HTYPE_BUFOBJ_MAX        59

/* HTYPE mask, only available with 64-bit types. */
#if defined(DUK_USE_64BIT_OPS)
#define DUK_HEAPHDR_GET_HMASK(h) (DUK_U64_CONSTANT(1) << ((h)->h_flags & 0x3fUL))

#define DUK_HMASK_STRING_INTERNAL   (DUK_U64_CONSTANT(1) << DUK_HTYPE_STRING_INTERNAL)
#define DUK_HMASK_STRING_EXTERNAL   (DUK_U64_CONSTANT(1) << DUK_HTYPE_STRING_EXTERNAL)
#define DUK_HMASK_RESERVED2         (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED2)
#define DUK_HMASK_RESERVED3         (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED3)
#define DUK_HMASK_BUFFER_FIXED      (DUK_U64_CONSTANT(1) << DUK_HTYPE_BUFFER_FIXED)
#define DUK_HMASK_BUFFER_DYNAMIC    (DUK_U64_CONSTANT(1) << DUK_HTYPE_BUFFER_DYNAMIC)
#define DUK_HMASK_BUFFER_EXTERNAL   (DUK_U64_CONSTANT(1) << DUK_HTYPE_BUFFER_EXTERNAL)
#define DUK_HMASK_RESERVED7         (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED7)
#define DUK_HMASK_ARRAY             (DUK_U64_CONSTANT(1) << DUK_HTYPE_ARRAY)
#define DUK_HMASK_ARGUMENTS         (DUK_U64_CONSTANT(1) << DUK_HTYPE_ARGUMENTS)
#define DUK_HMASK_OBJECT            (DUK_U64_CONSTANT(1) << DUK_HTYPE_OBJECT)
#define DUK_HMASK_RESERVED11        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED11)
#define DUK_HMASK_COMPFUNC          (DUK_U64_CONSTANT(1) << DUK_HTYPE_COMPFUNC)
#define DUK_HMASK_NATFUNC           (DUK_U64_CONSTANT(1) << DUK_HTYPE_NATFUNC)
#define DUK_HMASK_BOUNDFUNC         (DUK_U64_CONSTANT(1) << DUK_HTYPE_BOUNDFUNC)
#define DUK_HMASK_RESERVED15        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED15)
#define DUK_HMASK_BOOLEAN_OBJECT    (DUK_U64_CONSTANT(1) << DUK_HTYPE_BOOLEAN_OBJECT)
#define DUK_HMASK_DATE              (DUK_U64_CONSTANT(1) << DUK_HTYPE_DATE)
#define DUK_HMASK_ERROR             (DUK_U64_CONSTANT(1) << DUK_HTYPE_ERROR)
#define DUK_HMASK_JSON              (DUK_U64_CONSTANT(1) << DUK_HTYPE_JSON)
#define DUK_HMASK_MATH              (DUK_U64_CONSTANT(1) << DUK_HTYPE_MATH)
#define DUK_HMASK_NUMBER_OBJECT     (DUK_U64_CONSTANT(1) << DUK_HTYPE_NUMBER_OBJECT)
#define DUK_HMASK_REGEXP            (DUK_U64_CONSTANT(1) << DUK_HTYPE_REGEXP)
#define DUK_HMASK_STRING_OBJECT     (DUK_U64_CONSTANT(1) << DUK_HTYPE_STRING_OBJECT)
#define DUK_HMASK_GLOBAL            (DUK_U64_CONSTANT(1) << DUK_HTYPE_GLOBAL)
#define DUK_HMASK_SYMBOL_OBJECT     (DUK_U64_CONSTANT(1) << DUK_HTYPE_SYMBOL_OBJECT)
#define DUK_HMASK_OBJENV            (DUK_U64_CONSTANT(1) << DUK_HTYPE_OBJENV)
#define DUK_HMASK_DECENV            (DUK_U64_CONSTANT(1) << DUK_HTYPE_DECENV)
#define DUK_HMASK_POINTER_OBJECT    (DUK_U64_CONSTANT(1) << DUK_HTYPE_POINTER_OBJECT)
#define DUK_HMASK_THREAD            (DUK_U64_CONSTANT(1) << DUK_HTYPE_THREAD)
#define DUK_HMASK_PROXY             (DUK_U64_CONSTANT(1) << DUK_HTYPE_PROXY)
#define DUK_HMASK_RESERVED31        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED31)
#define DUK_HMASK_RESERVED33        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED33)
#define DUK_HMASK_RESERVED34        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED34)
#define DUK_HMASK_RESERVED35        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED35)
#define DUK_HMASK_RESERVED36        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED36)
#define DUK_HMASK_RESERVED37        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED37)
#define DUK_HMASK_RESERVED38        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED38)
#define DUK_HMASK_RESERVED39        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED39)
#define DUK_HMASK_RESERVED40        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED40)
#define DUK_HMASK_RESERVED41        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED41)
#define DUK_HMASK_RESERVED42        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED42)
#define DUK_HMASK_RESERVED43        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED43)
#define DUK_HMASK_RESERVED44        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED44)
#define DUK_HMASK_RESERVED45        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED45)
#define DUK_HMASK_RESERVED46        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED46)
#define DUK_HMASK_RESERVED47        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED47)
#define DUK_HMASK_ARRAYBUFFER       (DUK_U64_CONSTANT(1) << DUK_HTYPE_ARRAYBUFFER)
#define DUK_HMASK_RESERVED49        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED49)
#define DUK_HMASK_DATAVIEW          (DUK_U64_CONSTANT(1) << DUK_HTYPE_DATAVIEW)
#define DUK_HMASK_INT8ARRAY         (DUK_U64_CONSTANT(1) << DUK_HTYPE_INT8ARRAY)
#define DUK_HMASK_UINT8ARRAY        (DUK_U64_CONSTANT(1) << DUK_HTYPE_UINT8ARRAY)
#define DUK_HMASK_UINT8CLAMPEDARRAY (DUK_U64_CONSTANT(1) << DUK_HTYPE_UINT8CLAMPEDARRAY)
#define DUK_HMASK_INT16ARRAY        (DUK_U64_CONSTANT(1) << DUK_HTYPE_INT16ARRAY)
#define DUK_HMASK_UINT16ARRAY       (DUK_U64_CONSTANT(1) << DUK_HTYPE_UINT16ARRAY)
#define DUK_HMASK_INT32ARRAY        (DUK_U64_CONSTANT(1) << DUK_HTYPE_INT32ARRAY)
#define DUK_HMASK_UINT32ARRAY       (DUK_U64_CONSTANT(1) << DUK_HTYPE_UINT32ARRAY)
#define DUK_HMASK_FLOAT32ARRAY      (DUK_U64_CONSTANT(1) << DUK_HTYPE_FLOAT32ARRAY)
#define DUK_HMASK_FLOAT64ARRAY      (DUK_U64_CONSTANT(1) << DUK_HTYPE_FLOAT64ARRAY)
#define DUK_HMASK_RESERVED60        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED60)
#define DUK_HMASK_RESERVED61        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED61)
#define DUK_HMASK_RESERVED62        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED62)
#define DUK_HMASK_RESERVED63        (DUK_U64_CONSTANT(1) << DUK_HTYPE_RESERVED63)

#define DUK_HMASK_ALL_OBJECTS   DUK_U64_CONSTANT(0xffffffffffffff00)
#define DUK_HMASK_ALL_FUNCTIONS (DUK_HMASK_COMPFUNC | DUK_HMASK_NATFUNC | DUK_HMASK_BOUNDFUNC)
#define DUK_HMASK_ALL_BUFOBJS \
	(DUK_HMASK_ARRAYBUFFER | DUK_HMASK_DATAVIEW | DUK_HMASK_INT8ARRAY | DUK_HMASK_UINT8ARRAY | DUK_HMASK_UINT8CLAMPEDARRAY | \
	 DUK_HMASK_INT16ARRAY | DUK_HMASK_UINT16ARRAY | DUK_HMASK_INT32ARRAY | DUK_HMASK_UINT32ARRAY | DUK_HMASK_FLOAT32ARRAY | \
	 DUK_HMASK_FLOAT64ARRAY)

#endif /* DUK_USE_64BIT_OPS */

struct duk_heaphdr {
	duk_uint32_t h_flags;

#if defined(DUK_USE_REFERENCE_COUNTING)
#if defined(DUK_USE_ASSERTIONS)
	/* When assertions enabled, used by mark-and-sweep for refcount
	 * validation.  Largest reasonable type; also detects overflows.
	 */
	duk_size_t h_assert_refcount;
#endif
#if defined(DUK_USE_REFCOUNT16)
	duk_uint16_t h_refcount;
#elif defined(DUK_USE_REFCOUNT32)
	duk_uint32_t h_refcount;
#else
	duk_size_t h_refcount;
#endif
#endif /* DUK_USE_REFERENCE_COUNTING */

#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t h_next16;
#else
	duk_heaphdr *h_next;
#endif

#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
	/* refcounting requires direct heap frees, which in turn requires a dual linked heap */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t h_prev16;
#else
	duk_heaphdr *h_prev;
#endif
#endif

	/* When DUK_USE_HEAPPTR16 (and DUK_USE_REFCOUNT16) is in use, the
	 * struct won't align nicely to 4 bytes.  This 16-bit extra field
	 * is added to make the alignment clean; the field can be used by
	 * heap objects when 16-bit packing is used.  This field is now
	 * conditional to DUK_USE_HEAPPTR16 only, but it is intended to be
	 * used with DUK_USE_REFCOUNT16 and DUK_USE_DOUBLE_LINKED_HEAP;
	 * this only matter to low memory environments anyway.
	 */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t h_extra16;
#endif
};

struct duk_heaphdr_string {
	/* 16 bits would be enough for shared heaphdr flags and duk_hstring
	 * flags.  The initial parts of duk_heaphdr_string and duk_heaphdr
	 * must match so changing the flags field size here would be quite
	 * awkward.  However, to minimize struct size, we can pack at least
	 * 16 bits of duk_hstring data into the flags field.
	 */
	duk_uint32_t h_flags;

#if defined(DUK_USE_REFERENCE_COUNTING)
#if defined(DUK_USE_ASSERTIONS)
	/* When assertions enabled, used by mark-and-sweep for refcount
	 * validation.  Largest reasonable type; also detects overflows.
	 */
	duk_size_t h_assert_refcount;
#endif
#if defined(DUK_USE_REFCOUNT16)
	duk_uint16_t h_refcount;
	duk_uint16_t h_strextra16; /* round out to 8 bytes */
#elif defined(DUK_USE_REFCOUNT32)
	duk_uint32_t h_refcount;
#else
	duk_size_t h_refcount;
#endif
#else
	duk_uint16_t h_strextra16;
#endif /* DUK_USE_REFERENCE_COUNTING */

	duk_hstring *h_next;
	/* No 'h_prev' pointer for strings. */
};

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HEAPHDR_GET_NEXT(heap, h) ((duk_heaphdr *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (h)->h_next16))
#define DUK_HEAPHDR_SET_NEXT(heap, h, val) \
	do { \
		(h)->h_next16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) val); \
	} while (0)
#else
#define DUK_HEAPHDR_GET_NEXT(heap, h) ((h)->h_next)
#define DUK_HEAPHDR_SET_NEXT(heap, h, val) \
	do { \
		(h)->h_next = (val); \
	} while (0)
#endif

#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
#if defined(DUK_USE_HEAPPTR16)
#define DUK_HEAPHDR_GET_PREV(heap, h) ((duk_heaphdr *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (h)->h_prev16))
#define DUK_HEAPHDR_SET_PREV(heap, h, val) \
	do { \
		(h)->h_prev16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) (val)); \
	} while (0)
#else
#define DUK_HEAPHDR_GET_PREV(heap, h) ((h)->h_prev)
#define DUK_HEAPHDR_SET_PREV(heap, h, val) \
	do { \
		(h)->h_prev = (val); \
	} while (0)
#endif
#endif

#if defined(DUK_USE_REFERENCE_COUNTING)
#define DUK_HEAPHDR_GET_REFCOUNT(h) ((h)->h_refcount)
#define DUK_HEAPHDR_SET_REFCOUNT(h, val) \
	do { \
		(h)->h_refcount = (val); \
		DUK_ASSERT((h)->h_refcount == (val)); /* No truncation. */ \
	} while (0)
#define DUK_HEAPHDR_PREINC_REFCOUNT(h) (++(h)->h_refcount) /* result: updated refcount */
#define DUK_HEAPHDR_PREDEC_REFCOUNT(h) (--(h)->h_refcount) /* result: updated refcount */
#else
/* Refcount macros not defined without refcounting, caller must #if defined() now. */
#endif /* DUK_USE_REFERENCE_COUNTING */

/* Comparison for type >= DUK_HTYPE_MIN skipped; because DUK_HTYPE_MIN is zero
 * and the comparison is unsigned, it's always true and generates warnings.
 */
#define DUK_HEAPHDR_HTYPE_VALID(h) (DUK_HEAPHDR_GET_HTYPE((h)) <= DUK_HTYPE_MAX)

#define DUK_HEAPHDR_SET_FLAG_BITS(h, bits) \
	do { \
		DUK_ASSERT(((bits) & ~(DUK_HEAPHDR_FLAGS_FLAG_MASK)) == 0); \
		(h)->h_flags |= (bits); \
	} while (0)

#define DUK_HEAPHDR_CLEAR_FLAG_BITS(h, bits) \
	do { \
		DUK_ASSERT(((bits) & ~(DUK_HEAPHDR_FLAGS_FLAG_MASK)) == 0); \
		(h)->h_flags &= ~((bits)); \
	} while (0)

#define DUK_HEAPHDR_CHECK_FLAG_BITS(h, bits) (((h)->h_flags & (bits)) != 0)

#define DUK_HEAPHDR_SET_REACHABLE(h)   DUK_HEAPHDR_SET_FLAG_BITS((h), DUK_HEAPHDR_FLAG_REACHABLE)
#define DUK_HEAPHDR_CLEAR_REACHABLE(h) DUK_HEAPHDR_CLEAR_FLAG_BITS((h), DUK_HEAPHDR_FLAG_REACHABLE)
#define DUK_HEAPHDR_HAS_REACHABLE(h)   DUK_HEAPHDR_CHECK_FLAG_BITS((h), DUK_HEAPHDR_FLAG_REACHABLE)

#define DUK_HEAPHDR_SET_TEMPROOT(h)   DUK_HEAPHDR_SET_FLAG_BITS((h), DUK_HEAPHDR_FLAG_TEMPROOT)
#define DUK_HEAPHDR_CLEAR_TEMPROOT(h) DUK_HEAPHDR_CLEAR_FLAG_BITS((h), DUK_HEAPHDR_FLAG_TEMPROOT)
#define DUK_HEAPHDR_HAS_TEMPROOT(h)   DUK_HEAPHDR_CHECK_FLAG_BITS((h), DUK_HEAPHDR_FLAG_TEMPROOT)

#define DUK_HEAPHDR_SET_FINALIZABLE(h)   DUK_HEAPHDR_SET_FLAG_BITS((h), DUK_HEAPHDR_FLAG_FINALIZABLE)
#define DUK_HEAPHDR_CLEAR_FINALIZABLE(h) DUK_HEAPHDR_CLEAR_FLAG_BITS((h), DUK_HEAPHDR_FLAG_FINALIZABLE)
#define DUK_HEAPHDR_HAS_FINALIZABLE(h)   DUK_HEAPHDR_CHECK_FLAG_BITS((h), DUK_HEAPHDR_FLAG_FINALIZABLE)

#define DUK_HEAPHDR_SET_FINALIZED(h)   DUK_HEAPHDR_SET_FLAG_BITS((h), DUK_HEAPHDR_FLAG_FINALIZED)
#define DUK_HEAPHDR_CLEAR_FINALIZED(h) DUK_HEAPHDR_CLEAR_FLAG_BITS((h), DUK_HEAPHDR_FLAG_FINALIZED)
#define DUK_HEAPHDR_HAS_FINALIZED(h)   DUK_HEAPHDR_CHECK_FLAG_BITS((h), DUK_HEAPHDR_FLAG_FINALIZED)

#define DUK_HEAPHDR_SET_READONLY(h)   DUK_HEAPHDR_SET_FLAG_BITS((h), DUK_HEAPHDR_FLAG_READONLY)
#define DUK_HEAPHDR_CLEAR_READONLY(h) DUK_HEAPHDR_CLEAR_FLAG_BITS((h), DUK_HEAPHDR_FLAG_READONLY)
#define DUK_HEAPHDR_HAS_READONLY(h)   DUK_HEAPHDR_CHECK_FLAG_BITS((h), DUK_HEAPHDR_FLAG_READONLY)

/* Init pointer fields to NULL. */
#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
#define DUK_HEAPHDR_INIT_NULLS(h) \
	do { \
		DUK_HEAPHDR_SET_NEXT((h), (void *) NULL); \
		DUK_HEAPHDR_SET_PREV((h), (void *) NULL); \
	} while (0)
#else
#define DUK_HEAPHDR_INIT_NULLS(h) \
	do { \
		DUK_HEAPHDR_SET_NEXT((h), (void *) NULL); \
	} while (0)
#endif

#define DUK_HEAPHDR_STRING_INIT_NULLS(h) \
	do { \
		(h)->h_next = NULL; \
	} while (0)

/*
 *  Assert helpers
 */

/* Check that prev/next links are consistent: if e.g. h->prev is != NULL,
 * h->prev->next should point back to h.
 */
#if defined(DUK_USE_ASSERTIONS)
DUK_INTERNAL_DECL void duk_heaphdr_assert_valid_subclassed(duk_heap *heap, duk_heaphdr *h);
DUK_INTERNAL_DECL void duk_heaphdr_assert_links(duk_heap *heap, duk_heaphdr *h);
DUK_INTERNAL_DECL void duk_heaphdr_assert_valid(duk_heaphdr *h);
#define DUK_HEAPHDR_ASSERT_LINKS(heap, h) \
	do { \
		duk_heaphdr_assert_links((heap), (h)); \
	} while (0)
#define DUK_HEAPHDR_ASSERT_VALID(h) \
	do { \
		duk_heaphdr_assert_valid((h)); \
	} while (0)
#else
#define DUK_HEAPHDR_ASSERT_LINKS(heap, h) \
	do { \
	} while (0)
#define DUK_HEAPHDR_ASSERT_VALID(h) \
	do { \
	} while (0)
#endif

#endif /* DUK_HEAPHDR_H_INCLUDED */
