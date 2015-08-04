/*
 *  Utilities
 */

#ifndef DUK_UTIL_H_INCLUDED
#define DUK_UTIL_H_INCLUDED

#define DUK_UTIL_MIN_HASH_PRIME  17  /* must match genhashsizes.py */

#define DUK_UTIL_GET_HASH_PROBE_STEP(hash)  (duk_util_probe_steps[(hash) & 0x1f])

/*
 *  Endian conversion
 */

#if defined(DUK_USE_INTEGER_LE)
#define DUK_HTON32(x) DUK_BSWAP32((x))
#define DUK_NTOH32(x) DUK_BSWAP32((x))
#define DUK_HTON16(x) DUK_BSWAP16((x))
#define DUK_NTOH16(x) DUK_BSWAP16((x))
#elif defined(DUK_USE_INTEGER_BE)
#define DUK_HTON32(x) (x)
#define DUK_NTOH32(x) (x)
#define DUK_HTON16(x) (x)
#define DUK_NTOH16(x) (x)
#else
#error internal error, endianness defines broken
#endif

/*
 *  Bitstream decoder
 */

struct duk_bitdecoder_ctx {
	const duk_uint8_t *data;
	duk_size_t offset;
	duk_size_t length;
	duk_uint32_t currval;
	duk_small_int_t currbits;
};

/*
 *  Bitstream encoder
 */

struct duk_bitencoder_ctx {
	duk_uint8_t *data;
	duk_size_t offset;
	duk_size_t length;
	duk_uint32_t currval;
	duk_small_int_t currbits;
	duk_small_int_t truncated;
};

/*
 *  Buffer writer (dynamic buffer only)
 */

struct duk_bufwriter_ctx {
	duk_uint8_t *limit;
	duk_size_t offset;
	duk_size_t length;
	duk_hbuffer_dynamic *buf;
};

#define DUK_BW_SPARE_ADD           256
#define DUK_BW_SPARE_SHIFT         4    /* 2^4 -> 1/16 = 6.25% spare */

#define DUK_BW_INIT(thr,bw_ctx,buf) \
	duk_bw_init((thr),(bw_ctx),(buf))
#define DUK_BW_GETPTR(thr,bw_ctx) \
	duk_bw_getptr((thr),(bw_ctx))
#define DUK_BW_ENSURE(thr,bw_ctx,sz,ptr) \
	(((duk_size_t) ((bw_ctx)->limit - (ptr)) >= (sz)) ? (ptr) : duk_bw_resize((thr),(bw_ctx),(sz),(ptr)))
#define DUK_BW_FINISH(thr,bw_ctx,ptr) \
	duk_bw_finish((thr),(bw_ctx),(ptr))
#define DUK_BW_COMPACT(thr,bw_ctx) \
	duk_bw_compact((thr),(bw_ctx))

/*
 *  Raw write/read macros for big endian, unaligned basic values.
 *  Caller ensures there's enough space.  The macros update the pointer
 *  argument automatically on resizes.  The idiom seems a bit odd, but
 *  leads to compact code.
 */

#define DUK_RAW_WRITE_U8(ptr,val)  do { \
		*(ptr)++ = (duk_uint8_t) (val); \
	} while (0)
#define DUK_RAW_WRITE_U16_BE(ptr,val) duk_raw_write_u16_be(&(ptr), (duk_uint16_t) (val))
#define DUK_RAW_WRITE_U32_BE(ptr,val) duk_raw_write_u32_be(&(ptr), (duk_uint32_t) (val))
#define DUK_RAW_WRITE_DOUBLE_BE(ptr,val) duk_raw_write_double_be(&(ptr), (duk_double_t) (val))

#define DUK_RAW_READ_U8(ptr) ((duk_uint8_t) (*(ptr)++))
#define DUK_RAW_READ_U16_BE(ptr) duk_raw_read_u16_be(&(ptr));
#define DUK_RAW_READ_U32_BE(ptr) duk_raw_read_u32_be(&(ptr));
#define DUK_RAW_READ_DOUBLE_BE(ptr) duk_raw_read_double_be(&(ptr));

/*
 *  Externs and prototypes
 */

#if !defined(DUK_SINGLE_FILE)
DUK_INTERNAL_DECL duk_uint8_t duk_lc_digits[36];
DUK_INTERNAL_DECL duk_uint8_t duk_uc_nybbles[16];
DUK_INTERNAL_DECL duk_int8_t duk_hex_dectab[256];
#endif  /* !DUK_SINGLE_FILE */

/* Note: assumes that duk_util_probe_steps size is 32 */
#if defined(DUK_USE_HOBJECT_HASH_PART) || defined(DUK_USE_STRTAB_PROBE)
#if !defined(DUK_SINGLE_FILE)
DUK_INTERNAL_DECL duk_uint8_t duk_util_probe_steps[32];
#endif  /* !DUK_SINGLE_FILE */
#endif

DUK_INTERNAL_DECL duk_uint32_t duk_util_hashbytes(const duk_uint8_t *data, duk_size_t len, duk_uint32_t seed);

#if defined(DUK_USE_HOBJECT_HASH_PART) || defined(DUK_USE_STRTAB_PROBE)
DUK_INTERNAL_DECL duk_uint32_t duk_util_get_hash_prime(duk_uint32_t size);
#endif

DUK_INTERNAL_DECL duk_int32_t duk_bd_decode(duk_bitdecoder_ctx *ctx, duk_small_int_t bits);
DUK_INTERNAL_DECL duk_small_int_t duk_bd_decode_flag(duk_bitdecoder_ctx *ctx);
DUK_INTERNAL_DECL duk_int32_t duk_bd_decode_flagged(duk_bitdecoder_ctx *ctx, duk_small_int_t bits, duk_int32_t def_value);

DUK_INTERNAL_DECL void duk_be_encode(duk_bitencoder_ctx *ctx, duk_uint32_t data, duk_small_int_t bits);
DUK_INTERNAL_DECL void duk_be_finish(duk_bitencoder_ctx *ctx);

DUK_INTERNAL_DECL duk_uint32_t duk_util_tinyrandom_get_bits(duk_hthread *thr, duk_small_int_t n);
DUK_INTERNAL_DECL duk_double_t duk_util_tinyrandom_get_double(duk_hthread *thr);

DUK_INTERNAL_DECL void duk_bw_init(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_hbuffer_dynamic *h_buf);
DUK_INTERNAL_DECL duk_uint8_t *duk_bw_getptr(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx);
DUK_INTERNAL_DECL duk_uint8_t *duk_bw_resize(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_size_t sz, duk_uint8_t *ptr);
DUK_INTERNAL_DECL void duk_bw_finish(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_uint8_t *ptr);
DUK_INTERNAL_DECL void duk_bw_compact(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx);

DUK_ALWAYS_INLINE DUK_INTERNAL_DECL duk_uint16_t duk_raw_read_u16_be(duk_uint8_t **p);
DUK_ALWAYS_INLINE DUK_INTERNAL_DECL duk_uint32_t duk_raw_read_u32_be(duk_uint8_t **p);
DUK_ALWAYS_INLINE DUK_INTERNAL_DECL duk_double_t duk_raw_read_double_be(duk_uint8_t **p);
DUK_ALWAYS_INLINE DUK_INTERNAL_DECL void duk_raw_write_u16_be(duk_uint8_t **p, duk_uint16_t val);
DUK_ALWAYS_INLINE DUK_INTERNAL_DECL void duk_raw_write_u32_be(duk_uint8_t **p, duk_uint32_t val);
DUK_ALWAYS_INLINE DUK_INTERNAL_DECL void duk_raw_write_double_be(duk_uint8_t **p, duk_double_t val);

#if defined(DUK_USE_DEBUGGER_SUPPORT)  /* For now only needed by the debugger. */
DUK_INTERNAL void duk_byteswap_bytes(duk_uint8_t *p, duk_small_uint_t len);
#endif

#endif  /* DUK_UTIL_H_INCLUDED */
