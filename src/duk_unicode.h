/*
 *  Unicode helpers
 */

#ifndef DUK_UNICODE_H_INCLUDED
#define DUK_UNICODE_H_INCLUDED

#define  DUK_UNICODE_MAX_XUTF8_LENGTH   7   /* up to 36 bit codepoints */
#define  DUK_UNICODE_MAX_CESU8_LENGTH   6   /* all codepoints up to U+10FFFF */

#define  DUK_UNICODE_CP_ZWNJ                   0x200cUL  /* zero-width non-joiner */
#define  DUK_UNICODE_CP_ZWJ                    0x200dUL  /* zero-width joiner */
#define  DUK_UNICODE_CP_REPLACEMENT_CHARACTER  0xfffdUL  /* http://en.wikipedia.org/wiki/Replacement_character#Replacement_character */

#ifdef DUK_USE_SOURCE_NONBMP
#include "duk_unicode_ids_noa.h"
#else
#include "duk_unicode_ids_noa_bmpo.h"
#endif

#ifdef DUK_USE_SOURCE_NONBMP
#include "duk_unicode_idp_m_ids_noa.h"
#else
#include "duk_unicode_idp_m_ids_noa_bmpo.h"
#endif

#include "duk_unicode_caseconv.h"

/*
 *  Extern
 */

/* duk_unicode_support.c */
extern duk_uint8_t duk_unicode_xutf8_markers[7];
extern duk_uint16_t duk_unicode_re_ranges_digit[2];
extern duk_uint16_t duk_unicode_re_ranges_white[22];
extern duk_uint16_t duk_unicode_re_ranges_wordchar[8];
extern duk_uint16_t duk_unicode_re_ranges_not_digit[4];
extern duk_uint16_t duk_unicode_re_ranges_not_white[24];
extern duk_uint16_t duk_unicode_re_ranges_not_wordchar[10];

/*
 *  Prototypes
 */

duk_small_int_t duk_unicode_get_xutf8_length(duk_codepoint_t cp);
duk_small_int_t duk_unicode_encode_xutf8(duk_codepoint_t cp, duk_uint8_t *out);
duk_small_int_t duk_unicode_encode_cesu8(duk_codepoint_t cp, duk_uint8_t *out);
duk_small_int_t duk_unicode_decode_xutf8(duk_hthread *thr, duk_uint8_t **ptr, duk_uint8_t *ptr_start, duk_uint8_t *ptr_end, duk_codepoint_t *out_cp);
duk_codepoint_t duk_unicode_decode_xutf8_checked(duk_hthread *thr, duk_uint8_t **ptr, duk_uint8_t *ptr_start, duk_uint8_t *ptr_end);
duk_size_t duk_unicode_unvalidated_utf8_length(duk_uint8_t *data, duk_size_t blen);
duk_small_int_t duk_unicode_is_whitespace(duk_codepoint_t cp);
duk_small_int_t duk_unicode_is_line_terminator(duk_codepoint_t cp);
duk_small_int_t duk_unicode_is_identifier_start(duk_codepoint_t cp);
duk_small_int_t duk_unicode_is_identifier_part(duk_codepoint_t cp);
void duk_unicode_case_convert_string(duk_hthread *thr, duk_small_int_t uppercase);
duk_codepoint_t duk_unicode_re_canonicalize_char(duk_hthread *thr, duk_codepoint_t cp);
duk_small_int_t duk_unicode_re_is_wordchar(duk_codepoint_t cp);

#endif  /* DUK_UNICODE_H_INCLUDED */

