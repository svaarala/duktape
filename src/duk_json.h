/*
 *  Defines for JSON, especially duk_bi_json.c.
 */

#ifndef DUK_JSON_H_INCLUDED
#define DUK_JSON_H_INCLUDED

/* Object/array recursion limit (to protect C stack) */
#if defined(DUK_USE_DEEP_C_STACK)
#define DUK_JSON_ENC_RECURSION_LIMIT          1000
#define DUK_JSON_DEC_RECURSION_LIMIT          1000
#else
#define DUK_JSON_ENC_RECURSION_LIMIT          100
#define DUK_JSON_DEC_RECURSION_LIMIT          100
#endif

/* Encoding/decoding flags */
#define DUK_JSON_FLAG_ASCII_ONLY          (1 << 0)  /* escape any non-ASCII characters */
#define DUK_JSON_FLAG_AVOID_KEY_QUOTES    (1 << 1)  /* avoid key quotes when key is an ASCII Identifier */
#define DUK_JSON_FLAG_EXT_CUSTOM          (1 << 2)  /* extended types: custom encoding */
#define DUK_JSON_FLAG_EXT_COMPATIBLE      (1 << 3)  /* extended types: compatible encoding */

/* How much stack to require on entry to object/array encode */
#define DUK_JSON_ENC_REQSTACK                 32

/* How much stack to require on entry to object/array decode */
#define DUK_JSON_DEC_REQSTACK                 32

/* Encoding state.  Heap object references are all borrowed. */
typedef struct {
	duk_hthread *thr;
	duk_hbuffer_dynamic *h_buf;
	duk_hobject *h_replacer;     /* replacer function */
	duk_hstring *h_gap;          /* gap (if empty string, NULL) */
	duk_hstring *h_indent;       /* current indent (if gap is NULL, this is NULL) */
	int idx_proplist;            /* explicit PropertyList */
	int idx_loop;                /* valstack index of loop detection object */
	int flags;
	int flag_ascii_only;
	int flag_avoid_key_quotes;
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	int flag_ext_custom;
	int flag_ext_compatible;
#endif
	int recursion_depth;
	int recursion_limit;
	int mask_for_undefined;      /* type bit mask: types which certainly produce 'undefined' */
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	int stridx_custom_undefined;
	int stridx_custom_nan;
	int stridx_custom_neginf;
	int stridx_custom_posinf;
	int stridx_custom_function;
#endif
} duk_json_enc_ctx;

typedef struct {
	duk_hthread *thr;
	duk_uint8_t *p;
	duk_uint8_t *p_end;
	int idx_reviver;
	int flags;
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	int flag_ext_custom;
	int flag_ext_compatible;
#endif
	int recursion_depth;
	int recursion_limit;
} duk_json_dec_ctx;

#endif  /* DUK_JSON_H_INCLUDED */

