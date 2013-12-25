/*
 *  Bitstream decoder.
 */

#include "duk_internal.h"

/* Decode 'bits' bits from the input stream (bits must be 1...24).
 * When reading past bitstream end, zeroes are shifted in.  The result
 * is signed to match duk_bd_decode_flagged.
 */
duk_int32_t duk_bd_decode(duk_bitdecoder_ctx *ctx, duk_small_int_t bits) {
	duk_small_int_t shift;
	duk_uint32_t mask;
	duk_uint32_t tmp;

	/* Note: cannot read more than 24 bits without possibly shifting top bits out.
	 * Fixable, but adds complexity.
	 */
	DUK_ASSERT(bits >= 1 && bits <= 24);

	while (ctx->currbits < bits) {
#if 0
		DUK_DDDPRINT("decode_bits: shift more data (bits=%d, currbits=%d)", bits, ctx->currbits);
#endif
		ctx->currval <<= 8;
		if (ctx->offset < ctx->length) {
			/* If ctx->offset >= ctx->length, we "shift zeroes in"
			 * instead of croaking.
			 */
			ctx->currval |= ctx->data[ctx->offset++];
		}
		ctx->currbits += 8;
	}
#if 0
	DUK_DDDPRINT("decode_bits: bits=%d, currbits=%d, currval=0x%08x", bits, ctx->currbits, ctx->currval);
#endif

	/* Extract 'top' bits of currval; note that the extracted bits do not need
	 * to be cleared, we just ignore them on next round.
	 */
	shift = ctx->currbits - bits;
	mask = (1 << bits) - 1;
	tmp = (ctx->currval >> shift) & mask;
	ctx->currbits = shift;  /* remaining */

#if 0
	DUK_DDDPRINT("decode_bits: %d bits -> 0x%08x (%d), currbits=%d, currval=0x%08x",
	             bits, tmp, tmp, ctx->currbits, ctx->currval);
#endif

	return tmp;
}

duk_small_int_t duk_bd_decode_flag(duk_bitdecoder_ctx *ctx) {
	return (duk_small_int_t) duk_bd_decode(ctx, 1);
}

/* Decode a one-bit flag, and if set, decode a value of 'bits', otherwise return
 * default value.  Return value is signed so that negative marker value can be
 * used by caller as a "not present" value.
 */
duk_int32_t duk_bd_decode_flagged(duk_bitdecoder_ctx *ctx, duk_small_int_t bits, duk_int32_t def_value) {
	if (duk_bd_decode_flag(ctx)) {
		return (duk_int32_t) duk_bd_decode(ctx, bits);
	} else {
		return def_value;
	}
}

