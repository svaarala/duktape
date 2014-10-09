/*
 *  Utilities
 */

#ifndef DUK_UTIL_H_INCLUDED
#define DUK_UTIL_H_INCLUDED

#define DUK_UTIL_MIN_HASH_PRIME  17  /* must match genhashsizes.py */

#define DUK_UTIL_GET_HASH_PROBE_STEP(hash)  (duk_util_probe_steps[(hash) & 0x1f])

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
 *  Externs and prototypes
 */

DUK_INTERNAL_DECL duk_uint8_t duk_lc_digits[36];
DUK_INTERNAL_DECL duk_uint8_t duk_uc_nybbles[16];
DUK_INTERNAL_DECL duk_int8_t duk_hex_dectab[256];

/* Note: assumes that duk_util_probe_steps size is 32 */
DUK_INTERNAL_DECL duk_uint8_t duk_util_probe_steps[32];

DUK_INTERNAL_DECL duk_uint32_t duk_util_hashbytes(duk_uint8_t *data, duk_size_t len, duk_uint32_t seed);

DUK_INTERNAL_DECL duk_uint32_t duk_util_get_hash_prime(duk_uint32_t size);

DUK_INTERNAL_DECL duk_int32_t duk_bd_decode(duk_bitdecoder_ctx *ctx, duk_small_int_t bits);
DUK_INTERNAL_DECL duk_small_int_t duk_bd_decode_flag(duk_bitdecoder_ctx *ctx);
DUK_INTERNAL_DECL duk_int32_t duk_bd_decode_flagged(duk_bitdecoder_ctx *ctx, duk_small_int_t bits, duk_int32_t def_value);

DUK_INTERNAL_DECL void duk_be_encode(duk_bitencoder_ctx *ctx, duk_uint32_t data, duk_small_int_t bits);
DUK_INTERNAL_DECL void duk_be_finish(duk_bitencoder_ctx *ctx);

DUK_INTERNAL_DECL duk_uint32_t duk_util_tinyrandom_get_bits(duk_hthread *thr, duk_small_int_t n);
DUK_INTERNAL_DECL duk_double_t duk_util_tinyrandom_get_double(duk_hthread *thr);

#endif  /* DUK_UTIL_H_INCLUDED */
