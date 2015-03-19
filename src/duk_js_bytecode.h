/*
 *  Ecmascript bytecode
 */

#ifndef DUK_JS_BYTECODE_H_INCLUDED
#define DUK_JS_BYTECODE_H_INCLUDED

/*
 *  Logical instruction layout
 *  ==========================
 *
 *  !3!3!2!2!2!2!2!2!2!2!2!2!1!1!1!1!1!1!1!1!1!1! ! ! ! ! ! ! ! ! ! !
 *  !1!0!9!8!7!6!5!4!3!2!1!0!9!8!7!6!5!4!3!2!1!0!9!8!7!6!5!4!3!2!1!0!
 *  +---------------------------------------------------+-----------+
 *  !       C         !       B         !      A        !    OP     !
 *  +---------------------------------------------------+-----------+
 *
 *  OP (6 bits):  opcode (DUK_OP_*), access should be fastest
 *  A (8 bits):   typically a target register number
 *  B (9 bits):   typically first source register/constant number
 *  C (9 bits):   typically second source register/constant number
 *
 *  Some instructions combine BC or ABC together for larger parameter values.
 *  Signed integers (e.g. jump offsets) are encoded as unsigned, with an opcode
 *  specific bias.  B and C may denote a register or a constant, see
 *  DUK_BC_ISREG() and DUK_BC_ISCONST().
 *
 *  Note: macro naming is a bit misleading, e.g. "ABC" in macro name but
 *  the field layout is logically "CBA".
 */

typedef duk_uint32_t duk_instr_t;

#define DUK_DEC_OP(x)               ((x) & 0x3fUL)
#define DUK_DEC_A(x)                (((x) >> 6) & 0xffUL)
#define DUK_DEC_B(x)                (((x) >> 14) & 0x1ffUL)
#define DUK_DEC_C(x)                (((x) >> 23) & 0x1ffUL)
#define DUK_DEC_BC(x)               (((x) >> 14) & 0x3ffffUL)
#define DUK_DEC_ABC(x)              (((x) >> 6) & 0x3ffffffUL)

#define DUK_ENC_OP(op)              ((duk_instr_t) (op))
#define DUK_ENC_OP_ABC(op,abc)      ((duk_instr_t) ( \
                                        (((duk_instr_t) (abc)) << 6) | \
                                        ((duk_instr_t) (op)) \
                                    ))
#define DUK_ENC_OP_A_BC(op,a,bc)    ((duk_instr_t) ( \
                                        (((duk_instr_t) (bc)) << 14) | \
                                        (((duk_instr_t) (a)) << 6) | \
                                        ((duk_instr_t) (op)) \
                                    ))
#define DUK_ENC_OP_A_B_C(op,a,b,c)  ((duk_instr_t) ( \
                                        (((duk_instr_t) (c)) << 23) | \
                                        (((duk_instr_t) (b)) << 14) | \
                                        (((duk_instr_t) (a)) << 6) | \
                                        ((duk_instr_t) (op)) \
                                    ))
#define DUK_ENC_OP_A_B(op,a,b)      DUK_ENC_OP_A_B_C(op,a,b,0)
#define DUK_ENC_OP_A(op,a)          DUK_ENC_OP_A_B_C(op,a,0,0)

/* Constants should be signed so that signed arithmetic involving them
 * won't cause values to be coerced accidentally to unsigned.
 */
#define DUK_BC_OP_MIN               0
#define DUK_BC_OP_MAX               0x3fL
#define DUK_BC_A_MIN                0
#define DUK_BC_A_MAX                0xffL
#define DUK_BC_B_MIN                0
#define DUK_BC_B_MAX                0x1ffL
#define DUK_BC_C_MIN                0
#define DUK_BC_C_MAX                0x1ffL
#define DUK_BC_BC_MIN               0
#define DUK_BC_BC_MAX               0x3ffffL
#define DUK_BC_ABC_MIN              0
#define DUK_BC_ABC_MAX              0x3ffffffL
#define DUK_BC_EXTRAOP_MIN          DUK_BC_A_MIN
#define DUK_BC_EXTRAOP_MAX          DUK_BC_A_MAX

#define DUK_OP_LDREG                0
#define DUK_OP_STREG                1
#define DUK_OP_LDCONST              2
#define DUK_OP_LDINT                3
#define DUK_OP_LDINTX               4
#define DUK_OP_MPUTOBJ              5
#define DUK_OP_MPUTOBJI             6
#define DUK_OP_MPUTARR              7
#define DUK_OP_MPUTARRI             8
#define DUK_OP_NEW                  9
#define DUK_OP_NEWI                 10
#define DUK_OP_REGEXP               11
#define DUK_OP_CSREG                12
#define DUK_OP_CSREGI               13
#define DUK_OP_GETVAR               14
#define DUK_OP_PUTVAR               15
#define DUK_OP_DECLVAR              16
#define DUK_OP_DELVAR               17
#define DUK_OP_CSVAR                18
#define DUK_OP_CSVARI               19
#define DUK_OP_CLOSURE              20
#define DUK_OP_GETPROP              21
#define DUK_OP_PUTPROP              22
#define DUK_OP_DELPROP              23
#define DUK_OP_CSPROP               24
#define DUK_OP_CSPROPI              25
#define DUK_OP_ADD                  26
#define DUK_OP_SUB                  27
#define DUK_OP_MUL                  28
#define DUK_OP_DIV                  29
#define DUK_OP_MOD                  30
#define DUK_OP_BAND                 31
#define DUK_OP_BOR                  32
#define DUK_OP_BXOR                 33
#define DUK_OP_BASL                 34
#define DUK_OP_BLSR                 35
#define DUK_OP_BASR                 36
#define DUK_OP_EQ                   37
#define DUK_OP_NEQ                  38
#define DUK_OP_SEQ                  39
#define DUK_OP_SNEQ                 40
#define DUK_OP_GT                   41
#define DUK_OP_GE                   42
#define DUK_OP_LT                   43
#define DUK_OP_LE                   44
#define DUK_OP_IF                   45
#define DUK_OP_JUMP                 46
#define DUK_OP_RETURN               47
#define DUK_OP_CALL                 48
#define DUK_OP_CALLI                49
#define DUK_OP_TRYCATCH             50
#define DUK_OP_EXTRA                51
#define DUK_OP_PREINCR              52  /* pre/post opcode values have constraints, */
#define DUK_OP_PREDECR              53  /* see duk_js_executor.c */
#define DUK_OP_POSTINCR             54
#define DUK_OP_POSTDECR             55
#define DUK_OP_PREINCV              56
#define DUK_OP_PREDECV              57
#define DUK_OP_POSTINCV             58
#define DUK_OP_POSTDECV             59
#define DUK_OP_PREINCP              60
#define DUK_OP_PREDECP              61
#define DUK_OP_POSTINCP             62
#define DUK_OP_POSTDECP             63
#define DUK_OP_NONE                 64  /* dummy value used as marker */

/* DUK_OP_EXTRA, sub-operation in A */
#define DUK_EXTRAOP_NOP             0
#define DUK_EXTRAOP_INVALID         1
#define DUK_EXTRAOP_LDTHIS          2
#define DUK_EXTRAOP_LDUNDEF         3
#define DUK_EXTRAOP_LDNULL          4
#define DUK_EXTRAOP_LDTRUE          5
#define DUK_EXTRAOP_LDFALSE         6
#define DUK_EXTRAOP_NEWOBJ          7
#define DUK_EXTRAOP_NEWARR          8
#define DUK_EXTRAOP_SETALEN         9
#define DUK_EXTRAOP_TYPEOF          10
#define DUK_EXTRAOP_TYPEOFID        11
#define DUK_EXTRAOP_INITENUM        12
#define DUK_EXTRAOP_NEXTENUM        13
#define DUK_EXTRAOP_INITSET         14
#define DUK_EXTRAOP_INITSETI        15
#define DUK_EXTRAOP_INITGET         16
#define DUK_EXTRAOP_INITGETI        17
#define DUK_EXTRAOP_ENDTRY          18
#define DUK_EXTRAOP_ENDCATCH        19
#define DUK_EXTRAOP_ENDFIN          20
#define DUK_EXTRAOP_THROW           21
#define DUK_EXTRAOP_INVLHS          22
#define DUK_EXTRAOP_UNM             23
#define DUK_EXTRAOP_UNP             24
#define DUK_EXTRAOP_DEBUGGER        25
#define DUK_EXTRAOP_BREAK           26
#define DUK_EXTRAOP_CONTINUE        27
#define DUK_EXTRAOP_BNOT            28
#define DUK_EXTRAOP_LNOT            29
#define DUK_EXTRAOP_INSTOF          30
#define DUK_EXTRAOP_IN              31
#define DUK_EXTRAOP_LABEL           32
#define DUK_EXTRAOP_ENDLABEL        33

/* DUK_OP_EXTRA for debugging */
#define DUK_EXTRAOP_DUMPREG         128
#define DUK_EXTRAOP_DUMPREGS        129
#define DUK_EXTRAOP_LOGMARK         130

/* DUK_OP_CALL flags in A */
#define DUK_BC_CALL_FLAG_TAILCALL           (1 << 0)
#define DUK_BC_CALL_FLAG_EVALCALL           (1 << 1)

/* DUK_OP_TRYCATCH flags in A */
#define DUK_BC_TRYCATCH_FLAG_HAVE_CATCH     (1 << 0)
#define DUK_BC_TRYCATCH_FLAG_HAVE_FINALLY   (1 << 1)
#define DUK_BC_TRYCATCH_FLAG_CATCH_BINDING  (1 << 2)
#define DUK_BC_TRYCATCH_FLAG_WITH_BINDING   (1 << 3)

/* DUK_OP_RETURN flags in A */
#define DUK_BC_RETURN_FLAG_FAST             (1 << 0)
#define DUK_BC_RETURN_FLAG_HAVE_RETVAL      (1 << 1)

/* DUK_OP_DECLVAR flags in A; bottom bits are reserved for propdesc flags (DUK_PROPDESC_FLAG_XXX) */
#define DUK_BC_DECLVAR_FLAG_UNDEF_VALUE     (1 << 4)  /* use 'undefined' for value automatically */
#define DUK_BC_DECLVAR_FLAG_FUNC_DECL       (1 << 5)  /* function declaration */

/* misc constants and helper macros */
#define DUK_BC_REGLIMIT             256  /* if B/C is >= this value, refers to a const */
#define DUK_BC_ISREG(x)             ((x) < DUK_BC_REGLIMIT)
#define DUK_BC_ISCONST(x)           ((x) >= DUK_BC_REGLIMIT)
#define DUK_BC_LDINT_BIAS           (1L << 17)
#define DUK_BC_LDINTX_SHIFT         18
#define DUK_BC_JUMP_BIAS            (1L << 25)

#endif  /* DUK_JS_BYTECODE_H_INCLUDED */
