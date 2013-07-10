/*
 *  Regular expression structs, constants, and bytecode defines.
 */

#ifndef DUK_REGEXP_H_INCLUDED
#define DUK_REGEXP_H_INCLUDED

#include "duk_bittypes.h"

/* maximum bytecode copies for {n,m} quantifiers */
#define  DUK_RE_MAX_ATOM_COPIES             1000

/* regexp compilation limits */
#define  DUK_RE_COMPILE_RECURSION_LIMIT     100

/* regexp execution limits */
#define  DUK_RE_EXECUTE_RECURSION_LIMIT     100
#define  DUK_RE_EXECUTE_STEPS_LIMIT         (1 * 1000 * 1000 * 1000)

/* regexp opcodes */
#define  DUK_REOP_MATCH                     1
#define  DUK_REOP_CHAR                      2
#define  DUK_REOP_PERIOD                    3
#define  DUK_REOP_RANGES                    4
#define  DUK_REOP_INVRANGES                 5
#define  DUK_REOP_JUMP                      6
#define  DUK_REOP_SPLIT1                    7
#define  DUK_REOP_SPLIT2                    8
#define  DUK_REOP_SQMINIMAL                 9
#define  DUK_REOP_SQGREEDY                  10
#define  DUK_REOP_SAVE                      11
#define  DUK_REOP_LOOKPOS                   12
#define  DUK_REOP_LOOKNEG                   13
#define  DUK_REOP_BACKREFERENCE             14
#define  DUK_REOP_ASSERT_START              15
#define  DUK_REOP_ASSERT_END                16
#define  DUK_REOP_ASSERT_WORD_BOUNDARY      17
#define  DUK_REOP_ASSERT_NOT_WORD_BOUNDARY  18

/* flags */
#define  DUK_RE_FLAG_GLOBAL                 (1 << 0)
#define  DUK_RE_FLAG_IGNORE_CASE            (1 << 1)
#define  DUK_RE_FLAG_MULTILINE              (1 << 2)

struct duk_re_matcher_ctx {
	duk_hthread *thr;

	duk_u32 re_flags;
	duk_u8 *input;
	duk_u8 *input_end;
	duk_u8 *bytecode;
	duk_u8 *bytecode_end;
	duk_u8 **saved;		/* allocated from valstack (fixed buffer) */
	duk_u32 nsaved;
	duk_u32 recursion_depth;
	duk_u32 steps_count;
	duk_u32 recursion_limit;
	duk_u32 steps_limit;
};

struct duk_re_compiler_ctx {
	duk_hthread *thr;

	duk_u32 re_flags;
	duk_lexer_ctx lex;
	duk_re_token curr_token;
	duk_hbuffer_dynamic *buf;
	duk_u32 captures;
	duk_u32 highest_backref;
	duk_u32 recursion_depth;
	duk_u32 recursion_limit;
	duk_u32 nranges;	/* internal temporary value, used for char classes */
};

/*
 *  Prototypes
 */

void duk_regexp_compile(duk_hthread *thr);
void duk_regexp_create_instance(duk_hthread *thr);
void duk_regexp_match(duk_hthread *thr);
void duk_regexp_match_force_global(duk_hthread *thr);  /* hacky helper for String.prototype.split() */

#endif  /* DUK_REGEXP_H_INCLUDED */

