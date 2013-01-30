/*
 *  Bitstream decoder.
 */

#include "duk_internal.h"

duk_u32 duk_bd_decode(duk_bitdecoder_ctx *ctx, int bits) {
	int shift;
	int mask;
	int tmp;

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

int duk_bd_decode_flag(duk_bitdecoder_ctx *ctx) {
	return (int) duk_bd_decode(ctx, 1);
}

