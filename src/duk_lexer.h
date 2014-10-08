/*
 *  Lexer defines.
 */

#ifndef DUK_LEXER_H_INCLUDED
#define DUK_LEXER_H_INCLUDED

typedef void (*duk_re_range_callback)(void *user, duk_codepoint_t r1, duk_codepoint_t r2, duk_bool_t direct);

/*
 *  A token is interpreted as any possible production of InputElementDiv
 *  and InputElementRegExp, see E5 Section 7 in its entirety.  Note that
 *  the E5 "Token" production does not cover all actual tokens of the
 *  language (which is explicitly stated in the specification, Section 7.5).
 *  Null and boolean literals are defined as part of both ReservedWord
 *  (E5 Section 7.6.1) and Literal (E5 Section 7.8) productions.  Here,
 *  null and boolean values have literal tokens, and are not reserved
 *  words.
 *
 *  Decimal literal negative/positive sign is -not- part of DUK_TOK_NUMBER.
 *  The number tokens always have a non-negative value.  The unary minus
 *  operator in "-1.0" is optimized during compilation to yield a single
 *  negative constant.
 *
 *  Token numbering is free except that reserved words are required to be
 *  in a continuous range and in a particular order.  See genstrings.py.
 */

#define DUK_LEXER_INITCTX(ctx)        duk_lexer_initctx((ctx))

#define DUK_LEXER_SETPOINT(ctx,pt)    duk_lexer_setpoint((ctx), (pt))

#define DUK_LEXER_GETPOINT(ctx,pt)    do { (pt)->offset = (ctx)->offsets[0]; \
                                           (pt)->line = (ctx)->lines[0]; } while (0)

/* currently 6 characters of lookup are actually needed (duk_lexer.c) */
#define DUK_LEXER_WINDOW_SIZE                     8

#define DUK_TOK_MINVAL                            0

/* returned after EOF (infinite amount) */
#define DUK_TOK_EOF                               0

/* line terminator or multi-line comment with internal lineterm (E5 Sections 7.3, 7.4) */
#define DUK_TOK_LINETERM                          1

/* single-line comment or multi-line comment without internal lineterm (E5 Section 7.4) */
#define DUK_TOK_COMMENT                           2

/* identifier names (E5 Section 7.6) */
#define DUK_TOK_IDENTIFIER                        3

/* reserved words: keywords */
#define DUK_TOK_START_RESERVED                    4
#define DUK_TOK_BREAK                             4
#define DUK_TOK_CASE                              5
#define DUK_TOK_CATCH                             6
#define DUK_TOK_CONTINUE                          7
#define DUK_TOK_DEBUGGER                          8
#define DUK_TOK_DEFAULT                           9
#define DUK_TOK_DELETE                            10
#define DUK_TOK_DO                                11
#define DUK_TOK_ELSE                              12
#define DUK_TOK_FINALLY                           13
#define DUK_TOK_FOR                               14
#define DUK_TOK_FUNCTION                          15
#define DUK_TOK_IF                                16
#define DUK_TOK_IN                                17
#define DUK_TOK_INSTANCEOF                        18
#define DUK_TOK_NEW                               19
#define DUK_TOK_RETURN                            20
#define DUK_TOK_SWITCH                            21
#define DUK_TOK_THIS                              22
#define DUK_TOK_THROW                             23
#define DUK_TOK_TRY                               24
#define DUK_TOK_TYPEOF                            25
#define DUK_TOK_VAR                               26
#define DUK_TOK_VOID                              27
#define DUK_TOK_WHILE                             28
#define DUK_TOK_WITH                              29

/* reserved words: future reserved words */
#define DUK_TOK_CLASS                             30
#define DUK_TOK_CONST                             31
#define DUK_TOK_ENUM                              32
#define DUK_TOK_EXPORT                            33
#define DUK_TOK_EXTENDS                           34
#define DUK_TOK_IMPORT                            35
#define DUK_TOK_SUPER                             36

/* "null", "true", and "false" are always reserved words.
 * Note that "get" and "set" are not!
 */
#define DUK_TOK_NULL                              37
#define DUK_TOK_TRUE                              38
#define DUK_TOK_FALSE                             39

/* reserved words: additional future reserved words in strict mode */
#define DUK_TOK_START_STRICT_RESERVED             40  /* inclusive */
#define DUK_TOK_IMPLEMENTS                        40
#define DUK_TOK_INTERFACE                         41
#define DUK_TOK_LET                               42
#define DUK_TOK_PACKAGE                           43
#define DUK_TOK_PRIVATE                           44
#define DUK_TOK_PROTECTED                         45
#define DUK_TOK_PUBLIC                            46
#define DUK_TOK_STATIC                            47
#define DUK_TOK_YIELD                             48

#define DUK_TOK_END_RESERVED                      49  /* exclusive */

/* "get" and "set" are tokens but NOT ReservedWords.  They are currently
 * parsed and identifiers and these defines are actually now unused.
 */
#define DUK_TOK_GET                               49
#define DUK_TOK_SET                               50

/* punctuators (unlike the spec, also includes "/" and "/=") */
#define DUK_TOK_LCURLY                            51
#define DUK_TOK_RCURLY                            52
#define DUK_TOK_LBRACKET                          53
#define DUK_TOK_RBRACKET                          54
#define DUK_TOK_LPAREN                            55
#define DUK_TOK_RPAREN                            56
#define DUK_TOK_PERIOD                            57
#define DUK_TOK_SEMICOLON                         58
#define DUK_TOK_COMMA                             59
#define DUK_TOK_LT                                60
#define DUK_TOK_GT                                61
#define DUK_TOK_LE                                62
#define DUK_TOK_GE                                63
#define DUK_TOK_EQ                                64
#define DUK_TOK_NEQ                               65
#define DUK_TOK_SEQ                               66
#define DUK_TOK_SNEQ                              67
#define DUK_TOK_ADD                               68
#define DUK_TOK_SUB                               69
#define DUK_TOK_MUL                               70
#define DUK_TOK_DIV                               71
#define DUK_TOK_MOD                               72
#define DUK_TOK_INCREMENT                         73
#define DUK_TOK_DECREMENT                         74
#define DUK_TOK_ALSHIFT                           75  /* named "arithmetic" because result is signed */
#define DUK_TOK_ARSHIFT                           76
#define DUK_TOK_RSHIFT                            77
#define DUK_TOK_BAND                              78
#define DUK_TOK_BOR                               79
#define DUK_TOK_BXOR                              80
#define DUK_TOK_LNOT                              81
#define DUK_TOK_BNOT                              82
#define DUK_TOK_LAND                              83
#define DUK_TOK_LOR                               84
#define DUK_TOK_QUESTION                          85
#define DUK_TOK_COLON                             86
#define DUK_TOK_EQUALSIGN                         87
#define DUK_TOK_ADD_EQ                            88
#define DUK_TOK_SUB_EQ                            89
#define DUK_TOK_MUL_EQ                            90
#define DUK_TOK_DIV_EQ                            91
#define DUK_TOK_MOD_EQ                            92
#define DUK_TOK_ALSHIFT_EQ                        93
#define DUK_TOK_ARSHIFT_EQ                        94
#define DUK_TOK_RSHIFT_EQ                         95
#define DUK_TOK_BAND_EQ                           96
#define DUK_TOK_BOR_EQ                            97
#define DUK_TOK_BXOR_EQ                           98

/* literals (E5 Section 7.8), except null, true, false, which are treated
 * like reserved words (above).
 */
#define DUK_TOK_NUMBER                            99
#define DUK_TOK_STRING                            100
#define DUK_TOK_REGEXP                            101

#define DUK_TOK_MAXVAL                            101  /* inclusive */

/* Convert heap string index to a token (reserved words) */
#define DUK_STRIDX_TO_TOK(x)                        ((x) - DUK_STRIDX_START_RESERVED + DUK_TOK_START_RESERVED)

/* Sanity check */
#if (DUK_TOK_MAXVAL > 255)
#error DUK_TOK_MAXVAL too large, code assumes it fits into 8 bits
#endif

/* Sanity checks for string and token defines */
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_BREAK) != DUK_TOK_BREAK)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_CASE) != DUK_TOK_CASE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_CATCH) != DUK_TOK_CATCH)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_CONTINUE) != DUK_TOK_CONTINUE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_DEBUGGER) != DUK_TOK_DEBUGGER)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_DEFAULT) != DUK_TOK_DEFAULT)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_DELETE) != DUK_TOK_DELETE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_DO) != DUK_TOK_DO)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_ELSE) != DUK_TOK_ELSE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_FINALLY) != DUK_TOK_FINALLY)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_FOR) != DUK_TOK_FOR)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_LC_FUNCTION) != DUK_TOK_FUNCTION)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_IF) != DUK_TOK_IF)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_IN) != DUK_TOK_IN)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_INSTANCEOF) != DUK_TOK_INSTANCEOF)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_NEW) != DUK_TOK_NEW)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_RETURN) != DUK_TOK_RETURN)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_SWITCH) != DUK_TOK_SWITCH)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_THIS) != DUK_TOK_THIS)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_THROW) != DUK_TOK_THROW)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_TRY) != DUK_TOK_TRY)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_TYPEOF) != DUK_TOK_TYPEOF)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_VAR) != DUK_TOK_VAR)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_VOID) != DUK_TOK_VOID)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_WHILE) != DUK_TOK_WHILE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_WITH) != DUK_TOK_WITH)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_CLASS) != DUK_TOK_CLASS)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_CONST) != DUK_TOK_CONST)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_ENUM) != DUK_TOK_ENUM)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_EXPORT) != DUK_TOK_EXPORT)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_EXTENDS) != DUK_TOK_EXTENDS)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_IMPORT) != DUK_TOK_IMPORT)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_SUPER) != DUK_TOK_SUPER)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_LC_NULL) != DUK_TOK_NULL)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_TRUE) != DUK_TOK_TRUE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_FALSE) != DUK_TOK_FALSE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_IMPLEMENTS) != DUK_TOK_IMPLEMENTS)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_INTERFACE) != DUK_TOK_INTERFACE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_LET) != DUK_TOK_LET)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_PACKAGE) != DUK_TOK_PACKAGE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_PRIVATE) != DUK_TOK_PRIVATE)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_PROTECTED) != DUK_TOK_PROTECTED)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_PUBLIC) != DUK_TOK_PUBLIC)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_STATIC) != DUK_TOK_STATIC)
#error mismatch in token defines
#endif
#if (DUK_STRIDX_TO_TOK(DUK_STRIDX_YIELD) != DUK_TOK_YIELD)
#error mismatch in token defines
#endif

/* Regexp tokens */
#define DUK_RETOK_EOF                              0
#define DUK_RETOK_DISJUNCTION                      1
#define DUK_RETOK_QUANTIFIER                       2
#define DUK_RETOK_ASSERT_START                     3
#define DUK_RETOK_ASSERT_END                       4
#define DUK_RETOK_ASSERT_WORD_BOUNDARY             5
#define DUK_RETOK_ASSERT_NOT_WORD_BOUNDARY         6
#define DUK_RETOK_ASSERT_START_POS_LOOKAHEAD       7
#define DUK_RETOK_ASSERT_START_NEG_LOOKAHEAD       8
#define DUK_RETOK_ATOM_PERIOD                      9
#define DUK_RETOK_ATOM_CHAR                        10
#define DUK_RETOK_ATOM_DIGIT                       11
#define DUK_RETOK_ATOM_NOT_DIGIT                   12
#define DUK_RETOK_ATOM_WHITE                       13
#define DUK_RETOK_ATOM_NOT_WHITE                   14
#define DUK_RETOK_ATOM_WORD_CHAR                   15
#define DUK_RETOK_ATOM_NOT_WORD_CHAR               16
#define DUK_RETOK_ATOM_BACKREFERENCE               17
#define DUK_RETOK_ATOM_START_CAPTURE_GROUP         18
#define DUK_RETOK_ATOM_START_NONCAPTURE_GROUP      19
#define DUK_RETOK_ATOM_START_CHARCLASS             20
#define DUK_RETOK_ATOM_START_CHARCLASS_INVERTED    21
#define DUK_RETOK_ATOM_END_GROUP                   22

/* constants for duk_lexer_ctx.buf */
#define DUK_LEXER_TEMP_BUF_INITIAL                 64
#define DUK_LEXER_TEMP_BUF_LIMIT                   256

/* A token value.  Can be memcpy()'d, but note that slot1/slot2 values are on the valstack. */
struct duk_token {
	duk_small_int_t t;            /* token type (with reserved word identification) */
	duk_small_int_t t_nores;      /* token type (with reserved words as DUK_TOK_IDENTIFER) */
	duk_double_t num;             /* numeric value of token */
	duk_hstring *str1;            /* string 1 of token (borrowed, stored to ctx->slot1_idx) */
	duk_hstring *str2;            /* string 2 of token (borrowed, stored to ctx->slot1_idx) */
	duk_size_t start_offset;      /* start byte offset of token in lexer input */
	duk_int_t start_line;         /* start line of token (first char) */
	duk_int_t num_escapes;        /* number of escapes and line continuations (for directive prologue) */
	duk_bool_t lineterm;          /* token was preceded by a lineterm */
	duk_bool_t allow_auto_semi;   /* token allows automatic semicolon insertion (eof or preceded by newline) */
};

#define DUK_RE_QUANTIFIER_INFINITE         ((duk_uint32_t) 0xffffffffUL)

/* A regexp token value. */
struct duk_re_token {
	duk_small_int_t t;           /* token type */
	duk_small_int_t greedy;
	duk_uint_fast32_t num;       /* numeric value (character, count) */
	duk_uint_fast32_t qmin;
	duk_uint_fast32_t qmax;
};

/* A structure for 'snapshotting' a point for rewinding */
struct duk_lexer_point {
	duk_size_t offset;
	duk_int_t line;
};

/* Lexer context.  Same context is used for Ecmascript and Regexp parsing. */
struct duk_lexer_ctx {
	duk_hthread *thr;                              /* thread; minimizes argument passing */

	const duk_uint8_t *input;                      /* input string (may be a user pointer) */
	duk_size_t input_length;                       /* input byte length */
	duk_size_t input_offset;                       /* input offset for window leading edge (not window[0]) */

	duk_codepoint_t window[DUK_LEXER_WINDOW_SIZE]; /* window of unicode code points */
	duk_size_t offsets[DUK_LEXER_WINDOW_SIZE];     /* input byte offset for each char */
	duk_int_t lines[DUK_LEXER_WINDOW_SIZE];        /* input lines for each char */
	duk_int_t input_line;                          /* input linenumber at input_offset (not window[0]), init to 1 */
	duk_idx_t slot1_idx;                           /* valstack slot for 1st token value */
	duk_idx_t slot2_idx;                           /* valstack slot for 2nd token value */
	duk_idx_t buf_idx;                             /* valstack slot for temp buffer */
	duk_hbuffer_dynamic *buf;                      /* temp accumulation buffer (on valstack) */

	duk_int_t token_count;                         /* number of tokens parsed */
	duk_int_t token_limit;                         /* maximum token count before error (sanity backstop) */
};

/*
 *  Prototypes
 */

DUK_INTERNAL_DECL void duk_lexer_initctx(duk_lexer_ctx *lex_ctx);

DUK_INTERNAL_DECL void duk_lexer_setpoint(duk_lexer_ctx *lex_ctx, duk_lexer_point *pt);

DUK_INTERNAL_DECL
void duk_lexer_parse_js_input_element(duk_lexer_ctx *lex_ctx,
                                      duk_token *out_token,
                                      duk_bool_t strict_mode,
                                      duk_bool_t regexp_mode);
#ifdef DUK_USE_REGEXP_SUPPORT
DUK_INTERNAL_DECL void duk_lexer_parse_re_token(duk_lexer_ctx *lex_ctx, duk_re_token *out_token);
DUK_INTERNAL_DECL void duk_lexer_parse_re_ranges(duk_lexer_ctx *lex_ctx, duk_re_range_callback gen_range, void *userdata);
#endif  /* DUK_USE_REGEXP_SUPPORT */

#endif  /* DUK_LEXER_H_INCLUDED */
