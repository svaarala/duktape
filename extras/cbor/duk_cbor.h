#if !defined(DUK_CBOR_H_INCLUDED)
#define DUK_CBOR_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

extern void duk_cbor_encode(duk_context *ctx, duk_idx_t idx, duk_uint_t encode_flags);
extern void duk_cbor_decode(duk_context *ctx, duk_idx_t idx, duk_uint_t decode_flags);

#if defined(__cplusplus)
}
#endif  /* end 'extern "C"' wrapper */

#endif /* DUK_CBOR_H_INCLUDED */
