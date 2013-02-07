/*
 *  Bitstream encoder.
 */

#include "duk_internal.h"

void duk_be_encode(duk_bitencoder_ctx *ctx, duk_u32 data, int bits) {
	int tmp;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(ctx->currbits < 8);

	/* This limitation would be fixable but adds unnecessary complexity. */
	DUK_ASSERT(bits >= 1 && bits <= 24);

	ctx->currval = (ctx->currval << bits) | data;
	ctx->currbits += bits;

	while (ctx->currbits >= 8) {
		tmp = (ctx->currval >> (ctx->currbits - 8)) & 0xff;

		if (ctx->offset < ctx->length) {
			ctx->data[ctx->offset++] = (duk_u8) tmp;
		} else {
			/* If buffer has been exhausted, truncate bitstream */
			ctx->truncated = 1;
		}

		ctx->currbits -= 8;
	}
}

void duk_be_finish(duk_bitencoder_ctx *ctx) {
	int npad;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(ctx->currbits < 8);

	npad = 8 - ctx->currbits;
	if (npad > 0) {
		duk_be_encode(ctx, 0, npad);
	}
	DUK_ASSERT(ctx->currbits == 0);
}

