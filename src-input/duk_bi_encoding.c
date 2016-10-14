/*
 *  WHATWG Encoding API built-ins
 *
 *  API specification: https://encoding.spec.whatwg.org/#api
 *  Web IDL: https://www.w3.org/TR/WebIDL/
 */

#include "duk_internal.h"

#if defined(DUK_USE_ENCODING_BUILTINS)
typedef struct {
	duk_uint8_t *out;      /* where to write next byte(s) */
	duk_codepoint_t lead;  /* lead surrogate */
} duk__encode_context;

typedef struct {
	duk_bool_t bom_seen;
	duk_bool_t fatal;
	duk_bool_t ignore_bom;

	/* UTF-8 decoding state */
	duk_codepoint_t codepoint;  /* built up incrementally */
	duk_int_t needed;           /* how many more bytes we need */
	duk_uint8_t upper;          /* max value of next byte (decode error otherwise) */
	duk_uint8_t lower;          /* min value of next byte (ditto) */
} duk__decode_context;

#define DUK__U8_CONTINUE   0  /* continue to next byte */
#define DUK__U8_CODEPOINT  1  /* have codepoint */
#define DUK__U8_ERROR      2  /* decoding error */
#define DUK__U8_RETRY      3  /* retry last byte, constraint: RETRY > ERROR */

DUK_LOCAL void duk__utf8_decode_init(duk__decode_context *dec_ctx) {
	dec_ctx->codepoint = 0x0000L;
	dec_ctx->needed = 0;
	dec_ctx->upper = 0xbf;
	dec_ctx->lower = 0x80;
	dec_ctx->bom_seen = 0;
}

DUK_LOCAL int duk__utf8_decode_next(duk__decode_context *dec_ctx, duk_uint8_t byte, duk_codepoint_t *cp) {
	/*
	 *  UTF-8 algorithm based on the Encoding specification:
	 *  https://encoding.spec.whatwg.org/#utf-8-decoder
	 */

	if (dec_ctx->needed == 0) {
		/* process initial byte */
		if (byte <= 0x7f) {
			/* U+0000-U+007F, 1 byte (ASCII) */
			*cp = (duk_codepoint_t) byte;
			return DUK__U8_CODEPOINT;
		} else if (byte >= 0xc2 && byte <= 0xdf) {
			/* U+0080-U+07FF, 2 bytes */
			dec_ctx->needed = 1;
			dec_ctx->codepoint = byte & 0x1f;
			return DUK__U8_CONTINUE;
		} else if (byte >= 0xe0 && byte <= 0xef) {
			/* U+0800-U+FFFF, 3 bytes */
			if (byte == 0xe0) {
				dec_ctx->lower = 0xa0;
			} else if (byte == 0xed) {
				dec_ctx->upper = 0x9f;
			}
			dec_ctx->needed = 2;
			dec_ctx->codepoint = byte & 0xf;
			return DUK__U8_CONTINUE;
		} else if (byte >= 0xf0 && byte <= 0xf4) {
			/* U+010000-U+10FFFF, 4 bytes */
			if (byte == 0xf0) {
				dec_ctx->lower = 0x90;
			} else if (byte == 0xf4) {
				dec_ctx->upper = 0x8f;
			}
			dec_ctx->needed = 3;
			dec_ctx->codepoint = byte & 0x7;
			return DUK__U8_CONTINUE;
		} else {
			/* not a legal initial byte */
			return DUK__U8_ERROR;
		}
	} else {
		/* process continuation byte */
		if (byte >= dec_ctx->lower && byte <= dec_ctx->upper) {
			dec_ctx->lower = 0x80;
			dec_ctx->upper = 0xbf;
			dec_ctx->codepoint = (dec_ctx->codepoint << 6) | (byte & 0x3f);
			if (--dec_ctx->needed > 0) {
				/* need more bytes */
				return DUK__U8_CONTINUE;
			} else {
				/* got a codepoint */
				*cp = dec_ctx->codepoint;
				dec_ctx->codepoint = 0x0000L;
				dec_ctx->needed = 0;
				return DUK__U8_CODEPOINT;
			}
		} else {
			/* We just encountered an illegal UTF-8 continuation byte.  This might
			 * be the initial byte of the next character; if we return a plain
			 * error status and the decoder is in replacement mode, the character
			 * will be masked.  We still need to alert the caller to the error
			 * though.
			 */
			dec_ctx->codepoint = 0x0000L;
			dec_ctx->needed = 0;
			dec_ctx->lower = 0x80;
			dec_ctx->upper = 0xbf;
			return DUK__U8_RETRY;
		}
	}
}

DUK_LOCAL void duk__utf8_encode_char(void *udata, duk_codepoint_t codepoint) {
	duk__encode_context *enc_ctx;

	enc_ctx = (duk__encode_context *) udata;
	if (codepoint > 0x10ffffL) {
		/* cannot legally encode in UTF-8 */
		enc_ctx->out += duk_unicode_encode_xutf8(DUK_UNICODE_CP_REPLACEMENT_CHARACTER, enc_ctx->out);
	} else if (codepoint >= 0xd800L && codepoint <= 0xdbffL) {
		/* high surrogate */
		if (enc_ctx->lead != 0x0000L) {
			/* consecutive high surrogates, consider first one unpaired */
			enc_ctx->out += duk_unicode_encode_xutf8(DUK_UNICODE_CP_REPLACEMENT_CHARACTER, enc_ctx->out);
		}
		enc_ctx->lead = codepoint;
	} else if (codepoint >= 0xdc00L && codepoint <= 0xdfffL) {
		/* low surrogate */
		if (enc_ctx->lead != 0x0000L) {
			codepoint = 0x010000L | ((enc_ctx->lead - 0xd800L) << 10) | (codepoint - 0xdc00L);
			enc_ctx->out += duk_unicode_encode_xutf8(codepoint, enc_ctx->out);
			enc_ctx->lead = 0x0000L;
		} else {
			/* unpaired low surrogate */
			enc_ctx->out += duk_unicode_encode_xutf8(DUK_UNICODE_CP_REPLACEMENT_CHARACTER, enc_ctx->out);
		}
	} else {
		if (enc_ctx->lead != 0x0000L) {
			/* unpaired high surrogate */
			enc_ctx->out += duk_unicode_encode_xutf8(DUK_UNICODE_CP_REPLACEMENT_CHARACTER, enc_ctx->out);
			enc_ctx->lead = 0x0000L;
		}
		enc_ctx->out += duk_unicode_encode_xutf8(codepoint, enc_ctx->out);
	}
}

DUK_INTERNAL duk_ret_t duk_bi_textencoder_constructor(duk_context *ctx) {
	/*
	 *  TextEncoder currently requires no persistent state, so the constructor
	 *  does nothing on purpose.
	 */

	if (!duk_is_constructor_call(ctx)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "constructor must use 'new'");
	}
	return 0;
}

DUK_INTERNAL duk_ret_t duk_bi_textencoder_prototype_encoding_getter(duk_context *ctx) {
	duk_push_string(ctx, "utf-8");
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_textencoder_prototype_encode(duk_context *ctx) {
	duk__encode_context enc_ctx;
	duk_size_t len;
	duk_uint8_t *output;

	if (!duk_is_undefined(ctx, 0)) {
		duk_to_string(ctx, 0);
	} else {
		duk_push_string(ctx, "");
		duk_replace(ctx, 0);
	}

	len = duk_get_length(ctx, 0);
	if (len >= DUK_HBUFFER_MAX_BYTELEN / 3) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "input too large");
	}

	/* Allowance is 3*len because all bytes can potentially be replaced with
	 * U+FFFD--which rather inconveniently encodes to 3 bytes in UTF-8.
	 *
	 * XXX: The buffer allocation strategy used here is rather inefficient.
	 * Maybe switch to a chunk-based strategy, or preprocess the string to
	 * figure out the space needed ahead of time?
	 */
	output = (duk_uint8_t *) duk_push_dynamic_buffer(ctx, 3 * len);

	/* XXX: duk_decode_string() is used to process the input string.  For strings
	 * originating in C, this means we will see the string the same way Ecmascript
	 * code would.  But maybe we need something stricter in this case?
	 */
	enc_ctx.lead = 0x0000L;
	enc_ctx.out = output;
	duk_decode_string(ctx, 0, duk__utf8_encode_char, (void *) &enc_ctx);
	if (enc_ctx.lead != 0x0000L) {
		/* unpaired high surrogate at end of string */
		enc_ctx.out += duk_unicode_encode_xutf8(DUK_UNICODE_CP_REPLACEMENT_CHARACTER, enc_ctx.out);
		DUK_ASSERT(enc_ctx.out <= output + (3 * len));
	}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	duk_push_buffer_object(ctx, -1, 0, (duk_size_t) (enc_ctx.out - output), DUK_BUFOBJ_UINT8ARRAY);
#else
	/* shrink buffer to include only the encoded text */
	duk_resize_buffer(ctx, -1, (duk_size_t) (enc_ctx.out - output));
#endif
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_textdecoder_constructor(duk_context *ctx) {
	duk__decode_context *dec_ctx;
	duk_bool_t fatal = 0;
	duk_bool_t ignore_bom = 0;

	if (!duk_is_constructor_call(ctx)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "constructor must use 'new'");
	}
	if (!duk_is_undefined(ctx, 0)) {
		/* XXX: for now, 'label' argument is ignored */
		duk_to_string(ctx, 0);
	}
	if (!duk_is_null_or_undefined(ctx, 1)) {
		if (duk_get_prop_string(ctx, 1, "fatal")) {
			fatal = duk_to_boolean(ctx, -1);
		}
		if (duk_get_prop_string(ctx, 1, "ignoreBOM")) {
			ignore_bom = duk_to_boolean(ctx, -1);
		}
	}

	dec_ctx = (duk__decode_context *) duk_push_fixed_buffer(ctx, sizeof(duk__decode_context));
	dec_ctx->fatal = fatal;
	dec_ctx->ignore_bom = ignore_bom;
	duk__utf8_decode_init(dec_ctx);

	duk_push_this(ctx);
	duk_dup(ctx, -2);
	duk_put_prop_string(ctx, -2, "\xff" "Context");
	return 0;
}

DUK_INTERNAL duk_ret_t duk_bi_textdecoder_prototype_encoding_getter(duk_context *ctx) {
	duk__decode_context *dec_ctx;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "\xff" "Context");
	dec_ctx = (duk__decode_context *) duk_require_buffer(ctx, -1, NULL);
	DUK_ASSERT(dec_ctx != NULL);

	/* XXX: change to look up from dec_ctx once other encodings are supported */
	DUK_UNREF(dec_ctx);
	duk_push_string(ctx, "utf-8");
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_textdecoder_prototype_fatal_getter(duk_context *ctx) {
	duk__decode_context *dec_ctx;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "\xff" "Context");
	dec_ctx = (duk__decode_context *) duk_require_buffer(ctx, -1, NULL);
	DUK_ASSERT(dec_ctx != NULL);

	duk_push_boolean(ctx, dec_ctx->fatal);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_textdecoder_prototype_ignorebom_getter(duk_context *ctx) {
	duk__decode_context *dec_ctx;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "\xff" "Context");
	dec_ctx = (duk__decode_context *) duk_require_buffer(ctx, -1, NULL);

	duk_push_boolean(ctx, dec_ctx->ignore_bom);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_textdecoder_prototype_decode(duk_context *ctx) {
	const duk_uint8_t *input = (const duk_uint8_t *) "";
	duk__decode_context *dec_ctx;
	duk_size_t len = 0;
	duk_bool_t stream = 0;
	duk_codepoint_t codepoint;
	int ret;
	duk_uint8_t *output;
	const duk_uint8_t *in;
	duk_uint8_t *out;

	if (!duk_is_undefined(ctx, 0)) {
		input = (const duk_uint8_t *) duk_require_buffer_data(ctx, 0, &len);
	}
	if (!duk_is_null_or_undefined(ctx, 1)) {
		if (duk_get_prop_string(ctx, 1, "stream")) {
			stream = duk_to_boolean(ctx, -1);
		}
	}
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "\xff" "Context");
	dec_ctx = (duk__decode_context *) duk_require_buffer(ctx, -1, NULL);
	DUK_ASSERT(dec_ctx != NULL);

	if (len >= (DUK_HBUFFER_MAX_BYTELEN / 3) - 3) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "input too large");
	}

	/* Allowance is 3*len in the general case because all bytes may potentially
	 * become U+FFFD.  If the first byte completes a non-BMP codepoint it will
	 * decode to a CESU-8 surrogate pair (6 bytes) so we allow 3 extra bytes to
	 * compensate: (1*3)+3 = 6.  Non-BMP codepoints are safe otherwise because
	 * the 4->6 expansion is well under the 3x allowance.
	 *
	 * XXX: As with TextEncoder, need a better buffer allocation strategy here.
	 */
	output = (duk_uint8_t *) duk_push_fixed_buffer(ctx, 3 + (3 * len));

	in = input;
	out = output;
	DUK_ASSERT(DUK__U8_RETRY > DUK__U8_ERROR);
	while (in < input + len) {
		ret = duk__utf8_decode_next(dec_ctx, *in++, &codepoint);
		if (ret == DUK__U8_CONTINUE) {
			continue;
		}
		if (ret >= DUK__U8_ERROR) {
			/* decoding error */
			DUK_ASSERT(ret == DUK__U8_ERROR || ret == DUK__U8_RETRY);
			if (dec_ctx->fatal) {
				/* fatal mode: throw a TypeError */
				goto fatal;
			} else {
				/* replacement mode: replace with U+FFFD */
				codepoint = DUK_UNICODE_CP_REPLACEMENT_CHARACTER;
			}
		}
		if (ret == DUK__U8_RETRY) {
			--in;  /* retry last byte */
		}
		if (!dec_ctx->bom_seen) {
			dec_ctx->bom_seen = 1;
			if (codepoint == 0xfeffL && !dec_ctx->ignore_bom) {
				continue;
			}
		}
		out += duk_unicode_encode_cesu8(codepoint, out);
		DUK_ASSERT(out <= output + (3 + (3 * len)));
	}

	if (!stream) {
		if (dec_ctx->needed != 0) {
			/* truncated sequence at end of buffer */
			if (dec_ctx->fatal) {
				goto fatal;
			} else {
				out += duk_unicode_encode_cesu8(DUK_UNICODE_CP_REPLACEMENT_CHARACTER, out);
				DUK_ASSERT(out <= output + (3 + (3 * len)));
			}
		}
		duk__utf8_decode_init(dec_ctx);
	}

	duk_push_lstring(ctx, (const char *) output, (duk_size_t) (out - output));
	return 1;

fatal:
	duk_error(ctx, DUK_ERR_TYPE_ERROR, "cannot decode as utf-8");
	DUK_UNREACHABLE();
}
#endif  /* DUK_USE_ENCODING_BUILTINS */
