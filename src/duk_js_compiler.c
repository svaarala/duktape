/*
 *  Ecmascript compiler.
 *
 *  Parses an input string and generates a function template result.
 *  Compilation may happen in multiple contexts (global code, eval
 *  code, function code).
 *
 *  The parser uses a traditional top-down recursive parsing for the
 *  statement level, and an operator precedence based top-down approach
 *  for the expression level.  The attempt is to minimize the C stack
 *  depth.  Bytecode is generated directly without an intermediate
 *  representation (tree), at the cost of needing two passes over each
 *  function.
 *
 *  The top-down recursive parser functions are named "parse_XXX".
 *
 *  Recursion limits are in key functions to prevent arbitrary C recursion:
 *  function body parsing, statement parsing, and expression parsing.
 *
 *  See doc/compiler.txt for discussion on the design.
 */

#include "duk_internal.h"

/* if highest bit of a register number is set, it refers to a constant instead */
#define  CONST_MARKER                 DUK_JS_CONST_MARKER

/* for array and object literals */
#define  MAX_ARRAY_INIT_VALUES        20
#define  MAX_OBJECT_INIT_PAIRS        10

/* FIXME: hack, remove when const lookup is not O(n) */
#define  GETCONST_MAX_CONSTS_CHECK    256

#define  RECURSION_INCREASE(comp_ctx,thr)  do { \
		DUK_DDDPRINT("RECURSION INCREASE: %s:%d", __FILE__, __LINE__); \
		recursion_increase((comp_ctx)); \
	} while(0)

#define  RECURSION_DECREASE(comp_ctx,thr)  do { \
		DUK_DDDPRINT("RECURSION DECREASE: %s:%d", __FILE__, __LINE__); \
		recursion_decrease((comp_ctx)); \
	} while(0)

/* Note: slots limits below are quite approximate right now, and because they
 * overlap (in control flow), some can be eliminated.
 */

#define  COMPILE_ENTRY_SLOTS          8
#define  FUNCTION_INIT_REQUIRE_SLOTS  16
#define  FUNCTION_BODY_REQUIRE_SLOTS  16
#define  PARSE_STATEMENTS_SLOTS       16

/*
 *  Prototypes
 */

/* lexing */
static void advance_helper(duk_compiler_ctx *comp_ctx, int expect);
static void advance_expect(duk_compiler_ctx *comp_ctx, int expect);
static void advance(duk_compiler_ctx *ctx);

/* function helpers */
static void init_function_valstack_slots(duk_compiler_ctx *comp_ctx);
static void reset_function_for_pass2(duk_compiler_ctx *comp_ctx);
static void init_varmap_and_prologue_for_pass2(duk_compiler_ctx *comp_ctx, int *out_stmt_value_reg);
static void convert_to_function_template(duk_compiler_ctx *comp_ctx);

/* code emission */
static int get_current_pc(duk_compiler_ctx *comp_ctx);
static duk_compiler_instr *get_instr_ptr(duk_compiler_ctx *comp_ctx, int pc);
static void emit(duk_compiler_ctx *comp_ctx, duk_instr ins);
#if 0  /* unused */
static void emit_op_only(duk_compiler_ctx *comp_ctx, int op);
#endif
static void emit_a_b_c(duk_compiler_ctx *comp_ctx, int op, int a, int b, int c);
static void emit_a_b(duk_compiler_ctx *comp_ctx, int op, int a, int b);
#if 0  /* unused */
static void emit_a(duk_compiler_ctx *comp_ctx, int op, int a);
#endif
static void emit_a_bc(duk_compiler_ctx *comp_ctx, int op, int a, int bc);
static void emit_abc(duk_compiler_ctx *comp_ctx, int op, int abc);
static void emit_extraop_b_c(duk_compiler_ctx *comp_ctx, int extraop, int b, int c);
static void emit_extraop_b(duk_compiler_ctx *comp_ctx, int extraop, int b);
static void emit_extraop_only(duk_compiler_ctx *comp_ctx, int extraop);
static void emit_loadint(duk_compiler_ctx *comp_ctx, int reg, int val);
static void emit_jump(duk_compiler_ctx *comp_ctx, int target_pc);
static int emit_jump_empty(duk_compiler_ctx *comp_ctx);
static void insert_jump_empty(duk_compiler_ctx *comp_ctx, int jump_pc);
static void patch_jump(duk_compiler_ctx *comp_ctx, int jump_pc, int target_pc);
static void patch_jump_here(duk_compiler_ctx *comp_ctx, int jump_pc);
static void patch_trycatch(duk_compiler_ctx *comp_ctx, int trycatch_pc, int reg_catch, int const_varname, int flags);
static void emit_if_false_skip(duk_compiler_ctx *comp_ctx, int regconst);
static void emit_if_true_skip(duk_compiler_ctx *comp_ctx, int regconst);
static void emit_invalid(duk_compiler_ctx *comp_ctx);

/* FIXME */
static void copy_ivalue(duk_compiler_ctx *comp_ctx, duk_ivalue *src, duk_ivalue *dst);
static void copy_ispec(duk_compiler_ctx *comp_ctx, duk_ispec *src, duk_ispec *dst);
static int is_whole_get_i32(double x, duk_i32 *ival);

/* ivalue/ispec helpers */
static int alloctemps(duk_compiler_ctx *comp_ctx, int num);
static int alloctemp(duk_compiler_ctx *comp_ctx);
static void settemp_checkmax(duk_compiler_ctx *comp_ctx, int temp_next);
static int getconst(duk_compiler_ctx *comp_ctx);
static int ispec_toregconst_raw(duk_compiler_ctx *comp_ctx,
                                duk_ispec *x,
                                int forced_reg,
                                int allow_const,
                                int require_temp);
static int ispec_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ispec *x, int forced_reg);
static void ivalue_toplain_raw(duk_compiler_ctx *comp_ctx, duk_ivalue *x, int forced_reg);
static void ivalue_toplain(duk_compiler_ctx *comp_ctx, duk_ivalue *x);
static void ivalue_toplain_ignore(duk_compiler_ctx *comp_ctx, duk_ivalue *x);
static int ivalue_toregconst_raw(duk_compiler_ctx *comp_ctx,
                                 duk_ivalue *x,
                                 int forced_reg,
                                 int allow_const,
                                 int require_temp);
static int ivalue_toreg(duk_compiler_ctx *comp_ctx, duk_ivalue *x);
#if 0  /* unused */
static int ivalue_totempreg(duk_compiler_ctx *comp_ctx, duk_ivalue *x);
#endif
static int ivalue_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ivalue *x, int forced_reg);
static int ivalue_toregconst(duk_compiler_ctx *comp_ctx, duk_ivalue *x);

/* identifier handling */
static int lookup_active_register_binding(duk_compiler_ctx *comp_ctx);
static int lookup_lhs(duk_compiler_ctx *ctx, int *out_reg_varbind, int *out_reg_varname);

/* label handling */
static void add_label(duk_compiler_ctx *comp_ctx, duk_hstring *h_label, int pc_label, int label_id);
static void update_label_flags(duk_compiler_ctx *comp_ctx, int label_id, int flags);
static void lookup_active_label(duk_compiler_ctx *comp_ctx, duk_hstring *h_label, int is_break, int *out_label_id, int *out_label_catch_depth, int *out_label_pc, int *out_is_closest);
static void reset_labels_to_length(duk_compiler_ctx *comp_ctx, int len);

/* top-down expression parser */
static void expr_nud(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void expr_led(duk_compiler_ctx *comp_ctx, duk_ivalue *left, duk_ivalue *res);
static int expr_lbp(duk_compiler_ctx *comp_ctx);
static int expr_is_empty(duk_compiler_ctx *comp_ctx);

/* exprtop is the top level variant which resets nud/led counts */
static void expr(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp);
static void exprtop(duk_compiler_ctx *ctx, duk_ivalue *res, int rbp_flags);

/* convenience helpers */
static int expr_toreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp);
#if 0  /* unused */
static int expr_totempreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp);
#endif
static int expr_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp, int forced_reg);
static int expr_toregconst(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp);
static void expr_toplain(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp);
static void expr_toplain_ignore(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp);
static int exprtop_toreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags);
#if 0  /* unused */
static int exprtop_totempreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags);
static int exprtop_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags, int forced_reg);
#endif
static int exprtop_toregconst(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags);
#if 0  /* unused */
static void exprtop_toplain_ignore(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags);
#endif

/* expression parsing helpers */
static int parse_arguments(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void nud_array_literal(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void nud_object_literal(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static int nud_object_literal_key_check(duk_compiler_ctx *comp_ctx, int new_key_flags);

/* statement parsing */
static void parse_variable_declaration(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int expr_flags, int *out_reg_varname, int *out_reg_varbind);
static void parse_var_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void parse_for_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site);
static void parse_switch_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site);
static void parse_if_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void parse_do_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site);
static void parse_while_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site);
static void parse_break_or_continue_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void parse_return_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void parse_throw_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void parse_try_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void parse_with_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res);
static void parse_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int allow_source_elem);
static void parse_statements(duk_compiler_ctx *comp_ctx, int allow_source_elem, int expect_eof);

static void parse_function_body(duk_compiler_ctx *comp_ctx, int expect_eof, int implicit_return_value);
static void parse_function_formals(duk_compiler_ctx *comp_ctx);
static void parse_function_like_raw(duk_compiler_ctx *comp_ctx, int is_decl, int is_setget);
static int parse_function_like_fnum(duk_compiler_ctx *comp_ctx, int is_decl, int is_setget);

/*
 *  Parser control values for tokens.  The token table is ordered by the
 *  DUK_TOK_XXX defines.
 *
 *  The binding powers are for lbp() use (i.e. for use in led() context).
 *  Binding powers are positive for typing convenience, and bits at the
 *  top should be reserved for flags.  Binding power step must be higher
 *  than 1 so that binding power "lbp - 1" can be used for right associative
 *  operators.  Currently a step of 2 is used (which frees one more bit for
 *  flags).
 */

/* FIXME: actually single step levels would work just fine, clean up */

/* binding power "levels" (see doc/compiler.txt) */
#define  BP_INVALID                0             /* always terminates led() */
#define  BP_EOF                    2
#define  BP_CLOSING                4             /* token closes expression, e.g. ')', ']' */
#define  BP_FOR_EXPR               BP_CLOSING    /* bp to use when parsing a top level Expression */
#define  BP_COMMA                  6
#define  BP_ASSIGNMENT             8
#define  BP_CONDITIONAL            10
#define  BP_LOR                    12
#define  BP_LAND                   14
#define  BP_BOR                    16
#define  BP_BXOR                   18
#define  BP_BAND                   20
#define  BP_EQUALITY               22
#define  BP_RELATIONAL             24
#define  BP_SHIFT                  26
#define  BP_ADDITIVE               28
#define  BP_MULTIPLICATIVE         30
#define  BP_POSTFIX                32
#define  BP_CALL                   34
#define  BP_MEMBER                 36

#define  TOKEN_LBP_BP_MASK         0x1f
#define  TOKEN_LBP_FLAG_NO_REGEXP  (1 << 5)   /* regexp literal must not follow this token */
#define  TOKEN_LBP_FLAG_TERMINATES (1 << 6)   /* FIXME: terminates expression; e.g. post-increment/-decrement */
#define  TOKEN_LBP_FLAG_UNUSED     (1 << 7)   /* spare */

#define  TOKEN_LBP_GET_BP(x)       ((int) (((x) & TOKEN_LBP_BP_MASK) * 2))

#define  MK_LBP(bp)                ((bp) >> 1)    /* bp is assumed to be even */
#define  MK_LBP_FLAGS(bp,flags)    (((bp) >> 1) | (flags))

static duk_i8 token_lbp[] = {
	MK_LBP(BP_EOF),                                 /* DUK_TOK_EOF */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_LINETERM */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_COMMENT */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_IDENTIFIER */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_BREAK */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_CASE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_CATCH */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_CONTINUE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_DEBUGGER */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_DEFAULT */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_DELETE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_DO */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_ELSE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_FINALLY */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_FOR */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_FUNCTION */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_IF */
	MK_LBP(BP_RELATIONAL),                          /* DUK_TOK_IN */
	MK_LBP(BP_RELATIONAL),                          /* DUK_TOK_INSTANCEOF */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_NEW */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_RETURN */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_SWITCH */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_THIS */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_THROW */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_TRY */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_TYPEOF */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_VAR */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_VOID */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_WHILE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_WITH */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_CLASS */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_CONST */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_ENUM */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_EXPORT */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_EXTENDS */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_IMPORT */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_SUPER */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_NULL */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_TRUE */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_FALSE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_GET */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_SET */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_IMPLEMENTS */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_INTERFACE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_LET */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_PACKAGE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_PRIVATE */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_PROTECTED */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_PUBLIC */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_STATIC */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_YIELD */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_LCURLY */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_RCURLY */
	MK_LBP(BP_MEMBER),                              /* DUK_TOK_LBRACKET */
	MK_LBP_FLAGS(BP_CLOSING, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_RBRACKET */
	MK_LBP(BP_CALL),                                /* DUK_TOK_LPAREN */
	MK_LBP_FLAGS(BP_CLOSING, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_RPAREN */
	MK_LBP(BP_MEMBER),                              /* DUK_TOK_PERIOD */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_SEMICOLON */
	MK_LBP(BP_COMMA),                               /* DUK_TOK_COMMA */
	MK_LBP(BP_RELATIONAL),                          /* DUK_TOK_LT */
	MK_LBP(BP_RELATIONAL),                          /* DUK_TOK_GT */
	MK_LBP(BP_RELATIONAL),                          /* DUK_TOK_LE */
	MK_LBP(BP_RELATIONAL),                          /* DUK_TOK_GE */
	MK_LBP(BP_EQUALITY),                            /* DUK_TOK_EQ */
	MK_LBP(BP_EQUALITY),                            /* DUK_TOK_NEQ */
	MK_LBP(BP_EQUALITY),                            /* DUK_TOK_SEQ */
	MK_LBP(BP_EQUALITY),                            /* DUK_TOK_SNEQ */
	MK_LBP(BP_ADDITIVE),                            /* DUK_TOK_ADD */
	MK_LBP(BP_ADDITIVE),                            /* DUK_TOK_SUB */
	MK_LBP(BP_MULTIPLICATIVE),                      /* DUK_TOK_MUL */
	MK_LBP(BP_MULTIPLICATIVE),                      /* DUK_TOK_DIV */
	MK_LBP(BP_MULTIPLICATIVE),                      /* DUK_TOK_MOD */
	MK_LBP(BP_POSTFIX),                             /* DUK_TOK_INCREMENT */
	MK_LBP(BP_POSTFIX),                             /* DUK_TOK_DECREMENT */
	MK_LBP(BP_SHIFT),                               /* DUK_TOK_ALSHIFT */
	MK_LBP(BP_SHIFT),                               /* DUK_TOK_ARSHIFT */
	MK_LBP(BP_SHIFT),                               /* DUK_TOK_RSHIFT */
	MK_LBP(BP_BAND),                                /* DUK_TOK_BAND */
	MK_LBP(BP_BOR),                                 /* DUK_TOK_BOR */
	MK_LBP(BP_BXOR),                                /* DUK_TOK_BXOR */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_LNOT */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_BNOT */
	MK_LBP(BP_LAND),                                /* DUK_TOK_LAND */
	MK_LBP(BP_LOR),                                 /* DUK_TOK_LOR */
	MK_LBP(BP_CONDITIONAL),                         /* DUK_TOK_QUESTION */
	MK_LBP(BP_INVALID),                             /* DUK_TOK_COLON */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_EQUALSIGN */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_ADD_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_SUB_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_MUL_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_DIV_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_MOD_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_ALSHIFT_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_ARSHIFT_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_RSHIFT_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_BAND_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_BOR_EQ */
	MK_LBP(BP_ASSIGNMENT),                          /* DUK_TOK_BXOR_EQ */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_NUMBER */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_STRING */
	MK_LBP_FLAGS(BP_INVALID, TOKEN_LBP_FLAG_NO_REGEXP),  /* DUK_TOK_REGEXP */
};

/*
 *  Misc helpers
 */

static void recursion_increase(duk_compiler_ctx *comp_ctx) {
	DUK_ASSERT(comp_ctx != NULL);
	DUK_ASSERT(comp_ctx->recursion_depth >= 0);
	if (comp_ctx->recursion_depth >= comp_ctx->recursion_limit) {
		DUK_ERROR(comp_ctx->thr, DUK_ERR_INTERNAL_ERROR, "compiler recursion limit reached");
	}
	comp_ctx->recursion_depth++;
}

static void recursion_decrease(duk_compiler_ctx *comp_ctx) {
	DUK_ASSERT(comp_ctx != NULL);
	DUK_ASSERT(comp_ctx->recursion_depth > 0);
	comp_ctx->recursion_depth--;
}

static int hstring_is_eval_or_arguments(duk_compiler_ctx *comp_ctx, duk_hstring *h) {
	DUK_ASSERT(h != NULL);
	return DUK_HSTRING_HAS_EVAL_OR_ARGUMENTS(h);
}

static int hstring_is_eval_or_arguments_in_strict_mode(duk_compiler_ctx *comp_ctx, duk_hstring *h) {
	DUK_ASSERT(h != NULL);
	return (comp_ctx->curr_func.is_strict &&
	        DUK_HSTRING_HAS_EVAL_OR_ARGUMENTS(h));
}

/*
 *  Parser advance() token eating functions
 */

/* FIXME: valstack handling is awkward.  Add a valstack helper which
 * avoids dup():ing; valstack_copy(src, dst)?
 */

static void advance_helper(duk_compiler_ctx *comp_ctx, int expect) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int regexp;

	DUK_ASSERT(comp_ctx->curr_token.t >= 0 && comp_ctx->curr_token.t <= DUK_TOK_MAXVAL);  /* MAXVAL is inclusive */

	/*
	 *  Use current token to decide whether a RegExp can follow.
	 *
	 *  We can use either 't' or 't_nores'; the latter would not
	 *  recognize keywords.  Some keywords can be followed by a
	 *  RegExp (e.g. "return"), so using 't' is better.  This is
	 *  not trivial, see doc/compiler.txt.
	 */

	regexp = 1;
	if (token_lbp[comp_ctx->curr_token.t] & TOKEN_LBP_FLAG_NO_REGEXP) {
		regexp = 0;
	}
	if (comp_ctx->curr_func.reject_regexp_in_adv) {
		comp_ctx->curr_func.reject_regexp_in_adv = 0;
		regexp = 0;
	}

	if (expect >= 0 && comp_ctx->curr_token.t != expect) {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "parse error (expected token %d, got %d on line %d)",
		          expect, comp_ctx->curr_token.t, comp_ctx->curr_token.start_line);
	}

	/* make current token the previous; need to fiddle with valstack "backing store" */
	memcpy(&comp_ctx->prev_token, &comp_ctx->curr_token, sizeof(duk_token));
	duk_dup(ctx, comp_ctx->tok11_idx);
	duk_replace(ctx, comp_ctx->tok21_idx);
	duk_dup(ctx, comp_ctx->tok12_idx);
	duk_replace(ctx, comp_ctx->tok22_idx);

	/* parse new token */
	duk_lexer_parse_js_input_element(&comp_ctx->lex,
	                                 &comp_ctx->curr_token,
	                                 comp_ctx->curr_func.is_strict,
	                                 regexp);

	DUK_DDDPRINT("advance: curr: tok=%d/%d,%d-%d,term=%d,%!T,%!T "
	             "prev: tok=%d/%d,%d-%d,term=%d,%!T,%!T",
	             comp_ctx->curr_token.t,
	             comp_ctx->curr_token.t_nores,
	             comp_ctx->curr_token.start_line,
	             comp_ctx->curr_token.end_line,
	             comp_ctx->curr_token.lineterm,
	             duk_get_tval(ctx, comp_ctx->tok11_idx),
	             duk_get_tval(ctx, comp_ctx->tok12_idx),
	             comp_ctx->prev_token.t,
	             comp_ctx->prev_token.t_nores,
	             comp_ctx->prev_token.start_line,
	             comp_ctx->prev_token.end_line,
	             comp_ctx->prev_token.lineterm,
	             duk_get_tval(ctx, comp_ctx->tok21_idx),
	             duk_get_tval(ctx, comp_ctx->tok22_idx));
}

/* advance, expecting current token to be a specific token; parse next token in regexp context */
static void advance_expect(duk_compiler_ctx *comp_ctx, int expect) {
	advance_helper(comp_ctx, expect);
}

/* advance, whatever the current token is; parse next token in regexp context */
static void advance(duk_compiler_ctx *comp_ctx) {
	advance_helper(comp_ctx, -1);
}

/*
 *  Helpers for duk_compiler_func.
 */

/* init function state: inits valstack allocations */
static void init_function_valstack_slots(duk_compiler_ctx *comp_ctx) {
	duk_compiler_func *func = &comp_ctx->curr_func;
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int entry_top;

	entry_top = duk_get_top(ctx);

	memset(func, 0, sizeof(*func));  /* intentional overlap with earlier memzero */
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	func->h_name = NULL;
	func->h_code = NULL;
	func->h_consts = NULL;
	func->h_funcs = NULL;
	func->h_decls = NULL;
	func->h_labelnames = NULL;
	func->h_labelinfos = NULL;
	func->h_argnames = NULL;
	func->h_varmap = NULL;
#endif

	duk_require_stack(ctx, FUNCTION_INIT_REQUIRE_SLOTS);

	/* FIXME: getter for growable buffer */

	duk_push_new_growable_buffer(ctx, 0);
	func->code_idx = entry_top + 0;
	func->h_code = (duk_hbuffer_growable *) duk_get_hbuffer(ctx, entry_top + 0);
	DUK_ASSERT(func->h_code != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(func->h_code));

	duk_push_new_array(ctx);
	func->consts_idx = entry_top + 1;
	func->h_consts = duk_get_hobject(ctx, entry_top + 1);
	DUK_ASSERT(func->h_consts != NULL);

	duk_push_new_array(ctx);
	func->funcs_idx = entry_top + 2;
	func->h_funcs = duk_get_hobject(ctx, entry_top + 2);
	DUK_ASSERT(func->h_funcs != NULL);

	duk_push_new_array(ctx);
	func->decls_idx = entry_top + 3;
	func->h_decls = duk_get_hobject(ctx, entry_top + 3);
	DUK_ASSERT(func->h_decls != NULL);

	duk_push_new_array(ctx);
	func->labelnames_idx = entry_top + 4;
	func->h_labelnames = duk_get_hobject(ctx, entry_top + 4);
	DUK_ASSERT(func->h_labelnames != NULL);

	duk_push_new_growable_buffer(ctx, 0);
	func->labelinfos_idx = entry_top + 5;
	func->h_labelinfos = (duk_hbuffer_growable *) duk_get_hbuffer(ctx, entry_top + 5);
	DUK_ASSERT(func->h_labelinfos != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(func->h_labelinfos));

	duk_push_new_array(ctx);
	func->argnames_idx = entry_top + 6;
	func->h_argnames = duk_get_hobject(ctx, entry_top + 6);
	DUK_ASSERT(func->h_argnames != NULL);

	duk_push_new_object_internal(ctx);
	func->varmap_idx = entry_top + 7;
	func->h_varmap = duk_get_hobject(ctx, entry_top + 7);
	DUK_ASSERT(func->h_varmap != NULL);
}

/* reset function state (prepare for pass 2) */
static void reset_function_for_pass2(duk_compiler_ctx *comp_ctx) {
	duk_compiler_func *func = &comp_ctx->curr_func;
	duk_hthread *thr = comp_ctx->thr;

	/* FIXME: reset buffers while keeping existing spare */

	duk_hbuffer_reset(thr, func->h_code);
	duk_hobject_set_length_zero(thr, func->h_consts);
	duk_hobject_set_length_zero(thr, func->h_funcs);
	duk_hobject_set_length_zero(thr, func->h_labelnames);
	duk_hbuffer_reset(thr, func->h_labelinfos);
	/* keep func->h_argnames; it is fixed for all passes */
}

/* cleanup varmap from any null entries, compact it, etc; returns number
 * of final entries after cleanup.
 */
static int cleanup_varmap(duk_compiler_ctx *comp_ctx) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_hobject *h_varmap;
	duk_hstring *h_key;
	duk_tval *tv;
	int i, e_used;
	int ret;

	/* [ ... varmap ] */

	h_varmap = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h_varmap != NULL);

	ret = 0;
	e_used = h_varmap->e_used;
	for (i = 0; i < e_used; i++) {
		h_key = DUK_HOBJECT_E_GET_KEY(h_varmap, i);
		if (!h_key) {
			continue;
		}

		DUK_ASSERT(!DUK_HOBJECT_E_SLOT_IS_ACCESSOR(h_varmap, i));

		/* The entries can either be register numbers or 'null' values.
		 * Thus, no need to DECREF them and get side effects.  DECREF'ing
		 * the keys (strings) can cause memory to be freed but no side
		 * effects as strings don't have finalizers.  This is why we can
		 * rely on the object properties not changing from underneath us.
		 */

		tv = DUK_HOBJECT_E_GET_VALUE_TVAL_PTR(h_varmap, i);
		if (!DUK_TVAL_IS_NUMBER(tv)) {
			DUK_ASSERT(!DUK_TVAL_IS_HEAP_ALLOCATED(tv));
			DUK_TVAL_SET_UNDEFINED_UNUSED(tv);
			DUK_HOBJECT_E_SET_KEY(h_varmap, i, NULL);
			DUK_HSTRING_DECREF(thr, h_key);
		} else {
			ret++;
		}
	}

	duk_compact(ctx, -1);

	return ret;
}

/* convert duk_compiler_func into a function template, leaving the result
 * on top of stack.
 */
/* FIXME: awkward and bloated asm -- use faster internal accesses */
static void convert_to_function_template(duk_compiler_ctx *comp_ctx) {
	duk_compiler_func *func = &comp_ctx->curr_func;
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_hcompiledfunction *h_res;
	duk_hbuffer_fixed *h_data;
	size_t consts_count;
	size_t funcs_count;
	size_t code_count;
	size_t code_size;
	size_t data_size;
	size_t i;
	duk_tval *p_const;
	duk_hobject **p_func;
	duk_instr *p_instr;
	duk_compiler_instr *q_instr;
	duk_tval *tv;

	DUK_DDDPRINT("converting duk_compiler_func to function/template");
	DUK_DDPRINT("code=%!xO consts=%!O funcs=%!O", func->h_code, func->h_consts, func->h_funcs);

	/*
	 *  Push result object and init its flags
	 */

	/* Valstack should suffice here, required on function valstack init */

	(void) duk_push_new_compiledfunction(ctx);
	h_res = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);  /* FIXME: specific getter */

	if (func->is_function) {
		DUK_DDDPRINT("function -> set NEWENV");
		DUK_HOBJECT_SET_NEWENV((duk_hobject *) h_res);

		if (!func->is_arguments_shadowed) {
			/* arguments object would be accessible; note that shadowing
			 * bindings are arguments or function declarations, neither
			 * of which are deletable, so this is safe.
			 */

			if (func->id_access_arguments || func->may_direct_eval) {
				DUK_DDDPRINT("function may access 'arguments' object directly or "
				             "indirectly -> set CREATEARGS");
				DUK_HOBJECT_SET_CREATEARGS((duk_hobject *) h_res);
			}
		}
	} else if (func->is_eval && func->is_strict) {
		DUK_DDDPRINT("strict eval code -> set NEWENV");
		DUK_HOBJECT_SET_NEWENV((duk_hobject *) h_res);
	} else {
		/* non-strict eval: env is caller's env or global env (direct vs. indirect call)
		 * global code: env is is global env
		 */
		DUK_DDDPRINT("non-strict eval code or global code -> no NEWENV");
		DUK_ASSERT(!DUK_HOBJECT_HAS_NEWENV((duk_hobject *) h_res));
	}

	if (func->is_function && !func->is_decl && func->h_name != NULL) {
		DUK_DDDPRINT("function expression with a name -> set NAMEBINDING");
		DUK_HOBJECT_SET_NAMEBINDING((duk_hobject *) h_res);
	}

	if (func->is_strict) {
		DUK_DDDPRINT("function is strict -> set STRICT");
		DUK_HOBJECT_SET_STRICT((duk_hobject *) h_res);
	}

	/*
	 *  Build function fixed size 'data' buffer, which contains bytecode,
	 *  constants, and inner function references.
	 *
	 *  During the building phase 'data' is reachable but incomplete.
	 *  Only incref's occur during building (no refzero or GC happens),
	 *  so the building process is atomic.
	 */

	consts_count = duk_hobject_get_length(comp_ctx->thr, func->h_consts);
	funcs_count = duk_hobject_get_length(comp_ctx->thr, func->h_funcs);
	code_count = DUK_HBUFFER_GET_SIZE(func->h_code) / sizeof(duk_compiler_instr);
	code_size = code_count * sizeof(duk_instr);

	data_size = consts_count * sizeof(duk_tval) +
	            funcs_count * sizeof(duk_hobject *) +
	            code_size;

	DUK_DDDPRINT("consts_count=%d, funcs_count=%d, code_size=%d -> "
	             "data_size=%d*%d + %d*%d + %d = %d",
	             (int) consts_count, (int) funcs_count, (int) code_size,
	             (int) consts_count, (int) sizeof(duk_tval),
	             (int) funcs_count, (int) sizeof(duk_hobject *),
	             (int) code_size, (int) data_size);

	duk_push_new_fixed_buffer(ctx, data_size);
	h_data = (duk_hbuffer_fixed *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(h_data != NULL);

	h_res->data = (duk_hbuffer *) h_data;
	DUK_HEAPHDR_INCREF(thr, h_data);

	p_const = (duk_tval *) DUK_HBUFFER_FIXED_GET_DATA_PTR(h_data);
	for (i = 0; i < consts_count; i++) {
		tv = duk_hobject_find_existing_array_entry_tval_ptr(func->h_consts, i);
		DUK_ASSERT(tv != NULL);
		DUK_TVAL_SET_TVAL(p_const, tv);
		p_const++;
		DUK_TVAL_INCREF(thr, tv);  /* may be a string constant */

		DUK_DDDPRINT("constant: %!T", tv);
	}

	p_func = (duk_hobject **) p_const;
	h_res->funcs = p_func;
	for (i = 0; i < funcs_count; i++) {
		duk_hobject *h;
		tv = duk_hobject_find_existing_array_entry_tval_ptr(func->h_funcs, i);
		DUK_ASSERT(tv != NULL);
		DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv));
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION(h));
		*p_func++ = h;
		DUK_HOBJECT_INCREF(thr, h);

		DUK_DDDPRINT("inner function: %p -> %!iO", (void *) h, h);
	}

	p_instr = (duk_instr *) p_func;
	h_res->bytecode = p_instr;

	/* copy bytecode instructions one at a time */
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(func->h_code));
	q_instr = (duk_compiler_instr *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(func->h_code);
	for (i = 0; i < code_count; i++) {
		p_instr[i] = q_instr[i].ins;
	}
	/* Note: 'q_instr' is still used below */

	duk_pop(ctx);  /* 'data' (and everything in it) is reachable through h_res now */

	/*
	 *  Init object properties
	 *
	 *  Properties should be added in decreasing order of access frequency.
	 *  (Not very critical for function templates.)
	 */

	DUK_DDDPRINT("init function properties");

	/* [ ... res ] */

	/* _varmap: omitted if function is guaranteed not to do slow path identifier
	 * accesses or if it would turn out to be empty of actual register mappings
	 * after a cleanup.
	 */
	if (func->id_access_slow ||     /* directly uses slow accesses */
	    func->may_direct_eval ||    /* may indirectly slow access through a direct eval */
	    funcs_count > 0) {          /* has inner functions which may slow access (XXX: this can be optimized by looking at the inner functions) */
		int num_used;
		duk_dup(ctx, func->varmap_idx);
		num_used = cleanup_varmap(comp_ctx);
		DUK_DDDPRINT("cleaned up varmap: %!T (num_used=%d)", duk_get_tval(ctx, -1), num_used);

		if (num_used > 0) {
			duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VARMAP, DUK_PROPDESC_FLAGS_NONE);
		} else {
			DUK_DDDPRINT("varmap is empty after cleanup -> no need to add");
			duk_pop(ctx);
		}
	}

	/* _formals: omitted if function is guaranteed not to need a (non-strict) arguments object */
	if (1) {  /* FIXME: condition */
		/* FIXME: if omitted, recheck handling for 'length' in duk_js_push_closure();
		 * it currently relies on _formals being set.
		 */
		duk_dup(ctx, func->argnames_idx);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_FORMALS, DUK_PROPDESC_FLAGS_NONE);
	}

	/* _name */
	if (func->h_name) {
		duk_push_hstring(ctx, func->h_name);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_NONE);
	}

	/* _source */
	if (0) {
		/* FIXME: Currently function source code is not stored, as it is not
		 * required by the standard.  Source code should not be stored by
		 * default (user should enable it explicitly), and the source should
		 * probably be compressed with a trivial text compressor; average
		 * compression of 20-30% is quite easy to achieve even with a trivial
		 * compressor (RLE + backwards lookup).
		 */

		/*
		 *  For global or eval code this is straightforward.  For functions
		 *  created with the Function constructor we only get the source for
		 *  the body and must manufacture the "function ..." part.
		 *
		 *  For instance, for constructed functions (v8):
		 *
		 *    > a = new Function("foo", "bar", "print(foo)");
		 *    [Function]
		 *    > a.toString()
		 *    'function anonymous(foo,bar) {\nprint(foo)\n}'
		 *
		 *  Similarly for e.g. getters (v8):
		 *
		 *    > x = { get a(foo,bar) { print(foo); } }
		 *    { a: [Getter] }
		 *    > Object.getOwnPropertyDescriptor(x, 'a').get.toString()
		 *    'function a(foo,bar) { print(foo); }'
		 */

		/* FIXME: need tokenizer indices for start and end to substring */
		/* FIXME: always normalize function declaration part? */
		/* FIXME: if we keep _formals, only need to store body */
#if 0
		duk_push_string(ctx, "FIXME");
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_SOURCE, DUK_PROPDESC_FLAGS_NONE);
#endif
	}

	/* _pc2line */
	if (1) {  /* FIXME: condition */
		/*
		 *  Size-optimized pc->line mapping.
		 */

		duk_hobject_pc2line_pack(thr, q_instr, code_count);  /* -> pushes fixed buffer */
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_PC2LINE, DUK_PROPDESC_FLAGS_NONE);

		/* FIXME: if assertions enabled, walk through all valid PCs
		 * and check line mapping.
		 */
	}

	/* _filename */
	if (comp_ctx->h_filename) {
		/*
		 *  Source filename (or equivalent), for identifying thrown errors.
		 */

		duk_push_hstring(ctx, comp_ctx->h_filename);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_FILENAME, DUK_PROPDESC_FLAGS_NONE);
	}

	/*
	 *  Init remaining result fields
	 *
	 *  'nregs' controls how large a register frame is allocated.
	 *
	 *  'nargs' controls how many formal arguments are written to registers:
	 *  r0, ... r(nargs-1).  The remaining registers are initialized to
	 *  undefined.
	 */

	DUK_ASSERT(func->temp_max >= 0);
	h_res->nregs = func->temp_max;
	h_res->nargs = duk_hobject_get_length(thr, func->h_argnames);
	DUK_ASSERT(h_res->nregs >= h_res->nargs);  /* pass2 allocation handles this */

	DUK_DDPRINT("converted function: %!ixT", duk_get_tval(ctx, -1));

#ifdef DUK_USE_DDDEBUG
	{
		duk_hcompiledfunction *h;
		duk_instr *p, *p_start, *p_end;

		h = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
		p_start = (duk_instr *) DUK_HCOMPILEDFUNCTION_GET_CODE_BASE(h);
		p_end = (duk_instr *) DUK_HCOMPILEDFUNCTION_GET_CODE_END(h);

		p = p_start;
		while (p < p_end) {
			DUK_DDDPRINT("BC %04d: %!I        ; 0x%08x op=%d (%!C) a=%d b=%d c=%d",
			             (int) (p - p_start),
			             (*p),
			             (int) (*p),
			             (int) DUK_DEC_OP(*p),
			             (int) DUK_DEC_OP(*p),
			             (int) DUK_DEC_A(*p),
			             (int) DUK_DEC_B(*p),
			             (int) DUK_DEC_C(*p));
			p++;
		}
	}
#endif
}

/*
 *  Code emission helpers
 */

/* FIXME: clarify on when and where CONST_MARKER is allowed */
/* FIXME: opcode specific assertions on when consts are allowed */

/* FIXME: macro smaller than call? */
static int get_current_pc(duk_compiler_ctx *comp_ctx) {
	return DUK_HBUFFER_GET_SIZE(comp_ctx->curr_func.h_code) / sizeof(duk_compiler_instr);
}

static duk_compiler_instr *get_instr_ptr(duk_compiler_ctx *comp_ctx, int pc) {
	duk_compiler_func *f = &comp_ctx->curr_func;
	char *p;
	duk_compiler_instr *code_begin, *code_end;

	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(f->h_code);
	code_begin = (duk_compiler_instr *) p;
	code_end = (duk_compiler_instr *) (p + DUK_HBUFFER_GET_SIZE(f->h_code));
	code_end = code_end;  /* suppress warning */

	DUK_ASSERT(pc >= 0);
	DUK_ASSERT(pc < (code_end - code_begin));

	return &code_begin[pc];
}

/* emit instruction; could return PC but that's not needed in the majority
 * of cases.
 */
static void emit(duk_compiler_ctx *comp_ctx, duk_instr ins) {
	duk_hbuffer_growable *h;
	int line;
	duk_compiler_instr instr;

	h = comp_ctx->curr_func.h_code;
	line = comp_ctx->curr_token.start_line;  /* approximation, close enough */

	instr.ins = ins;
	instr.line = line;

	duk_hbuffer_append_bytes(comp_ctx->thr, h, (duk_u8 *) &instr, sizeof(instr));
}

#if 0 /* unused */
static void emit_op_only(duk_compiler_ctx *comp_ctx, int op) {
	emit(comp_ctx, DUK_ENC_OP_ABC(op, 0));
}
#endif

static void emit_a_b_c(duk_compiler_ctx *comp_ctx, int op, int a, int b, int c) {
	duk_instr ins;

	DUK_ASSERT(op >= DUK_BC_OP_MIN && op <= DUK_BC_OP_MAX);
	if (b & CONST_MARKER) {
		b = (b & ~CONST_MARKER) + 256;
		DUK_ASSERT(b >= DUK_BC_B_MIN + 256 && b <= DUK_BC_B_MAX);
	}
	if (c & CONST_MARKER) {
		c = (c & ~CONST_MARKER) + 256;
		DUK_ASSERT(c >= DUK_BC_C_MIN + 256 && c <= DUK_BC_C_MAX);
	}
	DUK_ASSERT(a >= DUK_BC_A_MIN && a <= DUK_BC_A_MAX);
	DUK_ASSERT(b >= DUK_BC_B_MIN && b <= DUK_BC_B_MAX);
	DUK_ASSERT(c >= DUK_BC_C_MIN && c <= DUK_BC_C_MAX);

	ins = DUK_ENC_OP_A_B_C(op, a, b, c);
	DUK_DDDPRINT("emit: 0x%08x line=%d pc=%d op=%d (%!C) a=%d b=%d c=%d (%!I)",
	             ins, comp_ctx->curr_token.start_line, get_current_pc(comp_ctx), op, op, a, b, c, ins);
	emit(comp_ctx, ins);
}

static void emit_a_b(duk_compiler_ctx *comp_ctx, int op, int a, int b) {
	emit_a_b_c(comp_ctx, op, a, b, 0);
}

#if 0  /* unused */
static void emit_a(duk_compiler_ctx *comp_ctx, int op, int a) {
	emit_a_b_c(comp_ctx, op, a, 0, 0);
}
#endif

static void emit_a_bc(duk_compiler_ctx *comp_ctx, int op, int a, int bc) {
	duk_instr ins;

	/* allow caller to give a const number with the CONST_MARKER */
	bc = bc & (~CONST_MARKER);

	DUK_ASSERT(op >= DUK_BC_OP_MIN && op <= DUK_BC_OP_MAX);
	DUK_ASSERT(a >= DUK_BC_A_MIN && a <= DUK_BC_A_MAX);
	DUK_ASSERT(bc >= DUK_BC_BC_MIN && bc <= DUK_BC_BC_MAX);
	DUK_ASSERT((bc & CONST_MARKER) == 0);

	ins = DUK_ENC_OP_A_BC(op, a, bc);
	DUK_DDDPRINT("emit: 0x%08x line=%d pc=%d op=%d (%!C) a=%d bc=%d (%!I)",
	             ins, comp_ctx->curr_token.start_line, get_current_pc(comp_ctx), op, op, a, bc, ins);
	emit(comp_ctx, ins);
}

static void emit_abc(duk_compiler_ctx *comp_ctx, int op, int abc) {
	duk_instr ins;

	DUK_ASSERT(op >= DUK_BC_OP_MIN && op <= DUK_BC_OP_MAX);
	DUK_ASSERT(abc >= DUK_BC_ABC_MIN && abc <= DUK_BC_ABC_MAX);
	DUK_ASSERT((abc & CONST_MARKER) == 0);

	ins = DUK_ENC_OP_ABC(op, abc);
	DUK_DDDPRINT("emit: 0x%08x line=%d pc=%d op=%d (%!C) abc=%d (%!I)",
	             ins, comp_ctx->curr_token.start_line, get_current_pc(comp_ctx), op, op, abc, ins);
	emit(comp_ctx, ins);
}

static void emit_extraop_b_c(duk_compiler_ctx *comp_ctx, int extraop, int b, int c) {
	DUK_ASSERT(extraop >= DUK_BC_EXTRAOP_MIN && extraop <= DUK_BC_EXTRAOP_MAX);
	emit_a_b_c(comp_ctx, DUK_OP_EXTRA, extraop, b, c);
}

static void emit_extraop_b(duk_compiler_ctx *comp_ctx, int extraop, int b) {
	DUK_ASSERT(extraop >= DUK_BC_EXTRAOP_MIN && extraop <= DUK_BC_EXTRAOP_MAX);
	emit_a_b_c(comp_ctx, DUK_OP_EXTRA, extraop, b, 0);
}

static void emit_extraop_only(duk_compiler_ctx *comp_ctx, int extraop) {
	DUK_ASSERT(extraop >= DUK_BC_EXTRAOP_MIN && extraop <= DUK_BC_EXTRAOP_MAX);
	emit_a_b_c(comp_ctx, DUK_OP_EXTRA, extraop, 0, 0);
}

static void emit_loadint(duk_compiler_ctx *comp_ctx, int reg, int val) {
	/* FIXME: typing */
	/* FIXME: LDINTX support */
	DUK_DDDPRINT("emit loadint: %d -> reg %d", val, reg);
	DUK_ASSERT(reg >= DUK_BC_A_MIN && reg <= DUK_BC_A_MAX);
	DUK_ASSERT(val + DUK_BC_LDINT_BIAS >= DUK_BC_BC_MIN);
	DUK_ASSERT(val + DUK_BC_LDINT_BIAS <= DUK_BC_BC_MAX);
	emit_a_bc(comp_ctx, DUK_OP_LDINT, reg, val + DUK_BC_LDINT_BIAS);
}

static void emit_jump(duk_compiler_ctx *comp_ctx, int target_pc) {
	duk_hbuffer_growable *h;
	int curr_pc;
	int offset;

	h = comp_ctx->curr_func.h_code;
	curr_pc = DUK_HBUFFER_GET_SIZE(h) / sizeof(duk_compiler_instr);
	offset = target_pc - curr_pc - 1;
	DUK_ASSERT(offset + DUK_BC_JUMP_BIAS >= DUK_BC_ABC_MIN);
	DUK_ASSERT(offset + DUK_BC_JUMP_BIAS <= DUK_BC_ABC_MAX);
	emit_abc(comp_ctx, DUK_OP_JUMP, offset + DUK_BC_JUMP_BIAS);
}

static int emit_jump_empty(duk_compiler_ctx *comp_ctx) {
	int ret;

	ret = get_current_pc(comp_ctx);  /* useful for patching jumps later */
	emit_abc(comp_ctx, DUK_OP_JUMP, 0);
	return ret;
}

/* Insert an empty jump in the middle of code emitted earlier.  This is
 * currently needed for compiling for-in.
 */
static void insert_jump_empty(duk_compiler_ctx *comp_ctx, int jump_pc) {
	duk_hbuffer_growable *h;
	int line;
	duk_compiler_instr instr;
	size_t offset;

	h = comp_ctx->curr_func.h_code;
	line = comp_ctx->curr_token.start_line;  /* approximation, close enough */

	instr.ins = DUK_ENC_OP_ABC(DUK_OP_JUMP, 0);
	instr.line = line;
	offset = jump_pc * sizeof(duk_compiler_instr);

	duk_hbuffer_insert_bytes(comp_ctx->thr, h, offset, (duk_u8 *) &instr, sizeof(instr));
}

/* Does not assume that jump_pc contains a DUK_OP_JUMP previously; this is intentional
 * to allow e.g. an INVALID opcode be overwritten with a JUMP (label management uses this).
 */
static void patch_jump(duk_compiler_ctx *comp_ctx, int jump_pc, int target_pc) {
	duk_compiler_instr *instr;
	int offset;

	/* allow negative PCs, behave as a no-op */
	if (jump_pc < 0) {
		DUK_DDDPRINT("patch_jump(): nop call, jump_pc=%d (<0), target_pc=%d", jump_pc, target_pc);
		return;
	}
	DUK_ASSERT(jump_pc >= 0);

	/* FIXME: range assert */
	instr = get_instr_ptr(comp_ctx, jump_pc);
	DUK_ASSERT(instr != NULL);

	/* FIXME: range assert */
	offset = target_pc - jump_pc - 1;

	instr->ins = DUK_ENC_OP_ABC(DUK_OP_JUMP, offset + DUK_BC_JUMP_BIAS);
	DUK_DDDPRINT("patch_jump(): jump_pc=%d, target_pc=%d, offset=%d", jump_pc, target_pc, offset);
}

static void patch_jump_here(duk_compiler_ctx *comp_ctx, int jump_pc) {
	patch_jump(comp_ctx, jump_pc, get_current_pc(comp_ctx));
}

static void patch_trycatch(duk_compiler_ctx *comp_ctx, int trycatch_pc, int reg_catch, int const_varname, int flags) {
	duk_compiler_instr *instr;

	instr = get_instr_ptr(comp_ctx, trycatch_pc);
	DUK_ASSERT(instr != NULL);

	instr->ins = DUK_ENC_OP_A_B_C(DUK_OP_TRYCATCH, flags, reg_catch, const_varname);
}

static void emit_if_false_skip(duk_compiler_ctx *comp_ctx, int regconst) {
	emit_a_b_c(comp_ctx, DUK_OP_IF, 0 /*false*/, regconst, 0);
}

static void emit_if_true_skip(duk_compiler_ctx *comp_ctx, int regconst) {
	emit_a_b_c(comp_ctx, DUK_OP_IF, 1 /*true*/, regconst, 0);
}

static void emit_invalid(duk_compiler_ctx *comp_ctx) {
	emit_abc(comp_ctx, DUK_OP_INVALID, 0);
}

/*
 *  Peephole optimizer for finished bytecode.
 *
 *  Does not remove opcodes; currently only straightens out unconditional
 *  jump chains which are generated by several control structures.
 */

static void peephole_optimize_bytecode(duk_compiler_ctx *comp_ctx) {
	duk_hbuffer_growable *h;
	duk_compiler_instr *bc;
	int iter;
	int i, n;
	int count_opt;

	h = comp_ctx->curr_func.h_code;
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(h));

	bc = (duk_compiler_instr *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(h);
	n = DUK_HBUFFER_GET_SIZE(h) / sizeof(duk_compiler_instr);

	for (iter = 0; iter < DUK_COMPILER_PEEPHOLE_MAXITER; iter++) {
		count_opt = 0;

		for (i = 0; i < n; i++) {
			duk_instr ins;
			int target_pc1;
			int target_pc2;

			ins = bc[i].ins;
			if (DUK_DEC_OP(ins) != DUK_OP_JUMP) {
				continue;
			}
	
			target_pc1 = i + 1 + DUK_DEC_ABC(ins) - DUK_BC_JUMP_BIAS;
			DUK_DDDPRINT("consider jump at pc %d; target_pc=%d", i, target_pc1);
			DUK_ASSERT(target_pc1 >= 0);
			DUK_ASSERT(target_pc1 < n);

			/* Note: if target_pc1 == i, we'll optimize a jump to itself.
			 * This does not need to be checked for explicitly; the case
			 * is rare and max iter breaks us out.
			 */

			ins = bc[target_pc1].ins;
			if (DUK_DEC_OP(ins) != DUK_OP_JUMP) {
				continue;
			}

			target_pc2 = target_pc1 + 1 + DUK_DEC_ABC(ins) - DUK_BC_JUMP_BIAS;

			DUK_DDDPRINT("optimizing jump at pc %d; old target is %d -> new target is %d",
			             i, target_pc1, target_pc2);

			bc[i].ins = DUK_ENC_OP_ABC(DUK_OP_JUMP, target_pc2 - (i + 1) + DUK_BC_JUMP_BIAS);

			count_opt++;
		}

		DUK_DDPRINT("optimized %d jumps on peephole round %d", count_opt, iter + 1);

		if (count_opt == 0) {
			break;
		}
	}
}

/*
 *  Intermediate value helpers
 */

#define  ISREG(comp_ctx,x)              (((x) & CONST_MARKER) == 0)
#define  ISTEMP(comp_ctx,x)             (ISREG((comp_ctx), (x)) && (x) >= ((comp_ctx)->curr_func.temp_first))
#define  GETTEMP(comp_ctx)              ((comp_ctx)->curr_func.temp_next)
#define  SETTEMP(comp_ctx,x)            ((comp_ctx)->curr_func.temp_next = (x))  /* dangerous: must only lower (temp_max not updated) */
#define  SETTEMP_CHECKMAX(comp_ctx,x)   settemp_checkmax((comp_ctx),(x))
#define  ALLOCTEMP(comp_ctx)            alloctemp((comp_ctx))
#define  ALLOCTEMPS(comp_ctx,count)     alloctemps((comp_ctx),(count))

/* FIXME: some code might benefit from SETTEMP_IFTEMP(ctx,x) */

static void copy_ispec(duk_compiler_ctx *comp_ctx, duk_ispec *src, duk_ispec *dst) {
	duk_context *ctx = (duk_context *) comp_ctx->thr;

	/* FIXME: use "dup+replace" primitive */
	dst->t = src->t;
	dst->regconst = src->regconst;
	duk_dup(ctx, src->valstack_idx);
	duk_replace(ctx, dst->valstack_idx);
}

static void copy_ivalue(duk_compiler_ctx *comp_ctx, duk_ivalue *src, duk_ivalue *dst) {
	duk_context *ctx = (duk_context *) comp_ctx->thr;

	/* FIXME: use "dup+replace" primitive */
	dst->t = src->t;
	dst->op = src->op;
	dst->x1.t = src->x1.t;
	dst->x1.regconst = src->x1.regconst;
	dst->x2.t = src->x2.t;
	dst->x2.regconst = src->x2.regconst;
	duk_dup(ctx, src->x1.valstack_idx);
	duk_replace(ctx, dst->x1.valstack_idx);
	duk_dup(ctx, src->x2.valstack_idx);
	duk_replace(ctx, dst->x2.valstack_idx);
}

/* FIXME: to util */
static int is_whole_get_i32(double x, duk_i32 *ival) {
	duk_i32 t;

	if (fpclassify(x) != FP_NORMAL) {
		return 0;
	}

	t = (duk_i32) x;
	if ((double) t == x) {
		*ival = t;
		return 1;
	}

	return 0;
}

static int alloctemps(duk_compiler_ctx *comp_ctx, int num) {
	int res;

	res = comp_ctx->curr_func.temp_next;
	comp_ctx->curr_func.temp_next += num;

	/* FIXME: placeholder, catches most cases */
	if (comp_ctx->curr_func.temp_next > 256) { /* 256 is OK */
		DUK_ERROR(comp_ctx->thr, DUK_ERR_INTERNAL_ERROR, "out of temp regs");
	}

	/* maintain highest 'used' temporary, needed to figure out nregs of function */
	if (comp_ctx->curr_func.temp_next > comp_ctx->curr_func.temp_max) {
		comp_ctx->curr_func.temp_max = comp_ctx->curr_func.temp_next;
	}

	return res;
}

static int alloctemp(duk_compiler_ctx *comp_ctx) {
	return alloctemps(comp_ctx, 1);
}

static void settemp_checkmax(duk_compiler_ctx *comp_ctx, int temp_next) {
	comp_ctx->curr_func.temp_next = temp_next;
	if (temp_next > comp_ctx->curr_func.temp_max) {
		comp_ctx->curr_func.temp_max = temp_next;
	}
}

/* get const for value at valstack top */
static int getconst(duk_compiler_ctx *comp_ctx) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_compiler_func *f = &comp_ctx->curr_func;
	duk_tval *tv1;
	int i, n, n_check;

	n = duk_get_length(ctx, f->consts_idx);

	tv1 = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv1 != NULL);

	/* sanity workaround for handling functions with a large number of
	 * constants at least somewhat reasonably.
	 */
	n_check = (n > GETCONST_MAX_CONSTS_CHECK ? GETCONST_MAX_CONSTS_CHECK : n);
	for (i = 0; i < n_check; i++) {
		duk_tval *tv2 = DUK_HOBJECT_A_GET_VALUE_PTR(f->h_consts, i);

		/* Strict equality is NOT enough, because we cannot use the same
		 * constant for e.g. +0 and -0.
		 */
		if (duk_js_samevalue(tv1, tv2)) {
			DUK_DDDPRINT("reused existing constant for %!T -> const index %d", tv1, i);
			duk_pop(ctx);
			return i | CONST_MARKER;
		}
	}

	/* FIXME: placeholder, catches most cases */
	if (n > 255) { /* 255 is OK */
		DUK_ERROR(comp_ctx->thr, DUK_ERR_INTERNAL_ERROR, "out of consts");
	}

	DUK_DDDPRINT("allocating new constant for %!T -> const index %d", tv1, n);
	(void) duk_put_prop_index(ctx, f->consts_idx, n);  /* invalidates tv1, tv2 */
	return n | CONST_MARKER;
}

/* Get the value represented by an duk_ispec to a register or constant.
 * The caller can control the result by indicating whether or not:
 *
 *   (1) a constant is allowed (sometimes the caller needs the result to
 *       be in a register)
 *
 *   (2) a temporary register is required (usually when caller requires
 *       the register to be safely mutable; normally either a bound
 *       register or a temporary register are both OK)
 *
 *   (3) a forced register target needs to be used
 *
 * Bytecode may be emitted to generate the necessary value.  The return
 * value is either a register or a constant.
 */

static int ispec_toregconst_raw(duk_compiler_ctx *comp_ctx,
                                duk_ispec *x,
                                int forced_reg,
                                int allow_const,
                                int require_temp) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;

	DUK_DDDPRINT("ispec_toregconst_raw(): x={%d:%d:%!T}, "
	             "forced_reg=%d, allow_const=%d, require_temp=%d",
	             x->t, x->regconst, duk_get_tval(ctx, x->valstack_idx),
	             forced_reg, allow_const, require_temp);

	switch (x->t) {
	case DUK_ISPEC_VALUE: {
		duk_tval *tv;

		tv = duk_get_tval(ctx, x->valstack_idx);
		DUK_ASSERT(tv != NULL);

		switch (DUK_TVAL_GET_TAG(tv)) {
		case DUK_TAG_UNDEFINED: {
			/* Note: although there is no 'undefined' literal, undefined
			 * values can occur during compilation as a result of e.g.
			 * the 'void' operator.
			 */
			int dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
			emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_LDUNDEF, dest, 0);
			return dest; 
		}
		case DUK_TAG_NULL: {
			int dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
			emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_LDNULL, dest, 0);
			return dest;
		}
		case DUK_TAG_BOOLEAN: {
			int dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
			emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_LDBOOL, dest, DUK_TVAL_GET_BOOLEAN(tv));
			return dest;
		}
		case DUK_TAG_POINTER: {
			DUK_NEVER_HERE();
			break;
		}
		case DUK_TAG_STRING: {
			duk_hstring *h;
			int dest;
			int constidx;

			h = DUK_TVAL_GET_STRING(tv);
			h = h;  /* suppress warning */
			DUK_ASSERT(h != NULL);

#if 0  /* FIXME: to be implemented? */
			/* Use special opcodes to load short strings */
			if (DUK_HSTRING_GET_BYTELEN(h) <= 2) {
				/* Encode into a single opcode (18 bits can encode 1-2 bytes + length indicator) */
			} else if (DUK_HSTRING_GET_BYTELEN(h) <= 6) {
				/* Encode into a double constant (53 bits can encode 6*8 = 48 bits + 3-bit length */
			}
#endif
			duk_dup(ctx, x->valstack_idx);
			constidx = getconst(comp_ctx);

			if (allow_const) {
				return constidx;
			}

			dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
			emit_a_bc(comp_ctx, DUK_OP_LDCONST, dest, constidx);
			return dest;
		}
		case DUK_TAG_OBJECT: {
			DUK_NEVER_HERE();
			break;
		}
		case DUK_TAG_BUFFER: {
			DUK_NEVER_HERE();
			break;
		}
		default: {
			/* number */
			int constidx;
			int dest;
			double dval;
			duk_i32 ival;

			DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
			dval = DUK_TVAL_GET_NUMBER(tv);

			if (!allow_const) {
				/* A number can be loaded either through a constant or
				 * using LDINT+LDINTX.  Which is better depends on the
				 * context and how many times a certain constant would
				 * be reused.
				 *
				 * Currently, use LDINT if a constant is not allowed
				 * and a LDINT would work.
				 */

				if (is_whole_get_i32(dval, &ival)) {  /* FIXME: to util */
					ival += DUK_BC_LDINT_BIAS;
					if (ival >= DUK_BC_BC_MIN && ival <= DUK_BC_BC_MAX) {
						dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
						emit_a_bc(comp_ctx, DUK_OP_LDINT, dest, ival);
						return dest;
					}
				}
			}

			duk_dup(ctx, x->valstack_idx);
			constidx = getconst(comp_ctx);

			if (allow_const) {
				return constidx;
			} else {
				dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
				emit_a_bc(comp_ctx, DUK_OP_LDCONST, dest, constidx);
				return dest;
			}
		}
		}  /* end switch */
	}
	case DUK_ISPEC_REGCONST: {
		if ((x->regconst & CONST_MARKER) && !allow_const) {
			int dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
			emit_a_bc(comp_ctx, DUK_OP_LDCONST, dest, x->regconst);
			return dest;
		} else {
			if (forced_reg >= 0) {
				if (x->regconst != forced_reg) {
					emit_a_bc(comp_ctx, DUK_OP_LDREG, forced_reg, x->regconst);
				}
				return forced_reg;
			} else {
				if (require_temp && !ISTEMP(comp_ctx, x->regconst)) {
					int dest = ALLOCTEMP(comp_ctx);
					emit_a_bc(comp_ctx, DUK_OP_LDREG, dest, x->regconst);
					return dest;
				} else {
					return x->regconst;
				}
			}
		}
	}
	default: {
		break;
	}
	}

	DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "ispec_toregconst_raw() internal error");
	return 0;	/* FIXME: notreached */
}

static int ispec_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ispec *x, int forced_reg) {
	return ispec_toregconst_raw(comp_ctx, x, forced_reg, 0 /*allow_const*/, 0 /*require_temp*/);
}

/* Coerce an duk_ivalue to a 'plain' value by generating the necessary
 * arithmetic operations, property access, or variable access bytecode.
 *
 * The duk_ivalue argument ('x') is converted into a plain value as a
 * side effect.
 */
static void ivalue_toplain_raw(duk_compiler_ctx *comp_ctx, duk_ivalue *x, int forced_reg) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;

	DUK_DDDPRINT("ivalue_toplain_raw(): x={t=%d,op=%d,x1={%d:%d:%!T},x2={%d:%d:%!T}}, "
	             "forced_reg=%d",
	             x->t, x->op,
	             x->x1.t, x->x1.regconst, duk_get_tval(ctx, x->x1.valstack_idx),
	             x->x2.t, x->x2.regconst, duk_get_tval(ctx, x->x2.valstack_idx),
	             forced_reg);

	switch (x->t) {
	case DUK_IVAL_PLAIN: {
		return;
	}
	/* FIXME: support unary arithmetic ivalues (useful?) */
	case DUK_IVAL_ARITH: {
		int arg1;
		int arg2;
		int dest;
		duk_tval *tv1;
		duk_tval *tv2;

		DUK_DDDPRINT("arith to plain conversion");

		/* inline arithmetic check for constant values */
		/* FIXME: use the exactly same arithmetic function here as in executor */
		if (x->x1.t == DUK_ISPEC_VALUE && x->x2.t == DUK_ISPEC_VALUE) {
			tv1 = duk_get_tval(ctx, x->x1.valstack_idx);
			tv2 = duk_get_tval(ctx, x->x2.valstack_idx);
			DUK_ASSERT(tv1 != NULL);
			DUK_ASSERT(tv2 != NULL);

			DUK_DDDPRINT("arith: tv1=%!T, tv2=%!T", tv1, tv2);

			if (DUK_TVAL_IS_NUMBER(tv1) && DUK_TVAL_IS_NUMBER(tv2)) {
				double d1 = DUK_TVAL_GET_NUMBER(tv1);
				double d2 = DUK_TVAL_GET_NUMBER(tv2);
				double d3;
				int accept = 1;

				DUK_DDDPRINT("arith inline check: d1=%lf, d2=%lf, op=%d", d1, d2, x->op);
				switch (x->op) {
				case DUK_OP_ADD:	d3 = d1 + d2; break;
				case DUK_OP_SUB:	d3 = d1 - d2; break;
				case DUK_OP_MUL:	d3 = d1 * d2; break;
				case DUK_OP_DIV:	d3 = d1 / d2; break;
				default:		accept = 0; break;
				}

				if (accept) {
					x->t = DUK_IVAL_PLAIN;
					DUK_ASSERT(x->x1.t == DUK_ISPEC_VALUE);
					DUK_TVAL_SET_NUMBER(tv1, d3);  /* old value is number: no refcount */
					return;
				}
			} else if (x->op == DUK_OP_ADD && DUK_TVAL_IS_STRING(tv1) && DUK_TVAL_IS_STRING(tv2)) {
				/* inline string concatenation */
				duk_dup(ctx, x->x1.valstack_idx);
				duk_dup(ctx, x->x2.valstack_idx);
				duk_concat(ctx, 2);
				duk_replace(ctx, x->x1.valstack_idx);
				x->t = DUK_IVAL_PLAIN;
				DUK_ASSERT(x->x1.t == DUK_ISPEC_VALUE);
				return;
			}
		}

		arg1 = ispec_toregconst_raw(comp_ctx, &x->x1, -1, 1, 0);  /* no forced reg, allow const, no require temp */
		arg2 = ispec_toregconst_raw(comp_ctx, &x->x2, -1, 1, 0);  /* same flags */

		/* If forced reg, use it as destination.  Otherwise try to
		 * use either coerced ispec if it is a temporary.
		 */
		if (forced_reg >= 0) {
			dest = forced_reg;
		} else if (ISTEMP(comp_ctx, arg1)) {
			dest = arg1;
		} else if (ISTEMP(comp_ctx, arg2)) {
			dest = arg2;
		} else {
			dest = ALLOCTEMP(comp_ctx);
		}

		emit_a_b_c(comp_ctx, x->op, dest, arg1, arg2);

		x->t = DUK_IVAL_PLAIN;
		x->x1.t = DUK_ISPEC_REGCONST;
		x->x1.regconst = dest;
		return;
	}
	case DUK_IVAL_PROP: {
		int arg1 = ispec_toregconst_raw(comp_ctx, &x->x1, -1, 1, 0);  /* no forced reg, allow const, no require temp */
		int arg2 = ispec_toregconst_raw(comp_ctx, &x->x2, -1, 1, 0);  /* same flags */
		int dest;

		if (forced_reg >= 0) {
			dest = forced_reg;
		} else if (ISTEMP(comp_ctx, arg1)) {
			dest = arg1;
		} else if (ISTEMP(comp_ctx, arg2)) {
			dest = arg2;
		} else {
			dest = ALLOCTEMP(comp_ctx);
		}

		emit_a_b_c(comp_ctx, DUK_OP_GETPROP, dest, arg1, arg2);

		x->t = DUK_IVAL_PLAIN;
		x->x1.t = DUK_ISPEC_REGCONST;
		x->x1.regconst = dest;
		return;
	}
	case DUK_IVAL_VAR: {
		/* x1 must be a string */
		int dest;
		int reg_varbind;
		int reg_varname;

		DUK_ASSERT(x->x1.t == DUK_ISPEC_VALUE);

		duk_dup(ctx, x->x1.valstack_idx);
		if (lookup_lhs(comp_ctx, &reg_varbind, &reg_varname)) {
			x->t = DUK_IVAL_PLAIN;
			x->x1.t = DUK_ISPEC_REGCONST;
			x->x1.regconst = reg_varbind;
		} else {
			dest = (forced_reg >= 0 ? forced_reg : ALLOCTEMP(comp_ctx));
			emit_a_b(comp_ctx, DUK_OP_GETVAR, dest, reg_varname);
			x->t = DUK_IVAL_PLAIN;
			x->x1.t = DUK_ISPEC_REGCONST;
			x->x1.regconst = dest;
		}
		return;
	}
	case DUK_IVAL_NONE:
	default: {
		break;
	}
	}

	DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "ivalue_toplain_raw() internal error");
	return;	/* FIXME: unreachable */
}

/* evaluate to plain value, no forced register (temp/bound reg both ok) */
static void ivalue_toplain(duk_compiler_ctx *comp_ctx, duk_ivalue *x) {
	ivalue_toplain_raw(comp_ctx, x, -1);  /* no forced reg */
}

/* evaluate to final form (e.g. coerce GETPROP to code), throw away temp */
static void ivalue_toplain_ignore(duk_compiler_ctx *comp_ctx, duk_ivalue *x) {
	int temp;
	temp = GETTEMP(comp_ctx);
	ivalue_toplain_raw(comp_ctx, x, -1);  /* no forced reg */
	SETTEMP(comp_ctx, temp);
}

/* Coerce an duk_ivalue to a register or constant; result register may
 * be a temp or a bound register.
 *
 * The duk_ivalue argument ('x') is converted into a regconst as a
 * side effect.
 */
static int ivalue_toregconst_raw(duk_compiler_ctx *comp_ctx,
                                 duk_ivalue *x,
                                 int forced_reg,
                                 int allow_const,
                                 int require_temp) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int reg;

	thr = thr;  /* suppress warnings */
	ctx = ctx;

	DUK_DDDPRINT("ivalue_toregconst_raw(): x={t=%d,op=%d,x1={%d:%d:%!T},x2={%d:%d:%!T}}, "
	             "forced_reg=%d, allow_const=%d, require_temp=%d",
	             x->t, x->op,
	             x->x1.t, x->x1.regconst, duk_get_tval(ctx, x->x1.valstack_idx),
	             x->x2.t, x->x2.regconst, duk_get_tval(ctx, x->x2.valstack_idx),
	             forced_reg, allow_const, require_temp);

	/* first coerce to a plain value */
	ivalue_toplain_raw(comp_ctx, x, forced_reg);
	DUK_ASSERT(x->t == DUK_IVAL_PLAIN);

	/* then to a register */
	reg = ispec_toregconst_raw(comp_ctx, &x->x1, forced_reg, allow_const, require_temp);
	x->x1.t = DUK_ISPEC_REGCONST;
	x->x1.regconst = reg;

	return reg;
}

static int ivalue_toreg(duk_compiler_ctx *comp_ctx, duk_ivalue *x) {
	return ivalue_toregconst_raw(comp_ctx, x, -1, 0, 0);  /* no forced reg, don't allow const, don't require temp */
}

#if 0  /* unused */
static int ivalue_totempreg(duk_compiler_ctx *comp_ctx, duk_ivalue *x) {
	return ivalue_toregconst_raw(comp_ctx, x, -1, 0, 1);  /* no forced reg, don't allow const, require temp */
}
#endif

static int ivalue_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ivalue *x, int forced_reg) {
	return ivalue_toregconst_raw(comp_ctx, x, forced_reg, 0, 0);  /* forced reg, don't allow const, don't require temp */
}

static int ivalue_toregconst(duk_compiler_ctx *comp_ctx, duk_ivalue *x) {
	return ivalue_toregconst_raw(comp_ctx, x, -1, 1, 0);  /* no forced reg, allow const, don't require temp */
}

/* The issues below can be solved with better flags */

/* FIXME: many operations actually want toforcedtemp() -- brand new temp? */
/* FIXME: need a toplain_ignore() which will only coerce a value to a temp
 * register if it might have a side effect.  Side-effect free values do not
 * need to be coerced.
 */

/*
 *  Identifier handling
 */

static int lookup_active_register_binding(duk_compiler_ctx *comp_ctx) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *h_varname;
	int ret;

	DUK_DDDPRINT("resolving identifier reference to '%!T'", duk_get_tval(ctx, -1));

	/*
	 *  Special name handling
	 */

	h_varname = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_varname != NULL);

	if (h_varname == DUK_HTHREAD_STRING_LC_ARGUMENTS(thr)) {
		DUK_DDDPRINT("flagging function as accessing 'arguments'");
		comp_ctx->curr_func.id_access_arguments = 1;
	}

	/*
	 *  Inside one or more 'with' statements fall back to slow path always.
	 *  (See e.g. test-stmt-with.js.)
	 */

	if (comp_ctx->curr_func.with_depth > 0) {
		DUK_DDDPRINT("identifier lookup inside a 'with' -> fall back to slow path");
		goto slow_path;
	}

	/*
	 *  Any catch bindings ("catch (e)") also affect identifier binding.
	 *
	 *  Currently, the varmap is modified for the duration of the catch
	 *  clause to ensure any identifier accesses with the catch variable
	 *  name will use slow path.
	 */

	duk_get_prop(ctx, comp_ctx->curr_func.varmap_idx);
	if (duk_is_number(ctx, -1)) {
		ret = duk_to_int(ctx, -1);
		duk_pop(ctx);
	} else {
		duk_pop(ctx);
		goto slow_path;
	}

	DUK_DDDPRINT("identifier lookup -> reg %d", ret);
	return ret;

 slow_path:
	DUK_DDDPRINT("identifier lookup -> slow path");

	comp_ctx->curr_func.id_access_slow = 1;
	return -1;
}

/* Lookup an identifier name in the current varmap, indicating whether the
 * identifier is register-bound and if not, allocating a constant for the
 * identifier name.  Returns 1 if register-bound, 0 otherwise.
 */
static int lookup_lhs(duk_compiler_ctx *comp_ctx, int *out_reg_varbind, int *out_reg_varname) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int reg_varbind;
	int reg_varname;

	/* [ ... varname ] */

	duk_dup_top(ctx);
	reg_varbind = lookup_active_register_binding(comp_ctx);

	if (reg_varbind >= 0) {
		*out_reg_varbind = reg_varbind;
		*out_reg_varname = -1;
		duk_pop(ctx);
		return 1;
	} else {
		reg_varname = getconst(comp_ctx);
		*out_reg_varbind = -1;
		*out_reg_varname = reg_varname;
		return 0;
	}
}

/*
 *  Label handling
 *
 *  Labels are initially added with flags prohibiting both break and continue.
 *  When the statement type is finally uncovered (after potentially multiple
 *  labels), all the labels are updated to allow/prohibit break and continue.
 */

static void add_label(duk_compiler_ctx *comp_ctx, duk_hstring *h_label, int pc_label, int label_id) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	size_t n;
	size_t new_size;
	char *p;
	duk_labelinfo *li_start, *li;

	/* Duplicate (shadowing) labels are not allowed, except for the empty
	 * labels (which are used as default labels for switch and iteration
	 * statements).
	 *
	 * We could also allow shadowing of non-empty pending labels without any
	 * other issues than breaking the required label shadowing requirements
	 * of the E5 specification, see Section 12.12.
	 */

	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(comp_ctx->curr_func.h_labelinfos);
	li_start = (duk_labelinfo *) p;
	li = (duk_labelinfo *) (p + DUK_HBUFFER_GET_SIZE(comp_ctx->curr_func.h_labelinfos));
	n = (size_t) (li - li_start);

	while (li > li_start) {
		li--;

		if (li->h_label == h_label && h_label != DUK_HTHREAD_STRING_EMPTY_STRING(thr)) {
			DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "duplicate (non-empty) label");
		}
	}

	/* XXX: awkward */
	duk_push_hstring(ctx, h_label);
	(void) duk_put_prop_index(ctx, comp_ctx->curr_func.labelnames_idx, n);

	new_size = (n + 1) * sizeof(duk_labelinfo);
	duk_hbuffer_resize(thr, comp_ctx->curr_func.h_labelinfos, new_size, new_size);
	/* FIXME: spare handling, slow now */

	/* relookup after possible realloc */
	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(comp_ctx->curr_func.h_labelinfos);
	li_start = (duk_labelinfo *) p;
	li = (duk_labelinfo *) (p + DUK_HBUFFER_GET_SIZE(comp_ctx->curr_func.h_labelinfos));
	li--;

	/* Labels need to be recorded as pending before we know whether they will be
	 * actually be used as part of an iteration statement or a switch statement.
	 * The flags to allow break/continue are updated when we figure out the
	 * statement type.
	 */

	li->flags = 0;
	li->label_id = label_id;
	li->h_label = h_label;
	li->catch_depth = comp_ctx->curr_func.catch_depth;   /* catch depth from current func */
	li->pc_label = pc_label;

	DUK_DDDPRINT("registered label: flags=0x%08x, id=%d, name=%!O, catch_depth=%d, pc_label=%d",
	             li->flags, li->label_id, li->h_label, li->catch_depth, li->pc_label);
}

/* Update all labels with matching label_id. */
static void update_label_flags(duk_compiler_ctx *comp_ctx, int label_id, int flags) {
	char *p;
	duk_labelinfo *li_start, *li;

	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(comp_ctx->curr_func.h_labelinfos);
	li_start = (duk_labelinfo *) p;
	li = (duk_labelinfo *) (p + DUK_HBUFFER_GET_SIZE(comp_ctx->curr_func.h_labelinfos));

	/* Match labels starting from latest; once label_id no longer matches, we can
	 * safely exit without checking the rest of the labels (only the topmost labels
	 * are ever updated).
	 */
	while (li > li_start) {
		li--;

		if (li->label_id != label_id) {
			break;
		}

		DUK_DDDPRINT("updating label flags for li=%p, label_id=%d, flags=%d",
		             (void *) li, label_id, flags);

		li->flags = flags;
	}
}

/* Lookup active label information.  Break/continue distinction is necessary to handle switch
 * statement related labels correctly: a switch will only catch a 'break', not a 'continue'.
 *
 * An explicit label cannot appear multiple times in the active set, but empty labels (unlabelled
 * iteration and switch statements) can.  A break will match the closest unlabelled or labelled
 * statement.  A continue will match the closest unlabelled or labelled iteration statement.  It is
 * a syntax error if a continue matches a labelled switch statement; because an explicit label cannot
 * be duplicated, the continue cannot match any valid label outside the switch.
 *
 * A side effect of these rules is that a LABEL statement related to a switch should never actually
 * catch a continue abrupt completion at run-time.  Hence an INVALID opcode can be placed in the
 * continue slot of the switch's LABEL statement.
 */

/* FIXME: awkward, especially the bunch of separate output values -> output struct? */
static void lookup_active_label(duk_compiler_ctx *comp_ctx, duk_hstring *h_label, int is_break, int *out_label_id, int *out_label_catch_depth, int *out_label_pc, int *out_is_closest) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	char *p;
	duk_labelinfo *li_start, *li_end, *li;
	int match = 0;

	DUK_DDDPRINT("looking up active label: label='%!O', is_break=%d", h_label, is_break);

	ctx = ctx;  /* suppress warning */

	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(comp_ctx->curr_func.h_labelinfos);
	li_start = (duk_labelinfo *) p;
	li_end = (duk_labelinfo *) (p + DUK_HBUFFER_GET_SIZE(comp_ctx->curr_func.h_labelinfos));
	li = li_end;

	/* Match labels starting from latest label because there can be duplicate empty
	 * labels in the label set.
	 */
	while (li > li_start) {
		li--;

		if (li->h_label != h_label) {
			DUK_DDDPRINT("labelinfo[%d] ->'%!O' != %!O",
			             (int) (li - li_start), li->h_label, h_label);
			continue;
		}

		DUK_DDDPRINT("labelinfo[%d] -> '%!O' label name matches (still need to check type)",
		             (int) (li - li_start), h_label);

		/* currently all labels accept a break, so no explicit check for it now */
		DUK_ASSERT(li->flags & DUK_LABEL_FLAG_ALLOW_BREAK);

		if (is_break) {
			/* break matches always */
			match = 1;
			break;
		} else if (li->flags & DUK_LABEL_FLAG_ALLOW_CONTINUE) {
			/* iteration statements allow continue */
			match = 1;
			break;
		} else {
			/* continue matched this label -- we can only continue if this is the empty
			 * label, for which duplication is allowed, and thus there is hope of
			 * finding a match deeper in the label stack.
			 */
			if (h_label != DUK_HTHREAD_STRING_EMPTY_STRING(thr)) {
				DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "continue label matches an invalid statement type");
			} else {
				DUK_DDDPRINT("continue matched an empty label which does not "
				             "allow a continue -> continue lookup deeper in label stack");
			}
		}
	}
	/* FIXME: match flag is awkward, rework */
	if (!match) {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "cannot resolve label");
	}

	DUK_DDDPRINT("label match: %!O -> label_id %d, catch_depth=%d, pc_label=%d",
	             h_label, li->label_id, li->catch_depth, li->pc_label);

	*out_label_id = li->label_id;
	*out_label_catch_depth = li->catch_depth;
	*out_label_pc = li->pc_label;
	*out_is_closest = (li == li_end - 1);
}

static void reset_labels_to_length(duk_compiler_ctx *comp_ctx, int len) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	size_t new_size;

	/* FIXME: duk_set_length */
	new_size = sizeof(duk_labelinfo) * len;
	duk_push_int(ctx, len);
	duk_put_prop_stridx(ctx, comp_ctx->curr_func.labelnames_idx, DUK_STRIDX_LENGTH);
	duk_hbuffer_resize(thr, comp_ctx->curr_func.h_labelinfos, new_size, new_size);  /* FIXME: spare handling */
}

/*
 *  Expression parsing: expr_nud(), expr_led(), expr_lbp(), and helpers.
 *
 *  - expr_nud(): ("null denotation"): process prev_token as a "start" of an expression (e.g. literal)
 *  - expr_led(): ("left denotation"): process prev_token in the "middle" of an expression (e.g. operator)
 *  - expr_lbp(): ("left-binding power"): return left-binding power of curr_token
 */

/* object literal key tracking flags */
#define  OBJ_LIT_KEY_PLAIN  (1 << 0)  /* key encountered as a plain property */
#define  OBJ_LIT_KEY_GET    (1 << 1)  /* key encountered as a getter */
#define  OBJ_LIT_KEY_SET    (1 << 2)  /* key encountered as a setter */

static void nud_array_literal(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	duk_hthread *thr = comp_ctx->thr;
	int reg_obj;            /* result reg */
	int max_init_values;    /* max # of values initialized in one MPUTARR set */
	int temp_start;         /* temp reg value for start of loop */
	int num_values;         /* number of values in current MPUTARR set */
	int curr_idx;           /* current (next) array index */
	int start_idx;          /* start array index of current MPUTARR set */
	int init_idx;           /* last array index explicitly initialized, +1 */
	int reg_temp;           /* temp reg */
	int require_comma;      /* next loop requires a comma */

	/* DUK_TOK_LBRACKET already eaten, current token is right after that */
	DUK_ASSERT(comp_ctx->prev_token.t == DUK_TOK_LBRACKET);

	max_init_values = MAX_ARRAY_INIT_VALUES;  /* XXX: depend on available temps? */

	reg_obj = ALLOCTEMP(comp_ctx);
	emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_NEWARR, reg_obj, 0);  /* XXX: patch initial size afterwards? */
 	temp_start = GETTEMP(comp_ctx);

	/*
	 *  Emit initializers in sets of maximum max_init_values.
	 *  Corner cases such as single value initializers do not have
	 *  special handling now.
	 *
	 *  Elided elements must not be emitted as 'undefined' values,
	 *  because such values would be enumerable (which is incorrect).
	 *  Also note that trailing elisions must be reflected in the
	 *  length of the final array but cause no elements to be actually
	 *  inserted.
	 */

	curr_idx = 0;
	init_idx = 0;         /* tracks maximum initialized index + 1 */
	start_idx = 0;
	require_comma = 0;

	for (;;) {
		num_values = 0;
		SETTEMP(comp_ctx, temp_start);

		if (comp_ctx->curr_token.t == DUK_TOK_RBRACKET) {
			break;
		}

		for (;;) {
			if (comp_ctx->curr_token.t == DUK_TOK_RBRACKET) {
				/* the outer loop will recheck and exit */
				break;
			}

			/* comma check */
			if (require_comma) {
				if (comp_ctx->curr_token.t == DUK_TOK_COMMA) {
					/* comma after a value, expected */
					advance(comp_ctx);
					require_comma = 0;
					continue;
				} else {
					goto syntax_error;
				}
			} else {
				if (comp_ctx->curr_token.t == DUK_TOK_COMMA) {
					/* elision - flush */
					curr_idx++;
					advance(comp_ctx);
					/* if num_values > 0, MPUTARR emitted by outer loop after break */
					break;
				}
			}
			/* else an array initializer element */

			/* initial index */
			if (num_values == 0) {
				start_idx = curr_idx;
				reg_temp = ALLOCTEMP(comp_ctx);
				emit_loadint(comp_ctx, reg_temp, start_idx);
			}

			reg_temp = ALLOCTEMP(comp_ctx);   /* alloc temp just in case, to update max temp */
			SETTEMP(comp_ctx, reg_temp);      /* hope that the sub-expression writes to reg_temp */
			expr_toforcedreg(comp_ctx, res, BP_COMMA /*rbp_flags*/, reg_temp /*forced_reg*/);
			SETTEMP(comp_ctx, reg_temp + 1);

			num_values++;
			curr_idx++;
			require_comma = 1;

			if (num_values >= max_init_values) {
				/* MPUTARR emitted by outer loop */
				break;
			}
		}

		if (num_values > 0) {
			emit_a_b_c(comp_ctx, DUK_OP_MPUTARR, reg_obj, temp_start, num_values);
			init_idx = start_idx + num_values;
#if 0  /* these are not necessary, as they're done at the top of the loop */
			num_values = 0;
			SETTEMP(comp_ctx, temp_start);
#endif
		}	
	}

	DUK_ASSERT(comp_ctx->curr_token.t == DUK_TOK_RBRACKET);
	advance(comp_ctx);

	DUK_DDDPRINT("array literal done, curridx=%d, initidx=%d", curr_idx, init_idx);

	/* trailing elisions? */
	if (curr_idx > init_idx) {
		/* yes, must set array length explicitly */
		DUK_DDDPRINT("array literal has trailing elisions which affect its length");
		reg_temp = ALLOCTEMP(comp_ctx);
		emit_loadint(comp_ctx, reg_temp, curr_idx);
		emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_SETALEN, reg_obj, reg_temp);
	}

	SETTEMP(comp_ctx, temp_start);

	res->t = DUK_IVAL_PLAIN;
	res->x1.t = DUK_ISPEC_REGCONST;
	res->x1.regconst = reg_obj;
	return;

 syntax_error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid array literal");
}

/* duplicate/invalid key checks; returns 1 if syntax error */
static int nud_object_literal_key_check(duk_compiler_ctx *comp_ctx, int new_key_flags) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int key_flags;

	/* [ ... key_obj key ] */

	DUK_ASSERT(duk_is_string(ctx, -1));

	/*
	 *  'key_obj' tracks keys encountered so far by associating an
	 *  integer with flags with already encountered keys.  The checks
	 *  below implement E5 Section 11.1.5, step 4 for production:
	 *
	 *    PropertyNameAndValueList: PropertyNameAndValueList , PropertyAssignment
	 */

	duk_dup(ctx, -1);       /* [ ... key_obj key key ] */
	duk_get_prop(ctx, -3);  /* [ ... key_obj key val ] */
	key_flags = duk_to_int(ctx, -1);
	duk_pop(ctx);           /* [ ... key_obj key ] */

	if (new_key_flags & OBJ_LIT_KEY_PLAIN) {
		if ((key_flags & OBJ_LIT_KEY_PLAIN) && comp_ctx->curr_func.is_strict) {
			/* step 4.a */
			DUK_DDDPRINT("duplicate key: plain key appears twice in strict mode");
			return 1;
		}
		if (key_flags & (OBJ_LIT_KEY_GET | OBJ_LIT_KEY_SET)) {
			/* step 4.c */
			DUK_DDDPRINT("duplicate key: plain key encountered after setter/getter");
			return 1;
		}
	} else {
		if (key_flags & OBJ_LIT_KEY_PLAIN) {
			/* step 4.b */
			DUK_DDDPRINT("duplicate key: getter/setter encountered after plain key");
			return 1;
		}
		if (key_flags & new_key_flags) {
			/* step 4.d */
			DUK_DDDPRINT("duplicate key: getter/setter encountered twice");
			return 1;
		}
	}

	new_key_flags |= key_flags;
	DUK_DDDPRINT("setting/updating key %!T flags: 0x%08x -> 0x%08x",
	             duk_get_tval(ctx, -1), key_flags, new_key_flags);
	duk_dup(ctx, -1);
	duk_push_int(ctx, new_key_flags);   /* [ ... key_obj key key flags ] */
	duk_put_prop(ctx, -4);              /* [ ... key_obj key ] */

	return 0;
}

static void nud_object_literal(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int reg_obj;            /* result reg */
	int max_init_pairs;     /* max # of key-value pairs initialized in one MPUTOBJ set */
	int temp_start;         /* temp reg value for start of loop */
	int num_pairs;          /* number of pairs in current MPUTOBJ set */
	int reg_key;            /* temp reg for key literal */
	int reg_temp;           /* temp reg */

	DUK_ASSERT(comp_ctx->prev_token.t == DUK_TOK_LCURLY);

	max_init_pairs = MAX_OBJECT_INIT_PAIRS;  /* XXX: depend on available temps? */

	reg_obj = ALLOCTEMP(comp_ctx);
	emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_NEWOBJ, reg_obj, 0);  /* XXX: patch initial size afterwards? */
	temp_start = GETTEMP(comp_ctx);

	/* temp object for tracking / detecting duplicate keys */
	duk_push_new_object(ctx);

	/*
	 *  Emit initializers in sets of maximum max_init_pairs keys.
	 *  Setter/getter is handled separately and terminates the
	 *  current set of initializer values.  Corner cases such as
	 *  single value initializers do not have special handling now.
	 */

	for (;;) {
		num_pairs = 0;
		SETTEMP(comp_ctx, temp_start);

		if (comp_ctx->curr_token.t == DUK_TOK_RCURLY) {
			break;
		}

		for (;;) {
			/*
			 *  Three possible element formats:
			 *    1) PropertyName : AssignmentExpression
			 *    2) get PropertyName () { FunctionBody }
			 *    3) set PropertyName ( PropertySetParameterList ) { FunctionBody }
			 *
			 *  PropertyName can be IdentifierName (includes reserved words), a string
			 *  literal, or a number literal.  Note that IdentifierName allows 'get' and
			 *  'set' too, so we need to look ahead to the next token to distinguish:
			 *
			 *     { get : 1 }
			 *
			 *  and
			 *
			 *     { get foo() { return 1 } }
			 *     { get get() { return 1 } }    // 'get' as getter propertyname
			 *
			 *  Finally, a trailing comma is allowed.
			 *
			 *  Key name is coerced to string at compile time (and ends up as a
			 *  a string constant) even for numeric keys (e.g. "{1:'foo'}").
			 *  These could be emitted using e.g. LDINT, but that seems hardly
			 *  worth the effort and would increase code size.
			 */ 

			DUK_DDDPRINT("object literal inner loop, curr_token->t = %d", comp_ctx->curr_token.t);

			if (comp_ctx->curr_token.t == DUK_TOK_RCURLY) {
				/* the outer loop will recheck and exit */
				break;
			}

			/* advance to get one step of lookup */		
			advance(comp_ctx);

			if ((comp_ctx->prev_token.t == DUK_TOK_GET || comp_ctx->prev_token.t == DUK_TOK_SET) &&
			     comp_ctx->curr_token.t != DUK_TOK_COLON) {
				/* getter/setter */
				int is_getter = (comp_ctx->prev_token.t == DUK_TOK_GET);
				int fnum;
				int reg_temp;

				if (comp_ctx->curr_token.t_nores == DUK_TOK_IDENTIFIER ||
				    comp_ctx->curr_token.t_nores == DUK_TOK_STRING) {
					/* same handling for identifiers and strings */
					DUK_ASSERT(comp_ctx->curr_token.str1 != NULL);
					duk_push_hstring(ctx, comp_ctx->curr_token.str1);
				} else if (comp_ctx->curr_token.t == DUK_TOK_NUMBER) {
					duk_push_number(ctx, comp_ctx->curr_token.num);
					duk_to_string(ctx, -1);
				} else {
					goto syntax_error;
				}

				DUK_ASSERT(duk_is_string(ctx, -1));
				if (nud_object_literal_key_check(comp_ctx,
				                                 (is_getter ? OBJ_LIT_KEY_GET : OBJ_LIT_KEY_SET))) {
					goto syntax_error;
				}
				reg_key = getconst(comp_ctx);

				if (num_pairs > 0) {
					emit_a_b_c(comp_ctx, DUK_OP_MPUTOBJ, reg_obj, temp_start, num_pairs);
					num_pairs = 0;
					SETTEMP(comp_ctx, temp_start);
				}

				/* curr_token = get/set name */
				fnum = parse_function_like_fnum(comp_ctx, 0 /*is_decl*/, 1 /*is_setget*/);

				DUK_ASSERT(GETTEMP(comp_ctx) == temp_start);
				reg_temp = ALLOCTEMP(comp_ctx);
				emit_a_bc(comp_ctx, DUK_OP_LDCONST, reg_temp, reg_key);
				reg_temp = ALLOCTEMP(comp_ctx);
				emit_a_bc(comp_ctx, DUK_OP_CLOSURE, reg_temp, fnum);
				emit_extraop_b_c(comp_ctx,
				                 (is_getter ? DUK_EXTRAOP_INITGET : DUK_EXTRAOP_INITSET),
				                 reg_obj,
				                 temp_start);   /* temp_start+0 = key, temp_start+1 = closure */

				SETTEMP(comp_ctx, temp_start);
			} else {
				/* normal key/value */
				if (comp_ctx->prev_token.t_nores == DUK_TOK_IDENTIFIER ||
				    comp_ctx->prev_token.t_nores == DUK_TOK_STRING) {
					/* same handling for identifiers and strings */
					DUK_ASSERT(comp_ctx->prev_token.str1 != NULL);
					duk_push_hstring(ctx, comp_ctx->prev_token.str1);
				} else if (comp_ctx->prev_token.t == DUK_TOK_NUMBER) {
					duk_push_number(ctx, comp_ctx->prev_token.num);
					duk_to_string(ctx, -1);
				} else {
					goto syntax_error;
				}

				DUK_ASSERT(duk_is_string(ctx, -1));
				if (nud_object_literal_key_check(comp_ctx, OBJ_LIT_KEY_PLAIN)) {
					goto syntax_error;
				}
				reg_key = getconst(comp_ctx);

				reg_temp = ALLOCTEMP(comp_ctx);
				emit_a_bc(comp_ctx, DUK_OP_LDCONST, reg_temp, reg_key);
				advance_expect(comp_ctx, DUK_TOK_COLON);

				reg_temp = ALLOCTEMP(comp_ctx);  /* alloc temp just in case, to update max temp */
				SETTEMP(comp_ctx, reg_temp);
				expr_toforcedreg(comp_ctx, res, BP_COMMA /*rbp_flags*/, reg_temp /*forced_reg*/);
				SETTEMP(comp_ctx, reg_temp + 1);

				num_pairs++;
			}

			if (comp_ctx->curr_token.t != DUK_TOK_COMMA) {
				break;
			}
			advance(comp_ctx);

			if (num_pairs >= max_init_pairs) {
				/* MPUTOBJ emitted by outer loop */
				break;
			}
		}

		if (num_pairs > 0) {
			emit_a_b_c(comp_ctx, DUK_OP_MPUTOBJ, reg_obj, temp_start, num_pairs);
#if 0  /* these are not necessary, as they're done at the top of the loop */
			num_pairs = 0;
			SETTEMP(comp_ctx, temp_start);
#endif
		}
	}

	DUK_ASSERT(comp_ctx->curr_token.t == DUK_TOK_RCURLY);
	advance(comp_ctx);

	SETTEMP(comp_ctx, temp_start);

	res->t = DUK_IVAL_PLAIN;
	res->x1.t = DUK_ISPEC_REGCONST;
	res->x1.regconst = reg_obj;

	DUK_DDDPRINT("final tracking object: %!T", duk_get_tval(ctx, -1));
	duk_pop(ctx);
	return;

 syntax_error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid object literal");
}

/* Parse argument list.  Arguments are written to temps starting from
 * "next temp".  Returns number of arguments parsed.  Expects left paren
 * to be already eaten, and eats the right paren before returning.
 */
static int parse_arguments(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	int nargs = 0;
	int tr;

	/* Note: expect that caller has already eaten the left paren */

	DUK_DDDPRINT("start parsing arguments, prev_token.t=%d, curr_token.t=%d",
	             comp_ctx->prev_token.t, comp_ctx->curr_token.t);

	for (;;) {
		if (comp_ctx->curr_token.t == DUK_TOK_RPAREN) {
			break;
		}
		if (nargs > 0) {
			advance_expect(comp_ctx, DUK_TOK_COMMA);
		}

		/* We want the argument expression value to go to "next temp"
		 * without additional moves.  That should almost always be the
		 * case, but we double check after expression parsing.
		 *
		 * This is not the cleanest possible approach.
		 */

		tr = ALLOCTEMP(comp_ctx);  /* bump up "allocated" reg count, just in case */
		SETTEMP(comp_ctx, tr);

		/* binding power must be high enough to NOT allow comma expressions directly */
		expr_toforcedreg(comp_ctx, res, BP_COMMA /*rbp*/, tr);  /* always allow 'in', coerce to 'tr' just in case */

		SETTEMP(comp_ctx, tr + 1);
		nargs++;

		DUK_DDDPRINT("argument #%d written into reg %d", nargs, tr);
	}

	/* eat the right paren */
	advance_expect(comp_ctx, DUK_TOK_RPAREN);

	DUK_DDDPRINT("end parsing arguments");

	return nargs;
}

static int expr_is_empty(duk_compiler_ctx *comp_ctx) {
	/* empty expressions can be detected conveniently with nud/led counts */
	return (comp_ctx->curr_func.nud_count == 0) &&
	       (comp_ctx->curr_func.led_count == 0);
}

static void expr_nud(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_token *tk;
	int temp_at_entry;
	int tok;
	duk_u32 args;	/* temp variable to pass constants to shared code */

	/*
	 *  ctx->prev_token	token to process with expr_nud()
	 *  ctx->curr_token	updated by caller
	 *
	 *  Note: the token in the switch below has already been eaten.
	 */

	temp_at_entry = GETTEMP(comp_ctx);

	comp_ctx->curr_func.nud_count++;

	tk = &comp_ctx->prev_token;
	tok = tk->t;
	res->t = DUK_IVAL_NONE;

	DUK_DDDPRINT("expr_nud(), prev_token.t=%d, allow_in=%d, paren_level=%d",
	             tk->t, comp_ctx->curr_func.allow_in, comp_ctx->curr_func.paren_level);

	switch (tok) {

	/* PRIMARY EXPRESSIONS */

	case DUK_TOK_THIS: {
		int reg_temp;
		reg_temp = ALLOCTEMP(comp_ctx);
		emit_extraop_b(comp_ctx, DUK_EXTRAOP_LDTHIS, reg_temp);
		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_temp;
		return;
	}
	case DUK_TOK_IDENTIFIER: {
		res->t = DUK_IVAL_VAR;
		res->x1.t = DUK_ISPEC_VALUE;
		duk_push_hstring(ctx, tk->str1);
		duk_replace(ctx, res->x1.valstack_idx);
		return;
	}
	case DUK_TOK_NULL: {
		duk_push_null(ctx);
		goto plain_value;
	}
	case DUK_TOK_TRUE: {
		duk_push_true(ctx);
		goto plain_value;
	}
	case DUK_TOK_FALSE: {
		duk_push_false(ctx);
		goto plain_value;
	}
	case DUK_TOK_NUMBER: {
		duk_push_number(ctx, tk->num);
		goto plain_value;
	}
	case DUK_TOK_STRING: {
		DUK_ASSERT(tk->str1 != NULL);
		duk_push_hstring(ctx, tk->str1);
		goto plain_value;
	}
	case DUK_TOK_REGEXP: {
		int reg_temp;
		int reg_re_bytecode;  /* const */
		int reg_re_source;    /* const */

		DUK_ASSERT(tk->str1 != NULL);
		DUK_ASSERT(tk->str2 != NULL);

		DUK_DDDPRINT("emitting regexp op, str1=%!O, str2=%!O", tk->str1, tk->str2);

		reg_temp = ALLOCTEMP(comp_ctx);
		duk_push_hstring(ctx, tk->str1);
		duk_push_hstring(ctx, tk->str2);

		/* [ ... pattern flags ] */

		duk_regexp_compile(thr);

		/* [ ... escaped_source bytecode ] */

		reg_re_bytecode = getconst(comp_ctx);
		reg_re_source = getconst(comp_ctx);

		emit_a_b_c(comp_ctx,
		           DUK_OP_REGEXP,
		           reg_temp /*a*/,
		           reg_re_bytecode /*b*/,
		           reg_re_source /*c*/);

		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_temp;
		return;
	}
	case DUK_TOK_LBRACKET: {
		DUK_DDDPRINT("parsing array literal");
		nud_array_literal(comp_ctx, res);
		return;
	}
	case DUK_TOK_LCURLY: {
		DUK_DDDPRINT("parsing object literal");
		nud_object_literal(comp_ctx, res);
		return;
	}
	case DUK_TOK_LPAREN: {
		int prev_allow_in;

		comp_ctx->curr_func.paren_level++;
		prev_allow_in = comp_ctx->curr_func.allow_in;
		comp_ctx->curr_func.allow_in = 1; /* reset 'allow_in' for parenthesized expression */

		expr(comp_ctx, res, BP_FOR_EXPR /*rbp*/);  /* Expression, terminates at a ')' */

		advance_expect(comp_ctx, DUK_TOK_RPAREN);
		comp_ctx->curr_func.allow_in = prev_allow_in;
		comp_ctx->curr_func.paren_level--;
		return;
	}

	/* MEMBER/NEW/CALL EXPRESSIONS */

	case DUK_TOK_NEW: {
		/*
		 *  Parsing an expression starting with 'new' is tricky because
		 *  there are multiple possible productions deriving from
		 *  LeftHandSideExpression which begin with 'new'.
		 *
		 *  We currently resort to one-token lookahead to distinguish the
		 *  cases.  Hopefully this is correct.  The binding power must be
		 *  such that parsing ends at an LPAREN (CallExpression) but not at
		 *  a PERIOD or LBRACKET (MemberExpression).
		 *
		 *  See doc/compiler.txt for discussion on the parsing approach,
		 *  and testcases/test-dev-new.js for a bunch of documented tests.
		 */

		int reg_target;
		int nargs;

		DUK_DDDPRINT("begin parsing new expression");

		reg_target = ALLOCTEMP(comp_ctx);
		expr_toforcedreg(comp_ctx, res, BP_CALL /*rbp*/, reg_target /*forced_reg*/);
		SETTEMP(comp_ctx, reg_target + 1);

		if (comp_ctx->curr_token.t == DUK_TOK_LPAREN) {
			/* 'new' MemberExpression Arguments */
			DUK_DDDPRINT("new expression has argument list");
			advance(comp_ctx);
			nargs = parse_arguments(comp_ctx, res);  /* parse args starting from "next temp", reg_target + 1 */
			/* right paren eaten */
		} else {
			/* 'new' MemberExpression */
			DUK_DDDPRINT("new expression has no argument list");
			nargs = 0;
		}

		emit_a_b_c(comp_ctx, DUK_OP_NEW, reg_target /*target*/, reg_target /*start*/, nargs /*num_args*/);

		DUK_DDDPRINT("end parsing new expression");

		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_target;
		return;
	}

	/* FUNCTION EXPRESSIONS */

	case DUK_TOK_FUNCTION: {
		/* Function expression.  Note that any statement beginning with 'function'
		 * is handled by the statement parser as a function declaration, or a
		 * non-standard function expression/statement (or a SyntaxError).  We only
		 * handle actual function expressions (occurring inside an expression) here.
		 */

		int reg_temp;
		int fnum;

		reg_temp = ALLOCTEMP(comp_ctx);

		/* curr_token follows 'function' */
		fnum = parse_function_like_fnum(comp_ctx, 0 /*is_decl*/, 0 /*is_setget*/);
		DUK_DDDPRINT("parsed inner function -> fnum %d", fnum);

		emit_a_bc(comp_ctx, DUK_OP_CLOSURE, reg_temp /*a*/, fnum /*bc*/);

		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_temp;
		return;
	}

	/* UNARY EXPRESSIONS */

	case DUK_TOK_DELETE: {
		/* Delete semantics are a bit tricky.  The description in E5 specification
		 * is kind of confusing, because it distinguishes between resolvability of
		 * a reference (which is only known at runtime) seemingly at compile time
		 * (= SyntaxError throwing).
		 */
		expr(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */
		if (res->t == DUK_IVAL_VAR) {
			/* not allowed in strict mode, regardless of whether resolves;
			 * in non-strict mode DELVAR handles both non-resolving and
			 * resolving cases (the specification description is a bit confusing).
			 */

			int reg_temp;
			int reg_varname;
			int reg_varbind;

			if (comp_ctx->curr_func.is_strict) {
				DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "cannot delete identifier");
			}

			SETTEMP(comp_ctx, temp_at_entry);
			reg_temp = ALLOCTEMP(comp_ctx);

			duk_dup(ctx, res->x1.valstack_idx);
			if (lookup_lhs(comp_ctx, &reg_varbind, &reg_varname)) {
				/* register bound variables are non-configurable -> always false */
				emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_LDBOOL, reg_temp, 0);
			} else {
				duk_dup(ctx, res->x1.valstack_idx);
				reg_varname = getconst(comp_ctx);
				emit_a_b(comp_ctx, DUK_OP_DELVAR, reg_temp, reg_varname);
			}
			res->t = DUK_IVAL_PLAIN;
			res->x1.t = DUK_ISPEC_REGCONST;
			res->x1.regconst = reg_temp;
		} else if (res->t == DUK_IVAL_PROP) {
			int reg_temp;
			int reg_obj;
			int reg_key;

			SETTEMP(comp_ctx, temp_at_entry);
			reg_temp = ALLOCTEMP(comp_ctx);
			reg_obj = ispec_toregconst_raw(comp_ctx, &res->x1, -1 /*forced_reg*/, 0 /*allow_const*/, 0 /*require_temp*/);  /* don't allow const */
			reg_key = ispec_toregconst_raw(comp_ctx, &res->x2, -1 /*forced_reg*/, 1 /*allow_const*/, 0 /*require_temp*/);
			emit_a_b_c(comp_ctx, DUK_OP_DELPROP, reg_temp, reg_obj, reg_key);

			res->t = DUK_IVAL_PLAIN;
			res->x1.t = DUK_ISPEC_REGCONST;
			res->x1.regconst = reg_temp;
		} else {
			/* non-Reference deletion is always 'true', even in strict mode */
			duk_push_true(ctx);
			goto plain_value;
		}
		return;
	}
	case DUK_TOK_VOID: {
		expr_toplain_ignore(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */
		duk_push_undefined(ctx);
		goto plain_value;
	}
	case DUK_TOK_TYPEOF: {
		/* 'typeof' must handle unresolvable references without throwing
		 * a ReferenceError (E5 Section 11.4.3).  Register mapped values
		 * will never be unresolvable so special handling is only required
		 * when an identifier is a "slow path" one.
		 */
		expr(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */

		if (res->t == DUK_IVAL_VAR) {
			int reg_varbind;
			int reg_varname;
			int tr;

			duk_dup(ctx, res->x1.valstack_idx);
			if (!lookup_lhs(comp_ctx, &reg_varbind, &reg_varname)) {
				DUK_DDDPRINT("typeof for an identifier name which could not be resolved "
				             "at compile time, need to use special run-time handling");
				tr = ALLOCTEMP(comp_ctx);
				emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_TYPEOFID, tr, reg_varname);

				res->t = DUK_IVAL_PLAIN;
				res->x1.t = DUK_ISPEC_REGCONST;
				res->x1.regconst = tr;
				return;
			}
		}

		args = (DUK_EXTRAOP_TYPEOF << 8) + 0;
		goto unary_extraop;
	}
	case DUK_TOK_INCREMENT: {
		args = (DUK_OP_INC << 8) + 0;
		goto preincdec;
	}
	case DUK_TOK_DECREMENT: {
		args = (DUK_OP_DEC << 8) + 0;
		goto preincdec;
	}
	case DUK_TOK_ADD: {
		/* unary plus */
		expr(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */
		if (res->t == DUK_IVAL_PLAIN && res->x1.t == DUK_ISPEC_VALUE &&
		    duk_is_number(ctx, res->x1.valstack_idx)) {
			/* unary plus of a number is identity */
			;
		} else {
			args = (DUK_OP_UNP << 8) + 0;
			goto unary;
		}
		return;
	}
	case DUK_TOK_SUB: {
		/* unary minus */
		expr(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */
		if (res->t == DUK_IVAL_PLAIN && res->x1.t == DUK_ISPEC_VALUE &&
		    duk_is_number(ctx, res->x1.valstack_idx)) {
			/* this optimization is important to handle negative literals (which are not directly
			 * provided by the lexical grammar
			 */
			duk_tval *tv_num = duk_get_tval(ctx, res->x1.valstack_idx);
			double d;

			DUK_ASSERT(tv_num != NULL);
			DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_num));
			d = DUK_TVAL_GET_NUMBER(tv_num);
			DUK_TVAL_SET_NUMBER(tv_num, -d);  /* FIXME: OK for NaN, Infinity?  NaN normalization? */
		} else {
			args = (DUK_OP_UNM << 8) + 0;
			goto unary;
		}
		return;
	}
	case DUK_TOK_BNOT: {
		expr(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */
		args = (DUK_OP_BNOT << 8) + 0;
		goto unary;
	}
	case DUK_TOK_LNOT: {
		expr(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */
		args = (DUK_OP_LNOT << 8) + 0;
		goto unary;
	}

	}

	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "unexpected token to expr_nud(): %d", tok);
	return;

 unary:
	{
		/* Note: must coerce to a (writable) temp register, so that e.g. "!x" where x
		 * is a reg-mapped variable works correctly (does not mutate the variable register).
		 */

		int tr;
		tr = ivalue_toregconst_raw(comp_ctx, res, -1 /*forced_reg*/, 0 /*allow_const*/, 1 /*require_temp*/);
		emit_a_b(comp_ctx, args >> 8, tr, tr);
		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = tr;
		return;
	}

 unary_extraop:
	{
		/* FIXME: refactor into unary2: above? */
		int tr;
		tr = ivalue_toregconst_raw(comp_ctx, res, -1 /*forced_reg*/, 0 /*allow_const*/, 1 /*require_temp*/);
		emit_extraop_b_c(comp_ctx, args >> 8, tr, tr);
		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = tr;
		return;
	}

 preincdec:
	{
		/* preincrement and predecrement */
		int reg_res;
		int args_op = args >> 8;

		reg_res = ALLOCTEMP(comp_ctx);

		expr(comp_ctx, res, BP_MULTIPLICATIVE /*rbp*/);  /* UnaryExpression */
		if (res->t == DUK_IVAL_VAR) {
			duk_hstring *h_varname;
			int reg_varbind;
			int reg_varname;

			h_varname = duk_get_hstring(ctx, res->x1.valstack_idx);
			DUK_ASSERT(h_varname != NULL);

			if (hstring_is_eval_or_arguments_in_strict_mode(comp_ctx, h_varname)) {
				goto syntax_error;
			}

			duk_dup(ctx, res->x1.valstack_idx);
			if (lookup_lhs(comp_ctx, &reg_varbind, &reg_varname)) {
				emit_a_b(comp_ctx, args_op, reg_varbind, reg_varbind);
				emit_a_bc(comp_ctx, DUK_OP_LDREG, reg_res, reg_varbind);
			} else {
				emit_a_b(comp_ctx, DUK_OP_GETVAR, reg_res, reg_varname);
				emit_a_b(comp_ctx, args_op, reg_res, reg_res);
				emit_a_b_c(comp_ctx, DUK_OP_PUTVAR, 0, reg_varname, reg_res);
			}

			DUK_DDDPRINT("postincdec to '%!O' -> reg_varbind=%d, reg_varname=%d",
			             h_varname, reg_varbind, reg_varname);
		} else if (res->t == DUK_IVAL_PROP) {
			int reg_obj;  /* allocate to reg only (not const) */
			int reg_key;
			reg_obj = ispec_toregconst_raw(comp_ctx, &res->x1, -1 /*forced_reg*/, 0 /*allow_const*/, 0 /*require_temp*/);
			reg_key = ispec_toregconst_raw(comp_ctx, &res->x2, -1 /*forced_reg*/, 1 /*allow_const*/, 0 /*require_temp*/);
			emit_a_b_c(comp_ctx, DUK_OP_GETPROP, reg_res, reg_obj, reg_key);
			emit_a_b(comp_ctx, args_op, reg_res, reg_res);
			emit_a_b_c(comp_ctx, DUK_OP_PUTPROP, reg_obj, reg_key, reg_res);
		} else {
			/* Technically return value is not needed because INVLHS will
			 * unconditially throw a ReferenceError.  Coercion is necessary
			 * for proper semantics (consider ToNumber() called for an object).
			 */
			ivalue_toforcedreg(comp_ctx, res, reg_res);
			emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_TONUM, reg_res, reg_res);  /* for side effects */
			emit_extraop_only(comp_ctx, DUK_EXTRAOP_INVLHS);
		}
		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_res;
		SETTEMP(comp_ctx, reg_res + 1);
		return;
	}

 plain_value:
	{
		/* Stack top contains plain value */
		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_VALUE;
		duk_replace(ctx, res->x1.valstack_idx);
		return;
	}

 syntax_error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid expression");
}

/* FIXME: add flag to indicate whether caller cares about return value; this
 * affects e.g. handling of assignment expressions.  This change needs API
 * changes elsewhere too.
 */
static void expr_led(duk_compiler_ctx *comp_ctx, duk_ivalue *left, duk_ivalue *res) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_token *tk;
	int tok;
	duk_u32 args;	/* temp variable to pass constants and flags to shared code */

	/*
	 *  ctx->prev_token	token to process with expr_led()
	 *  ctx->curr_token	updated by caller
	 */

	comp_ctx->curr_func.led_count++;

	/* The token in the switch has already been eaten here */
	tk = &comp_ctx->prev_token;
	tok = tk->t;

	DUK_DDDPRINT("expr_led(), prev_token.t=%d, allow_in=%d, paren_level=%d",
	             tk->t, comp_ctx->curr_func.allow_in, comp_ctx->curr_func.paren_level);

	/* FIXME: default priority for infix operators is expr_lbp(tok) -> get it here? */

	switch (tok) {

	/* PRIMARY EXPRESSIONS */

	case DUK_TOK_PERIOD: {
		/* FIXME: this now coerces an identifier into a GETVAR to a temp, which
		 * causes an extra LDREG in call setup.  It's sufficient to coerce to a
		 * unary ivalue?
		 */
		ivalue_toplain(comp_ctx, left);

		/* NB: must accept reserved words as property name */
		if (comp_ctx->curr_token.t_nores != DUK_TOK_IDENTIFIER) {
			DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "expecting identifier name");
		}

		/* FIXME: use "dup+replace" primitive */
		res->t = DUK_IVAL_PROP;
		copy_ispec(comp_ctx, &left->x1, &res->x1);  /* left.x1 -> res.x1 */
		DUK_ASSERT(comp_ctx->curr_token.str1 != NULL);
		duk_push_hstring(ctx, comp_ctx->curr_token.str1);
		duk_replace(ctx, res->x2.valstack_idx);
		res->x2.t = DUK_ISPEC_VALUE;

		/* special RegExp literal handling after IdentifierName */
		comp_ctx->curr_func.reject_regexp_in_adv = 1;

		advance(comp_ctx);
		return;
	}
	case DUK_TOK_LBRACKET: {
		/* FIXME: optimize temp reg use */
		/* FIXME: similar coercion issue as in DUK_TOK_PERIOD */

		ivalue_toplain(comp_ctx, left);

		expr_toplain(comp_ctx, res, BP_FOR_EXPR /*rbp*/);  /* Expression, ']' terminates */

		advance_expect(comp_ctx, DUK_TOK_RBRACKET);

		/* FIXME: coerce to regs? it might be better for enumeration use, where the
		 * same PROP ivalue is used multiple times.  Or perhaps coerce PROP further
		 * there?
		 */

		res->t = DUK_IVAL_PROP;
		copy_ispec(comp_ctx, &res->x1, &res->x2);   /* res.x1 -> res.x2 */
		copy_ispec(comp_ctx, &left->x1, &res->x1);  /* left.x1 -> res.x1 */
		return;
	}
	case DUK_TOK_LPAREN: {
		/* function call */
		int reg_cs = ALLOCTEMPS(comp_ctx, 2);
		int nargs;
		int call_flags = 0;

		/*
		 *  FIXME: attempt to get the call result to "next temp" whenever
		 *  possible to avoid unnecessary register shuffles.
		 *
		 *  FIXME: CSPROP (and CSREG) can overwrite the call target register, and save one temp,
		 *  if the call target is a temporary register and at the top of the temp reg "stack".
		 */

		/*
		 *  Setup call: target and 'this' binding.  Three cases:
		 *
		 *    1. Identifier base (e.g. "foo()")
		 *    2. Property base (e.g. "foo.bar()")
		 *    3. Register base (e.g. "foo()()"; i.e. when a return value is a function)
		 */

		if (left->t == DUK_IVAL_VAR) {
			duk_hstring *h_varname;
			int reg_varname;
			int reg_varbind;

			DUK_DDDPRINT("function call with identifier base");

			h_varname = duk_get_hstring(ctx, left->x1.valstack_idx);
			DUK_ASSERT(h_varname != NULL);
			if (h_varname == DUK_HTHREAD_STRING_EVAL(thr)) {
				/* Potential direct eval call detected, flag the CALL
				 * so that a run-time "direct eval" check is made and
				 * special behavior may be triggered.  Note that this
				 * does not prevent 'eval' from being register bound.
				 */
				DUK_DDDPRINT("function call with identifier 'eval' "
				             "-> enabling EVALCALL flag, marking function "
				             "as may_direct_eval");
				call_flags |= DUK_BC_CALL_FLAG_EVALCALL;

				comp_ctx->curr_func.may_direct_eval = 1;
			}

			duk_dup(ctx, left->x1.valstack_idx);
			if (lookup_lhs(comp_ctx, &reg_varbind, &reg_varname)) {
				emit_a_b(comp_ctx, DUK_OP_CSREG, reg_cs + 0, reg_varbind);
			} else {
				emit_a_b(comp_ctx, DUK_OP_CSVAR, reg_cs + 0, reg_varname);
			}
		} else if (left->t == DUK_IVAL_PROP) {
			DUK_DDDPRINT("function call with property base");
			
			ispec_toforcedreg(comp_ctx, &left->x1, reg_cs + 0);  /* base */
			ispec_toforcedreg(comp_ctx, &left->x2, reg_cs + 1);  /* key */
			emit_a_b_c(comp_ctx, DUK_OP_CSPROP, reg_cs + 0, reg_cs + 0, reg_cs + 1);  /* in-place setup */
		} else {
			DUK_DDDPRINT("function call with register base");

			ivalue_toforcedreg(comp_ctx, left, reg_cs + 0);
			emit_a_b(comp_ctx, DUK_OP_CSREG, reg_cs + 0, reg_cs + 0);  /* in-place setup */
		}

		SETTEMP(comp_ctx, reg_cs + 2);
		nargs = parse_arguments(comp_ctx, res);  /* parse args starting from "next temp" */

		/* FIXME: opcode inconsistency with NEW now, which uses explicit result reg */

		/* Tailcalls are handled by back-patching the TAILCALL flag to the
		 * already emitted instruction later (in return statement parser).
		 */

		emit_a_b_c(comp_ctx, DUK_OP_CALL, call_flags /*flags*/, reg_cs /*basereg*/, nargs /*numargs*/);
		SETTEMP(comp_ctx, reg_cs + 1);    /* result in csreg */

		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_cs;
		return;
	}

	/* POSTFIX EXPRESSION */

	case DUK_TOK_INCREMENT: {
		args = (DUK_OP_INC << 8) + 0;
		goto postincdec;
	}
	case DUK_TOK_DECREMENT: {
		args = (DUK_OP_DEC << 8) + 0;
		goto postincdec;
	}

	/* MULTIPLICATIVE EXPRESSION */

	case DUK_TOK_MUL: {
		args = (DUK_OP_MUL << 8) + BP_MULTIPLICATIVE;  /* UnaryExpression */
		goto binary;
	}
	case DUK_TOK_DIV: {
		args = (DUK_OP_DIV << 8) + BP_MULTIPLICATIVE;  /* UnaryExpression */
		goto binary;
	}
	case DUK_TOK_MOD: {
		args = (DUK_OP_MOD << 8) + BP_MULTIPLICATIVE;  /* UnaryExpression */
		goto binary;
	}

	/* ADDITIVE EXPRESSION */

	case DUK_TOK_ADD: {
		args = (DUK_OP_ADD << 8) + BP_ADDITIVE;  /* MultiplicativeExpression */
		goto binary;
	}
	case DUK_TOK_SUB: {
		args = (DUK_OP_SUB << 8) + BP_ADDITIVE;  /* MultiplicativeExpression */
		goto binary;
	}

	/* SHIFT EXPRESSION */

	case DUK_TOK_ALSHIFT: {
		/* << */
		args = (DUK_OP_BASL << 8) + BP_SHIFT;
		goto binary;
	}
	case DUK_TOK_ARSHIFT: {
		/* >> */
		args = (DUK_OP_BASR << 8) + BP_SHIFT;
		goto binary;
	}
	case DUK_TOK_RSHIFT: {
		/* >>> */
		args = (DUK_OP_BLSR << 8) + BP_SHIFT;
		goto binary;
	}

	/* RELATIONAL EXPRESSION */

	case DUK_TOK_LT: {
		/* < */
		args = (DUK_OP_LT << 8) + BP_RELATIONAL;
		goto binary;
	}
	case DUK_TOK_GT: {
		args = (DUK_OP_GT << 8) + BP_RELATIONAL;
		goto binary;
	}
	case DUK_TOK_LE: {
		args = (DUK_OP_LE << 8) + BP_RELATIONAL;
		goto binary;
	}
	case DUK_TOK_GE: {
		args = (DUK_OP_GE << 8) + BP_RELATIONAL;
		goto binary;
	}
	case DUK_TOK_INSTANCEOF: {
		args = (DUK_OP_INSTOF << 8) + BP_RELATIONAL;
		goto binary;
	}
	case DUK_TOK_IN: {
		args = (DUK_OP_IN << 8) + BP_RELATIONAL;
		goto binary;
	}

	/* EQUALITY EXPRESSION */

	case DUK_TOK_EQ: {
		args = (DUK_OP_EQ << 8) + BP_EQUALITY;
		goto binary;
	}
	case DUK_TOK_NEQ: {
		args = (DUK_OP_NEQ << 8) + BP_EQUALITY;
		goto binary;
	}
	case DUK_TOK_SEQ: {
		args = (DUK_OP_SEQ << 8) + BP_EQUALITY;
		goto binary;
	}
	case DUK_TOK_SNEQ: {
		args = (DUK_OP_SNEQ << 8) + BP_EQUALITY;
		goto binary;
	}

	/* BITWISE EXPRESSIONS */

	case DUK_TOK_BAND: {
		args = (DUK_OP_BAND << 8) + BP_BAND;
		goto binary;
	}
	case DUK_TOK_BXOR: {
		args = (DUK_OP_BXOR << 8) + BP_BXOR;
		goto binary;
	}
	case DUK_TOK_BOR: {
		args = (DUK_OP_BOR << 8) + BP_BOR;
		goto binary;
	}

	/* LOGICAL EXPRESSIONS */

	case DUK_TOK_LAND: {
		/* syntactically left-associative but parsed as right-associative */
		args = (1 << 8) + BP_LAND - 1;
		goto binary_logical;
	}
	case DUK_TOK_LOR: {
		/* syntactically left-associative but parsed as right-associative */
		args = (0 << 8) + BP_LOR - 1;
		goto binary_logical;
	}

	/* CONDITIONAL EXPRESSION */

	case DUK_TOK_QUESTION: {
		/* FIXME: common reg allocation need is to reuse a sub-expression's temp reg,
		 * but only if it really is a temp.  Nothing fancy here now.
		 */
		int reg_temp;
		int pc_jump1;
		int pc_jump2;

		reg_temp = ALLOCTEMP(comp_ctx);
		ivalue_toforcedreg(comp_ctx, left, reg_temp);
		emit_if_true_skip(comp_ctx, reg_temp);
		pc_jump1 = emit_jump_empty(comp_ctx);  /* jump to false */
		expr_toforcedreg(comp_ctx, res, BP_COMMA /*rbp_flags*/, reg_temp /*forced_reg*/);  /* AssignmentExpression */
		advance_expect(comp_ctx, DUK_TOK_COLON);
		pc_jump2 = emit_jump_empty(comp_ctx);  /* jump to end */
		patch_jump_here(comp_ctx, pc_jump1);
		expr_toforcedreg(comp_ctx, res, BP_COMMA /*rbp_flags*/, reg_temp /*forced_reg*/);  /* AssignmentExpression */
		patch_jump_here(comp_ctx, pc_jump2);

		SETTEMP(comp_ctx, reg_temp + 1);
		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_temp;
		return;
	}

	/* ASSIGNMENT EXPRESSION */

	case DUK_TOK_EQUALSIGN: {
		/*
		 *  Assignments are right associative, allows e.g.
		 *    a = 5;
		 *    a += b = 9;   // same as a += (b = 9)
		 *  -> expression value 14, a = 14, b = 9
		 *
		 *  Right associativiness is reflected in the BP for recursion,
		 *  "-1" ensures assignment operations are allowed.
		 *
		 *  FIXME: just use BP_COMMA (i.e. no need for 2-step bp levels)?
		 */
		args = (DUK_OP_INVALID << 8) + BP_ASSIGNMENT - 1;   /* DUK_OP_INVALID marks a 'plain' assignment */
		goto assign;
	}
	case DUK_TOK_ADD_EQ: {
		/* right associative */
		args = (DUK_OP_ADD << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_SUB_EQ: {
		/* right associative */
		args = (DUK_OP_SUB << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_MUL_EQ: {
		/* right associative */
		args = (DUK_OP_MUL << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_DIV_EQ: {
		/* right associative */
		args = (DUK_OP_DIV << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_MOD_EQ: {
		/* right associative */
		args = (DUK_OP_MOD << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_ALSHIFT_EQ: {
		/* right associative */
		args = (DUK_OP_BASL << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_ARSHIFT_EQ: {
		/* right associative */
		args = (DUK_OP_BASR << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_RSHIFT_EQ: {
		/* right associative */
		args = (DUK_OP_BLSR << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_BAND_EQ: {
		/* right associative */
		args = (DUK_OP_BAND << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_BOR_EQ: {
		/* right associative */
		args = (DUK_OP_BOR << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}
	case DUK_TOK_BXOR_EQ: {
		/* right associative */
		args = (DUK_OP_BXOR << 8) + BP_ASSIGNMENT - 1;
		goto assign;
	}

	/* COMMA */

	case DUK_TOK_COMMA: {
		/* right associative */

		ivalue_toplain_ignore(comp_ctx, left);  /* need side effects, not value */
		expr_toplain(comp_ctx, res, BP_COMMA - 1 /*rbp*/);

		/* return 'res' (of right part) as our result */
		return;
	}

	default: {
		break;
	}
	}

	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "unexpected token to expr_led(): %d", tok);
	return;

#if 0
	/* FIXME: shared handling for 'expr_lhs'? */
	if (comp_ctx->curr_func.paren_level == 0 && XXX) {
		comp_ctx->curr_func.expr_lhs = 0;
	}
#endif

 binary:
	/*
	 *  Shared handling of binary operations
	 *
	 *  args = (opcode << 8) + rbp
	 */
	{
		ivalue_toplain(comp_ctx, left);
		expr_toplain(comp_ctx, res, args & 0xff /*rbp*/);

		/* combine left->x1 and res->x1 (right->x1, really) -> (left->x1 OP res->x1) */
		DUK_ASSERT(left->t == DUK_IVAL_PLAIN);
		DUK_ASSERT(res->t == DUK_IVAL_PLAIN);

		res->t = DUK_IVAL_ARITH;
		res->op = args >> 8;

		res->x2.t = res->x1.t;
		res->x2.regconst = res->x1.regconst;
		duk_dup(ctx, res->x1.valstack_idx);
		duk_replace(ctx, res->x2.valstack_idx);

		res->x1.t = left->x1.t;
		res->x1.regconst = left->x1.regconst;
		duk_dup(ctx, left->x1.valstack_idx);
		duk_replace(ctx, res->x1.valstack_idx);

		DUK_DDDPRINT("binary op, res: t=%d, x1.t=%d, x2.t=%d", res->t, res->x1.t, res->x2.t);
		return;
	}

 binary_logical:
	/*
	 *  Shared handling for logical AND and logical OR.
	 *
	 *  args = (truthval << 8) + rbp
	 *
	 *  Truthval determines when to skip right-hand-side.
	 *  For logical AND truthval=1, for logical OR truthval=0.
	 *
	 *  See doc/compiler.txt for discussion on compiling logical
	 *  AND and OR expressions.  The approach here is very simplistic,
	 *  generating extra jumps and multiple evaluations of truth values,
	 *  but generates code on-the-fly with only local back-patching.
	 *
	 *  Both logical AND and OR are syntactically left-associated.
	 *  However, logical ANDs are compiled as right associative
	 *  expressions, i.e. "A && B && C" as "A && (B && C)", to allow
	 *  skip jumps to skip over the entire tail.  Similarly for logical OR.
	 */

	{
		int reg_temp;
		int pc_jump;
		int args_truthval = args >> 8;
		int args_rbp = args & 0xff;

		/* FIXME: unoptimal use of temps, resetting */

		reg_temp = ALLOCTEMP(comp_ctx);

		ivalue_toforcedreg(comp_ctx, left, reg_temp);
		emit_a_b(comp_ctx, DUK_OP_IF, args_truthval, reg_temp);  /* skip jump conditionally */
		pc_jump = emit_jump_empty(comp_ctx);
		expr_toforcedreg(comp_ctx, res, args_rbp /*rbp*/, reg_temp /*forced_reg*/);
		patch_jump_here(comp_ctx, pc_jump);

		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_temp;
		return;
	}

 assign:
	/*
	 *  Shared assignment expression handling
	 *
	 *  args = (opcode << 8) + rbp
	 *
	 *  If 'opcode' is DUK_OP_INVALID, plain assignment without arithmetic.
	 *  Syntactically valid left-hand-side forms which are not accepted as
	 *  left-hand-side values (e.g. as in "f() = 1") must NOT cause a
	 *  SyntaxError, but rather a run-time ReferenceError.
	 */

	{
		int args_op = args >> 8;
		int args_rbp = args & 0xff;

		/* FIXME: here we need to know if 'left' is left-hand-side compatible.
		 * That information is no longer available from current expr parsing
		 * state; it would need to be carried into the 'left' ivalue or by
		 * some other means.
		 */

		if (left->t == DUK_IVAL_VAR) {
			duk_hstring *h_varname;
			int reg_varbind;
			int reg_varname;
			int reg_res;
			int reg_temp;

			/* already in fluly evaluated form */
			DUK_ASSERT(left->x1.t == DUK_ISPEC_VALUE);

			expr_toreg(comp_ctx, res, args_rbp /*rbp*/);
			DUK_ASSERT(res->t == DUK_IVAL_PLAIN && res->x1.t == DUK_ISPEC_REGCONST);

			h_varname = duk_get_hstring(ctx, left->x1.valstack_idx);
			DUK_ASSERT(h_varname != NULL);

			/* E5 Section 11.13.1 (and others for other assignments), step 4 */
			if (hstring_is_eval_or_arguments_in_strict_mode(comp_ctx, h_varname)) {
				goto syntax_error_lvalue;
			}

			duk_dup(ctx, left->x1.valstack_idx);
			(void) lookup_lhs(comp_ctx, &reg_varbind, &reg_varname);

			DUK_DDDPRINT("assign to '%!O' -> reg_varbind=%d, reg_varname=%d",
			             h_varname, reg_varbind, reg_varname);

			if (args_op == DUK_OP_INVALID) {
				reg_res = res->x1.regconst;
			} else {
				reg_temp = ALLOCTEMP(comp_ctx);
				if (reg_varbind >= 0) {
					emit_a_b_c(comp_ctx, args_op, reg_temp, reg_varbind, res->x1.regconst);
				} else {
					emit_a_b(comp_ctx, DUK_OP_GETVAR, reg_temp, reg_varname);
					emit_a_b_c(comp_ctx, args_op, reg_temp, reg_temp, res->x1.regconst);
				}
				reg_res = reg_temp;
			}

			if (reg_varbind >= 0) {
				emit_a_bc(comp_ctx, DUK_OP_LDREG, reg_varbind, reg_res);
			} else {
				emit_a_b_c(comp_ctx, DUK_OP_PUTVAR, 0, reg_varname, reg_res);
			}

			res->t = DUK_IVAL_PLAIN;
			res->x1.t = DUK_ISPEC_REGCONST;
			res->x1.regconst = reg_res;
		} else if (left->t == DUK_IVAL_PROP) {
			/* E5 Section 11.13.1 (and others) step 4 never matches for prop writes -> no check */
			int reg_obj;
			int reg_key;
			int reg_res;
			int reg_temp;

			expr_toregconst(comp_ctx, res, args_rbp /*rbp*/);
			DUK_ASSERT(res->t == DUK_IVAL_PLAIN && res->x1.t == DUK_ISPEC_REGCONST);

			/* Don't allow a constant for the object (even for a number etc), as
			 * it goes into the 'A' field of the opcode.
			 */

			reg_obj = ispec_toregconst_raw(comp_ctx, &left->x1, -1 /*forced_reg*/, 0 /*allow_const*/, 0 /*require_temp*/);
			reg_key = ispec_toregconst_raw(comp_ctx, &left->x2, -1 /*forced_reg*/, 1 /*allow_const*/, 0 /*require_temp*/);
	
			if (args_op == DUK_OP_INVALID) {
				reg_res = res->x1.regconst;
			} else {
				reg_temp = ALLOCTEMP(comp_ctx);
				emit_a_b_c(comp_ctx, DUK_OP_GETPROP, reg_temp, reg_obj, reg_key);
				emit_a_b_c(comp_ctx, args_op, reg_temp, reg_temp, res->x1.regconst);
				reg_res = reg_temp;
			}

			emit_a_b_c(comp_ctx, DUK_OP_PUTPROP, reg_obj, reg_key, reg_res);

			res->t = DUK_IVAL_PLAIN;
			res->x1.t = DUK_ISPEC_REGCONST;
			res->x1.regconst = reg_res;
		} else {
			/* No support for lvalues returned from new or function call expressions.
			 * However, these must NOT cause compile-time SyntaxErrors, but run-time
			 * ReferenceErrors.  Both left and right sides of the assignment must be
			 * evaluated before throwing a ReferenceError.  For instance:
			 *
			 *     f() = g();
			 *
			 * must result in f() being evaluated, then g() being evaluated, and
			 * finally, a ReferenceError being thrown.  See E5 Section 11.13.1.
			 */

			int reg_res;

			/* first evaluate LHS fully to ensure all side effects are out */
			ivalue_toplain_ignore(comp_ctx, left);

			/* then evaluate RHS fully (its value becomes the expression value too) */
			reg_res = expr_toregconst(comp_ctx, res, args_rbp /*rbp*/);
	
			emit_extraop_only(comp_ctx, DUK_EXTRAOP_INVLHS);

			/* FIXME: this value is irrelevant because of INVLHS? */

			res->t = DUK_IVAL_PLAIN;
			res->x1.t = DUK_ISPEC_REGCONST;
			res->x1.regconst = reg_res;
		}

		return;
	}

 postincdec:
	{
		/*
		 *  Post-increment/decrement will return the original value as its
		 *  result value.  However, even that value will be coerced using
		 *  ToNumber().
		 *
		 *  FIXME: the current solution for this is very ugly.
		 *
		 *  Note that post increment/decrement has a "no LineTerminator here"
		 *  restriction.  This is handled by expr_lbp(), which forcibly terminates
		 *  the previous expression if a LineTerminator occurs before '++'/'--'.
		 */

		int reg_res;
		int args_op = args >> 8;

		reg_res = ALLOCTEMP(comp_ctx);

		if (left->t == DUK_IVAL_VAR) {
			duk_hstring *h_varname;
			int reg_varbind;
			int reg_varname;

			h_varname = duk_get_hstring(ctx, left->x1.valstack_idx);
			DUK_ASSERT(h_varname != NULL);

			if (hstring_is_eval_or_arguments_in_strict_mode(comp_ctx, h_varname)) {
				goto syntax_error;
			}

			duk_dup(ctx, left->x1.valstack_idx);
			if (lookup_lhs(comp_ctx, &reg_varbind, &reg_varname)) {
				emit_a_bc(comp_ctx, DUK_OP_LDREG, reg_res, reg_varbind);
				emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_TONUM, reg_res, reg_res);
				emit_a_b(comp_ctx, args_op, reg_varbind, reg_res);
			} else {
				int reg_temp = ALLOCTEMP(comp_ctx);
				emit_a_b(comp_ctx, DUK_OP_GETVAR, reg_res, reg_varname);
				emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_TONUM, reg_res, reg_res);
				emit_a_b(comp_ctx, args_op, reg_temp, reg_res);
				emit_a_b_c(comp_ctx, DUK_OP_PUTVAR, 0, reg_varname, reg_temp);
			}

			DUK_DDDPRINT("postincdec to '%!O' -> reg_varbind=%d, reg_varname=%d",
			             h_varname, reg_varbind, reg_varname);
		} else if (left->t == DUK_IVAL_PROP) {
			int reg_obj;  /* allocate to reg only (not const) */
			int reg_key;
			int reg_temp = ALLOCTEMP(comp_ctx);
			reg_obj = ispec_toregconst_raw(comp_ctx, &left->x1, -1 /*forced_reg*/, 0 /*allow_const*/, 0 /*require_temp*/);
			reg_key = ispec_toregconst_raw(comp_ctx, &left->x2, -1 /*forced_reg*/, 1 /*allow_const*/, 0 /*require_temp*/);
			emit_a_b_c(comp_ctx, DUK_OP_GETPROP, reg_res, reg_obj, reg_key);
			emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_TONUM, reg_res, reg_res);
			emit_a_b(comp_ctx, args_op, reg_temp, reg_res);
			emit_a_b_c(comp_ctx, DUK_OP_PUTPROP, reg_obj, reg_key, reg_temp);
		} else {
			/* Technically return value is not needed because INVLHS will
			 * unconditially throw a ReferenceError.  Coercion is necessary
			 * for proper semantics (consider ToNumber() called for an object).
			 */
			ivalue_toforcedreg(comp_ctx, left, reg_res);
			emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_TONUM, reg_res, reg_res);  /* for side effects */
			emit_extraop_only(comp_ctx, DUK_EXTRAOP_INVLHS);
		}

		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_REGCONST;
		res->x1.regconst = reg_res;
		SETTEMP(comp_ctx, reg_res + 1);
		return;
	}

 syntax_error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid expression");
	return;

 syntax_error_lvalue:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid lvalue");
	return;
}

static int expr_lbp(duk_compiler_ctx *comp_ctx) {
	int tok = comp_ctx->curr_token.t;

	DUK_ASSERT(tok >= DUK_TOK_MINVAL && tok <= DUK_TOK_MAXVAL);
	DUK_ASSERT(sizeof(token_lbp) == DUK_TOK_MAXVAL + 1);

	/* FIXME: integrate support for this into led() instead?
	 * Similar issue as post-increment/post-decrement.
	 */

	/* prevent expr_led() by using a binding power less than anything valid */
	if (tok == DUK_TOK_IN && !comp_ctx->curr_func.allow_in) {
		return 0;
	}

	if ((tok == DUK_TOK_DECREMENT || tok == DUK_TOK_INCREMENT) &&
	    (comp_ctx->curr_token.lineterm)) {
		/* '++' or '--' in a post-increment/decrement position,
		 * and a LineTerminator occurs between the operator and
		 * the preceding expression.  Force the previous expr
		 * to terminate, in effect treating e.g. "a,b\n++" as
		 * "a,b;++" (= SyntaxError).
		 */
		return 0;
	}

	return TOKEN_LBP_GET_BP(token_lbp[tok]);  /* format is bit packed */
}

/*
 *  Expression parsing.
 *
 *  Upon entry to 'expr' and its variants, 'curr_tok' is assumed to be the
 *  first token of the expression.  Upon exit, 'curr_tok' will be the first
 *  token not part of the expression (e.g. semicolon terminating an expression
 *  statement).
 */

#define  EXPR_RBP_MASK           0xff
#define  EXPR_FLAG_REJECT_IN     (1 << 8)
#define  EXPR_FLAG_ALLOW_EMPTY   (1 << 9)

/* main expression parser function */
static void expr(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_ivalue tmp_alloc;   /* 'res' is used for "left", and 'tmp' for "right" */
	duk_ivalue *tmp = &tmp_alloc;

	RECURSION_INCREASE(comp_ctx, thr);

	/* filter out flags from exprtop rbp_flags here to save space */
	rbp = rbp & EXPR_RBP_MASK;

	DUK_DDDPRINT("expr(), rbp=%d, allow_in=%d, paren_level=%d",
	             rbp, comp_ctx->curr_func.allow_in, comp_ctx->curr_func.paren_level);

	memset(&tmp_alloc, 0, sizeof(tmp_alloc));
	tmp->x1.valstack_idx = duk_get_top(ctx);
	tmp->x2.valstack_idx = tmp->x1.valstack_idx + 1;
	duk_push_undefined(ctx);
	duk_push_undefined(ctx);

	/* FIXME: where to release temp regs in intermediate expressions?
	 * e.g. 1+2+3 -> don't inflate temp register count when parsing this.
	 * that particular expression temp regs can be forced here.
	 */

	/* FIXME: increase ctx->expr_tokens here for every consumed token
	 * (this would be a nice statistic)?
	 */

	if (comp_ctx->curr_token.t == DUK_TOK_SEMICOLON || comp_ctx->curr_token.t == DUK_TOK_RPAREN) {
		/* FIXME: incorrect hack for testing */
		DUK_DDDPRINT("empty expression");
		res->t = DUK_IVAL_PLAIN;
		res->x1.t = DUK_ISPEC_VALUE;
		duk_push_undefined(ctx);
		duk_replace(ctx, res->x1.valstack_idx);
		goto cleanup;
	}

	advance(comp_ctx);
	expr_nud(comp_ctx, res);  /* reuse 'res' as 'left' */
	while (rbp < expr_lbp(comp_ctx)) {
		advance(comp_ctx);
		expr_led(comp_ctx, res, tmp);
		copy_ivalue(comp_ctx, tmp, res);  /* tmp -> res */
	}

 cleanup:
	/* final result is already in 'res' */

	duk_pop_2(ctx);

	RECURSION_DECREASE(comp_ctx, thr);
}

static void exprtop(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags) {
	duk_hthread *thr = comp_ctx->thr;

	/* Note: these variables must reside in 'curr_func' instead of the global
	 * context: when parsing function expressions, expression parsing is nested.
	 */
	comp_ctx->curr_func.nud_count = 0;
	comp_ctx->curr_func.led_count = 0;
	comp_ctx->curr_func.paren_level = 0;
	comp_ctx->curr_func.expr_lhs = 1;
	comp_ctx->curr_func.allow_in = (rbp_flags & EXPR_FLAG_REJECT_IN ? 0 : 1);

	expr(comp_ctx, res, rbp_flags);

	if (!(rbp_flags & EXPR_FLAG_ALLOW_EMPTY) && expr_is_empty(comp_ctx)) {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "empty expression not allowed");
	}
}

/* A bunch of helpers (for size optimization) that combine expr()/exprtop()
 * and result conversions.
 *
 * Each helper needs at least 2-3 calls to make it worth while to wrap.
 */

static int expr_toreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp) {
	expr(comp_ctx, res, rbp);
	return ivalue_toreg(comp_ctx, res);
}

#if 0  /* unused */
static int expr_totempreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp) {
	expr(comp_ctx, res, rbp);
	return ivalue_totempreg(comp_ctx, res);
}
#endif

static int expr_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp, int forced_reg) {
	expr(comp_ctx, res, rbp);
	return ivalue_toforcedreg(comp_ctx, res, forced_reg);
}

static int expr_toregconst(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp) {
	expr(comp_ctx, res, rbp);
	return ivalue_toregconst(comp_ctx, res);
}

static void expr_toplain(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp) {
	expr(comp_ctx, res, rbp);
	ivalue_toplain(comp_ctx, res);
}

static void expr_toplain_ignore(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp) {
	expr(comp_ctx, res, rbp);
	ivalue_toplain_ignore(comp_ctx, res);
}

static int exprtop_toreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags) {
	exprtop(comp_ctx, res, rbp_flags);
	return ivalue_toreg(comp_ctx, res);
}

#if 0  /* unused */
static int exprtop_totempreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags) {
	exprtop(comp_ctx, res, rbp_flags);
	return ivalue_totempreg(comp_ctx, res);
}
#endif

#if 0  /* unused */
static int exprtop_toforcedreg(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags, int forced_reg) {
	exprtop(comp_ctx, res, rbp_flags);
	return ivalue_toforcedreg(comp_ctx, res, forced_reg);
}
#endif

static int exprtop_toregconst(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags) {
	exprtop(comp_ctx, res, rbp_flags);
	return ivalue_toregconst(comp_ctx, res);
}

#if 0  /* unused */
static void exprtop_toplain_ignore(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int rbp_flags) {
	exprtop(comp_ctx, res, rbp_flags);
	ivalue_toplain_ignore(comp_ctx, res);
}
#endif

/*
 *  Parse an individual source element (top level statement) or a statement.
 *
 *  Handles labeled statements automatically (peeling away labels before
 *  parsing an expression that follows the label(s)).
 *
 *  Upon entry, 'curr_tok' contains the first token of the statement (parsed
 *  in "allow regexp literal" mode).  Upon exit, 'curr_tok' contains the first
 *  token following the statement (if the statement has a terminator, this is
 *  the token after the terminator).
 */

#ifdef HAS_VAL
#undef HAS_VAL
#endif
#ifdef HAS_TERM
#undef HAS_TERM
#endif
#ifdef ALLOW_AUTO_SEMI_ALWAYS
#undef ALLOW_AUTO_SEMI_ALWAYS
#endif
#ifdef STILL_PROLOGUE
#undef STILL_PROLOGUE
#endif
#ifdef IS_TERMINAL
#undef IS_TERMINAL
#endif

#define  HAS_VAL                  (1 << 0)  /* stmt has non-empty value */
#define  HAS_TERM                 (1 << 1)  /* stmt has explicit/implicit semicolon terminator */
#define  ALLOW_AUTO_SEMI_ALWAYS   (1 << 2)  /* allow automatic semicolon even without lineterm (compatibility) */
#define  STILL_PROLOGUE           (1 << 3)  /* statement does not terminate directive prologue */
#define  IS_TERMINAL              (1 << 4)  /* statement is guaranteed to be terminal (control doesn't flow to next statement) */

/* Parse a single variable declaration (e.g. "i" or "i=10").  A leading 'var'
 * has already been eaten.  These is no return value in 'res', it is used only
 * as a temporary.
 *
 * When called from 'for-in' statement parser, the initializer expression must
 * not allow the 'in' token.  The caller supply additional expression parsing
 * flags (like EXPR_FLAG_REJECT_IN) in 'expr_flags'.
 *
 * Finally, out_reg_varname and out_reg_varbind are updated to reflect where
 * the identifier is bound:
 *
 *    If register bound:      out_reg_varbind >= 0, out_reg_varname < 0
 *    If not register bound:  out_reg_varbind < 0, out_reg_varname >= 0
 *
 * These allow the caller to use the variable for further assignment, e.g.
 * as is done in 'for-in' parsing.
 */

static void parse_variable_declaration(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int expr_flags, int *out_reg_varname, int *out_reg_varbind) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *h_varname;
	int reg_varname;
	int reg_varbind;

	/* assume 'var' has been eaten */

	/* Note: Identifier rejects reserved words */
	if (comp_ctx->curr_token.t != DUK_TOK_IDENTIFIER) {
		goto syntax_error;
	}
	h_varname = comp_ctx->curr_token.str1;

	DUK_ASSERT(h_varname != NULL);

	/* strict mode restrictions (E5 Section 12.2.1) */
	if (hstring_is_eval_or_arguments_in_strict_mode(comp_ctx, h_varname)) {
		goto syntax_error;
	}

	/* register declarations in first pass */
	if (comp_ctx->curr_func.in_scanning) {
		int n;
		DUK_DDDPRINT("register variable declaration %!O in pass 1", h_varname);
		n = duk_get_length(ctx, comp_ctx->curr_func.decls_idx);  /*FIXME: primitive for pushing*/
		duk_push_hstring(ctx, h_varname);
		duk_put_prop_index(ctx, comp_ctx->curr_func.decls_idx, n);
		duk_push_int(ctx, DUK_DECL_TYPE_VAR + (0 << 8));
		duk_put_prop_index(ctx, comp_ctx->curr_func.decls_idx, n + 1);
	}

	duk_push_hstring(ctx, h_varname);  /* push before advancing to keep reachable */

	/* register binding lookup is based on varmap (even in first pass) */
	duk_dup_top(ctx);
	(void) lookup_lhs(comp_ctx, &reg_varbind, &reg_varname);

	advance(comp_ctx);  /* eat identifier */

	if (comp_ctx->curr_token.t == DUK_TOK_EQUALSIGN) {
		advance(comp_ctx);

		DUK_DDDPRINT("vardecl, assign to '%!O' -> reg_varbind=%d, reg_varname=%d",
		             h_varname, reg_varbind, reg_varname);

		exprtop(comp_ctx, res, BP_COMMA | expr_flags /*rbp_flags*/);  /* AssignmentExpression */

		if (reg_varbind >= 0) {
			ivalue_toforcedreg(comp_ctx, res, reg_varbind);
		} else {
			int reg_val;
			reg_val = ivalue_toreg(comp_ctx, res);
			emit_a_b_c(comp_ctx, DUK_OP_PUTVAR, 0, reg_varname, reg_val);
		}
	}

	duk_pop(ctx);  /* pop varname */

	*out_reg_varname = reg_varname;
	*out_reg_varbind = reg_varbind;

	return;

 syntax_error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid variable declaration");
}

static void parse_var_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	int reg_varname;
	int reg_varbind;

	advance(comp_ctx);  /* eat 'var' */

	for (;;) {
		/* reg_varname and reg_varbind are ignored here */
		parse_variable_declaration(comp_ctx, res, 0, &reg_varname, &reg_varbind);

		if (comp_ctx->curr_token.t != DUK_TOK_COMMA) {
			break;
		}
		advance(comp_ctx);
	} 
}

static void parse_for_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int pc_v34_lhs;    /* start variant 3/4 left-hand-side code (L1 in doc/compiler.txt example) */
	int temp_reset;    /* knock back "next temp" to this whenever possible */
	int reg_temps;     /* preallocated temporaries (2) for variants 3 and 4 */

	DUK_DDDPRINT("start parsing a for/for-in statement");

	/* Two temporaries are preallocated here for variants 3 and 4 which need
	 * registers which are never clobbered by expressions in the loop
	 * (concretely: for the enumerator object and the next enumerated value).
	 * Variants 1 and 2 "release" these temps.
	 */

	reg_temps = ALLOCTEMPS(comp_ctx, 2);

	temp_reset = GETTEMP(comp_ctx);

	/*
	 *  For/for-in main variants are:
	 *
	 *    1. for (ExpressionNoIn_opt; Expression_opt; Expression_opt) Statement
	 *    2. for (var VariableDeclarationNoIn; Expression_opt; Expression_opt) Statement
	 *    3. for (LeftHandSideExpression in Expression) Statement
	 *    4. for (var VariableDeclarationNoIn in Expression) Statement
	 *
	 *  Parsing these without arbitrary lookahead or backtracking is relatively
	 *  tricky but we manage to do so for now.
	 *
	 *  See doc/compiler.txt for a detailed discussion of control flow
	 *  issues, evaluation order issues, etc.
	 */
	
	advance(comp_ctx);  /* eat 'for' */
	advance_expect(comp_ctx, DUK_TOK_LPAREN);

	DUK_DDDPRINT("detecting for/for-in loop variant, pc=%d", get_current_pc(comp_ctx));

	/* a label site has been emitted by parse_statement() automatically
	 * (it will also emit the ENDLABEL).
	 */

	if (comp_ctx->curr_token.t == DUK_TOK_VAR) {
		/*
		 *  Variant 2 or 4
		 */

		int reg_varname;  /* variable name reg/const, if variable not register-bound */
		int reg_varbind;  /* variable binding register if register-bound (otherwise < 0) */

		advance(comp_ctx);  /* eat 'var' */
		parse_variable_declaration(comp_ctx, res, EXPR_FLAG_REJECT_IN, &reg_varname, &reg_varbind);
		SETTEMP(comp_ctx, temp_reset);

		if (comp_ctx->curr_token.t == DUK_TOK_IN) {
			/*
			 *  Variant 4
			 */

			DUK_DDDPRINT("detected for variant 4: for (var VariableDeclarationNoIn in Expression) Statement");
			pc_v34_lhs = get_current_pc(comp_ctx);  /* jump is inserted here */
			if (reg_varbind >= 0) {
				emit_a_bc(comp_ctx, DUK_OP_LDREG, reg_varbind, reg_temps + 0);
			} else {
				emit_a_b_c(comp_ctx, DUK_OP_PUTVAR, 0, reg_varname, reg_temps + 0);
			}
			goto parse_3_or_4;
		} else {
			/*
			 *  Variant 2
			 */

			DUK_DDDPRINT("detected for variant 2: for (var VariableDeclarationNoIn; Expression_opt; Expression_opt) Statement");
			for (;;) {
				/* more initializers */
				if (comp_ctx->curr_token.t != DUK_TOK_COMMA) {
					break;
				}
				DUK_DDDPRINT("variant 2 has another variable initializer");

				parse_variable_declaration(comp_ctx, res, EXPR_FLAG_REJECT_IN, &reg_varname, &reg_varbind);
			}
			goto parse_1_or_2;
		}
	} else {
		/*
		 *  Variant 1 or 3
		 */

		pc_v34_lhs = get_current_pc(comp_ctx);  /* jump is inserted here (variant 3) */

		/* FIXME: note that exprtop() here can clobber any reg above current temp_next,
		 * so any loop variables (e.g. enumerator) must be *preallocated* ... */

		/* don't coerce yet to a plain value (variant 3 needs special handling) */
		exprtop(comp_ctx, res, BP_FOR_EXPR | EXPR_FLAG_REJECT_IN | EXPR_FLAG_ALLOW_EMPTY /*rbp_flags*/);  /* Expression */
		if (comp_ctx->curr_token.t == DUK_TOK_IN) {
			/*
			 *  Variant 3
			 */

			/* FIXME: need to determine LHS type, and check that it is LHS compatible */
			DUK_DDDPRINT("detected for variant 3: for (LeftHandSideExpression in Expression) Statement");
			if (expr_is_empty(comp_ctx)) {
				goto syntax_error;  /* LeftHandSideExpression does not allow empty expression */
			}

			if (res->t == DUK_IVAL_VAR) {
				int reg_varname;
				int reg_varbind;

				duk_dup(ctx, res->x1.valstack_idx);
				if (lookup_lhs(comp_ctx, &reg_varbind, &reg_varname)) {
					emit_a_bc(comp_ctx, DUK_OP_LDREG, reg_varbind, reg_temps + 0);
				} else {
					emit_a_b_c(comp_ctx, DUK_OP_PUTVAR, 0, reg_varname, reg_temps + 0);
				}
			} else if (res->t == DUK_IVAL_PROP) {
				/* Don't allow a constant for the object (even for a number etc), as
				 * it goes into the 'A' field of the opcode.
				 */
				int reg_obj;
				int reg_key;
				reg_obj = ispec_toregconst_raw(comp_ctx, &res->x1, -1 /*forced_reg*/, 0 /*allow_const*/, 0 /*require_temp*/);
				reg_key = ispec_toregconst_raw(comp_ctx, &res->x2, -1 /*forced_reg*/, 1 /*allow_const*/, 0 /*require_temp*/);
				emit_a_b_c(comp_ctx, DUK_OP_PUTPROP, reg_obj, reg_key, reg_temps + 0);
			} else {
				ivalue_toplain_ignore(comp_ctx, res);  /* just in case */
				emit_extraop_only(comp_ctx, DUK_EXTRAOP_INVLHS);
			}
			goto parse_3_or_4;
		} else {
			/*
			 *  Variant 1
			 */

			DUK_DDDPRINT("detected for variant 1: for (ExpressionNoIn_opt; Expression_opt; Expression_opt) Statement");
			ivalue_toplain_ignore(comp_ctx, res);
			goto parse_1_or_2;
		}
	}

 parse_1_or_2:
	/*
	 *  Parse variant 1 or 2.  The first part expression (which differs
	 *  in the variants) has already been parsed and its code emitted.
	 *
	 *  reg_temps + 0: unused
	 *  reg_temps + 1: unused
	 */
	{
		int reg_cond;
		int pc_l1, pc_l2, pc_l3, pc_l4;
		int pc_jumpto_l3, pc_jumpto_l4;
		int expr_c_empty;

		DUK_DDDPRINT("shared code for parsing variants 1 and 2");

		/* "release" preallocated temps since we won't need them */
		temp_reset = reg_temps + 0;
		SETTEMP(comp_ctx, temp_reset);

		advance_expect(comp_ctx, DUK_TOK_SEMICOLON);

		pc_l1 = get_current_pc(comp_ctx);
		exprtop(comp_ctx, res, BP_FOR_EXPR | EXPR_FLAG_ALLOW_EMPTY /*rbp_flags*/);  /* Expression_opt */
		if (expr_is_empty(comp_ctx)) {
			/* no need to coerce */
			pc_jumpto_l3 = emit_jump_empty(comp_ctx);  /* to body */
			pc_jumpto_l4 = -1;  /* omitted */
		} else {
			reg_cond = ivalue_toregconst(comp_ctx, res);
			emit_if_false_skip(comp_ctx, reg_cond);
			pc_jumpto_l3 = emit_jump_empty(comp_ctx);  /* to body */
			pc_jumpto_l4 = emit_jump_empty(comp_ctx);  /* to exit */
		}
		SETTEMP(comp_ctx, temp_reset);

		advance_expect(comp_ctx, DUK_TOK_SEMICOLON);

		pc_l2 = get_current_pc(comp_ctx);
		exprtop(comp_ctx, res, BP_FOR_EXPR | EXPR_FLAG_ALLOW_EMPTY /*rbp_flags*/);  /* Expression_opt */
		if (expr_is_empty(comp_ctx)) {
			/* no need to coerce */
			expr_c_empty = 1;
			/* JUMP L1 omitted */
		} else {
			ivalue_toplain_ignore(comp_ctx, res);
			expr_c_empty = 0;
			emit_jump(comp_ctx, pc_l1);
		}
		SETTEMP(comp_ctx, temp_reset);

		advance_expect(comp_ctx, DUK_TOK_RPAREN);

		pc_l3 = get_current_pc(comp_ctx);
		parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);
		if (expr_c_empty) {
			emit_jump(comp_ctx, pc_l1);
		} else {
			emit_jump(comp_ctx, pc_l2);
		}
		/* temp reset is not necessary after parse_statement(), which already does it */

		pc_l4 = get_current_pc(comp_ctx);

		DUK_DDDPRINT("patching jumps: jumpto_l3: %d->%d, jumpto_l4: %d->%d, "
		             "break: %d->%d, continue: %d->%d",
			     pc_jumpto_l3, pc_l3, pc_jumpto_l4, pc_l4,
		             pc_label_site + 1, pc_l4, pc_label_site + 2, pc_l2);

		patch_jump(comp_ctx, pc_jumpto_l3, pc_l3);
		patch_jump(comp_ctx, pc_jumpto_l4, pc_l4);
		patch_jump(comp_ctx, pc_label_site + 1, pc_l4);  /* break jump */
		patch_jump(comp_ctx, pc_label_site + 2, pc_l2);  /* continue jump */
	}
	goto finished;

 parse_3_or_4:
	/*
	 *  Parse variant 3 or 4.
	 *
	 *  For variant 3 (e.g. "for (A in C) D;") the code for A (except the
	 *  final property/variable write) has already been emitted.  The first
	 *  instruction of that code is at pc_v34_lhs; a JUMP needs to be inserted
	 *  there to satisfy control flow needs.
	 *
	 *  For variant 4, if the variable declaration had an initializer
	 *  (e.g. "for (var A = B in C) D;") the code for the assignment
	 *  (B) has already been emitted.
	 *
	 *  Variables set before entering here:
	 *
	 *    pc_v34_lhs:    insert a "JUMP L2" here (see doc/compiler.txt example).
	 *    reg_temps + 0: iteration target value (written to LHS)
	 *    reg_temps + 1: enumerator object
	 */
	{
		int pc_l1, pc_l2, pc_l3, pc_l4, pc_l5;
		int pc_jumpto_l2, pc_jumpto_l3, pc_jumpto_l4, pc_jumpto_l5;
		int reg_target;

		DUK_DDDPRINT("shared code for parsing variants 3 and 4, pc_v34_lhs=%d", pc_v34_lhs);

		SETTEMP(comp_ctx, temp_reset);

		/* First we need to insert a jump in the middle of previously
		 * emitted code to get the control flow right.  No jumps can
		 * cross the position where the jump is inserted.  See doc/compiler.txt
		 * for discussion on the intricacies of control flow and side effects
		 * for variants 3 and 4.
		 */

		insert_jump_empty(comp_ctx, pc_v34_lhs);
		pc_jumpto_l2 = pc_v34_lhs;  /* inserted jump */
		pc_l1 = pc_v34_lhs + 1;     /* +1, right after inserted jump */

		/* The code for writing reg_temps + 0 to the left hand side has already
		 * been emitted.
		 */

		pc_jumpto_l3 = emit_jump_empty(comp_ctx);  /* -> loop body */

		advance(comp_ctx);  /* eat 'in' */

		/* Parse enumeration target and initialize enumerator.  For 'null' and 'undefined',
		 * INITENUM will creates a 'null' enumerator which works like an empty enumerator
		 * (E5 Section 12.6.4, step 3).  Note that INITENUM requires the value to be in a
		 * register (constant not allowed).
	 	 */

		pc_l2 = get_current_pc(comp_ctx);
		reg_target = exprtop_toreg(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);  /* Expression */
		emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_INITENUM, reg_temps + 1, reg_target);
		pc_jumpto_l4 = emit_jump_empty(comp_ctx);
		SETTEMP(comp_ctx, temp_reset);

		advance_expect(comp_ctx, DUK_TOK_RPAREN);

		pc_l3 = get_current_pc(comp_ctx);
		parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);
		/* temp reset is not necessary after parse_statement(), which already does it */

		pc_l4 = get_current_pc(comp_ctx);
		emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_NEXTENUM, reg_temps + 0, reg_temps + 1);
		pc_jumpto_l5 = emit_jump_empty(comp_ctx);  /* NEXTENUM jump slot: executed when enum finished */
		emit_jump(comp_ctx, pc_l1);  /* jump to next loop, using reg_v34_iter as iterated value */

		pc_l5 = get_current_pc(comp_ctx);

		/* XXX: since the enumerator may be a memory expensive object,
		 * perhaps clear it explicitly here?  If so, break jump must
		 * go through this clearing operation.
		 */

		DUK_DDDPRINT("patching jumps: jumpto_l2: %d->%d, jumpto_l3: %d->%d, "
			     "jumpto_l4: %d->%d, jumpto_l5: %d->%d, "
		             "break: %d->%d, continue: %d->%d",
			     pc_jumpto_l2, pc_l2, pc_jumpto_l3, pc_l3,
			     pc_jumpto_l4, pc_l4, pc_jumpto_l5, pc_l5,
		             pc_label_site + 1, pc_l5, pc_label_site + 2, pc_l4);

		patch_jump(comp_ctx, pc_jumpto_l2, pc_l2);
		patch_jump(comp_ctx, pc_jumpto_l3, pc_l3);
		patch_jump(comp_ctx, pc_jumpto_l4, pc_l4);
		patch_jump(comp_ctx, pc_jumpto_l5, pc_l5);
		patch_jump(comp_ctx, pc_label_site + 1, pc_l5);  /* break jump */
		patch_jump(comp_ctx, pc_label_site + 2, pc_l4);  /* continue jump */
	}
	goto finished;

 finished:
	DUK_DDDPRINT("end parsing a for/for-in statement");
	return;

 syntax_error:		
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid for statement");
}

static void parse_switch_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site) {
	duk_hthread *thr = comp_ctx->thr;
	int temp_at_loop;
	int reg_switch;        /* reg/const for switch value */
	int reg_case;          /* reg/const for case value */
	int reg_temp;          /* general temp register */
	int pc_prevcase = -1;
	int pc_prevstmt = -1;
	int pc_default = -1;   /* -1 == not set, -2 == pending (next statement list) */

	/* Note: negative pc values are ignored when patching jumps, so no explicit checks needed */

	/*
	 *  Switch is pretty complicated because of several conflicting concerns:
	 *
	 *    - Want to generate code without an intermediate representation,
	 *      i.e., in one go
	 *
	 *    - Case selectors are expressions, not values, and may thus e.g. throw
	 *      exceptions (which causes evaluation order concerns)
	 *
	 *    - Evaluation semantics of case selectors and default clause need to be 
	 *      carefully implemented to provide correct behavior even with case value
	 *      side effects
	 *
	 *    - Fall through case and default clauses; avoiding dead JUMPs if case
	 *      ends with an unconditional jump (a break or a continue)
	 *
	 *    - The same case value may occur multiple times, but evaluation rules
	 *      only process the first match before switching to a "propagation" mode
	 *      where case values are no longer evaluated
	 *
	 *  See E5 Section 12.11.  Also see doc/compiler.txt for compilation
	 *  discussion.
	 */

	advance(comp_ctx);
	advance_expect(comp_ctx, DUK_TOK_LPAREN);
	reg_switch = exprtop_toregconst(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
	advance_expect(comp_ctx, DUK_TOK_RPAREN);
	advance_expect(comp_ctx, DUK_TOK_LCURLY);

	DUK_DDDPRINT("switch value in register %d", reg_switch);

	temp_at_loop = GETTEMP(comp_ctx);

	for (;;) {
		/* sufficient for keeping temp reg numbers in check */
		SETTEMP(comp_ctx, temp_at_loop);

		if (comp_ctx->curr_token.t == DUK_TOK_RCURLY) {
			break;
		}

		if (comp_ctx->curr_token.t == DUK_TOK_CASE) {
			/*
			 *  Case clause.
			 *
			 *  Note: cannot use reg_case as a temp register (for SEQ target)
			 *  because it may be a constant.
			 */

			patch_jump_here(comp_ctx, pc_prevcase);  /* chain jumps for case
			                                          * evaluation and checking
			                                          */

			advance(comp_ctx);
			reg_case = exprtop_toregconst(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
			advance_expect(comp_ctx, DUK_TOK_COLON);

			reg_temp = ALLOCTEMP(comp_ctx);
			emit_a_b_c(comp_ctx, DUK_OP_SEQ, reg_temp, reg_switch, reg_case);
			emit_if_true_skip(comp_ctx, reg_temp);

			/* jump to next case clause */
			pc_prevcase = emit_jump_empty(comp_ctx);  /* no match, next case */

			/* statements go here (if any) on next loop */
		} else if (comp_ctx->curr_token.t == DUK_TOK_DEFAULT) {
			/*
			 *  Default clause.
			 */

			if (pc_default >= 0) {
				goto syntax_error;
			}
			advance(comp_ctx);
			advance_expect(comp_ctx, DUK_TOK_COLON);

			/* default clause matches next statement list (if any) */
			pc_default = -2;
		} else {
			/*
			 *  Else must be a statement list, possible terminators are
			 *  'case', 'default', and '}'.
			 */

			int num_stmts = 0;
			int tok;

			if (pc_default == -2) {
				pc_default = get_current_pc(comp_ctx);
			}

			/* Note: this is correct even for default clause statements:
			 * they participate in 'fall-through' behavior even if the
			 * default clause is in the middle.
			 */
			patch_jump_here(comp_ctx, pc_prevstmt);  /* chain jumps for 'fall-through'
			                                          * after a case matches.
			                                          */

			for (;;) {
				tok = comp_ctx->curr_token.t;
				if (tok == DUK_TOK_CASE || tok == DUK_TOK_DEFAULT ||
				    tok == DUK_TOK_RCURLY) {
					break;
				}
				num_stmts++;
				parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);
			}

			/* fall-through jump to next code of next case (backpatched) */
			pc_prevstmt = emit_jump_empty(comp_ctx);

			/* FIXME: would be nice to omit this jump when the jump is not
			 * reachable, at least in the obvious cases (such as the case
			 * ending with a 'break'.
			 *
			 * Perhaps parse_statement() could provide some info on whether
			 * the statement is a "dead end"?
			 *
			 * If implemented, just set pc_prevstmt to -1 when not needed.
			 */
		}
	}

	DUK_ASSERT(comp_ctx->curr_token.t == DUK_TOK_RCURLY);
	advance(comp_ctx);

	/* default case control flow patchup; note that if pc_prevcase < 0
	 * (i.e. no case clauses), control enters default case automatically.
	 */
	if (pc_default >= 0) {
		/* default case exists: go there if no case matches */
		patch_jump(comp_ctx, pc_prevcase, pc_default);
	} else {
		/* default case does not exist, or no statements present
		 * after default case: finish case evaluation
		 */
		patch_jump_here(comp_ctx, pc_prevcase);
	}

	/* fall-through control flow patchup; note that pc_prevstmt may be
	 * < 0 (i.e. no case clauses), in which case this is a no-op.
	 */
	patch_jump_here(comp_ctx, pc_prevstmt);

	/* continue jump not patched, an INVALID opcode remains there */
	patch_jump_here(comp_ctx, pc_label_site + 1);  /* break jump */

	/* Note: 'fast' breaks will jump to pc_label_site + 1, which will
	 * then jump here.  The double jump will be eliminated by a
	 * peephole pass, resulting in an optimal jump here.  The label
	 * site jumps will remain in bytecode and will waste code size.
	 */

	return;

 syntax_error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid switch statement");
}

static void parse_if_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	int temp_reset;
	int reg_cond;
	int pc_jump_false;

	DUK_DDDPRINT("begin parsing if statement");

	temp_reset = GETTEMP(comp_ctx);

	advance(comp_ctx);  /* eat 'if' */
	advance_expect(comp_ctx, DUK_TOK_LPAREN);

	reg_cond = exprtop_toregconst(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
	emit_if_true_skip(comp_ctx, reg_cond);
	pc_jump_false = emit_jump_empty(comp_ctx);  /* jump to end or else part */
	SETTEMP(comp_ctx, temp_reset);

	advance_expect(comp_ctx, DUK_TOK_RPAREN);

	parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);

	/* The 'else' ambiguity is resolved by 'else' binding to the innermost
	 * construct, so greedy matching is correct here.
	 */

	if (comp_ctx->curr_token.t == DUK_TOK_ELSE) {
		int pc_jump_end;

		DUK_DDDPRINT("if has else part");

		advance(comp_ctx);

		pc_jump_end = emit_jump_empty(comp_ctx);  /* jump from true part to end */
		patch_jump_here(comp_ctx, pc_jump_false);

		parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);

		patch_jump_here(comp_ctx, pc_jump_end);
	} else {
		DUK_DDDPRINT("if does not have else part");

		patch_jump_here(comp_ctx, pc_jump_false);
	}

	DUK_DDDPRINT("end parsing if statement");
}

static void parse_do_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site) {
	int reg_cond;
	int pc_start;

	DUK_DDDPRINT("begin parsing do statement");

	advance(comp_ctx);  /* eat 'do' */

	pc_start = get_current_pc(comp_ctx);
	patch_jump_here(comp_ctx, pc_label_site + 2);  /* continue jump */
	parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);

	advance_expect(comp_ctx, DUK_TOK_WHILE);
	advance_expect(comp_ctx, DUK_TOK_LPAREN);

	reg_cond = exprtop_toregconst(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
	emit_if_false_skip(comp_ctx, reg_cond);
	emit_jump(comp_ctx, pc_start);
	/* no need to reset temps, as we're finished emitting code */

	advance_expect(comp_ctx, DUK_TOK_RPAREN);

	patch_jump_here(comp_ctx, pc_label_site + 1);  /* break jump */

	DUK_DDDPRINT("end parsing do statement");
}

static void parse_while_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int pc_label_site) {
	int temp_reset;
	int reg_cond;
	int pc_start;
	int pc_jump_false;

	DUK_DDDPRINT("begin parsing while statement");

	temp_reset = GETTEMP(comp_ctx);

	advance(comp_ctx);  /* eat 'while' */

	advance_expect(comp_ctx, DUK_TOK_LPAREN);

	pc_start = get_current_pc(comp_ctx);
	patch_jump_here(comp_ctx, pc_label_site + 2);  /* continue jump */

	reg_cond = exprtop_toregconst(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
	emit_if_true_skip(comp_ctx, reg_cond);
	pc_jump_false = emit_jump_empty(comp_ctx);
	SETTEMP(comp_ctx, temp_reset);

	advance_expect(comp_ctx, DUK_TOK_RPAREN);

	parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);
	emit_jump(comp_ctx, pc_start);

	patch_jump_here(comp_ctx, pc_jump_false);
	patch_jump_here(comp_ctx, pc_label_site + 1);  /* break jump */

	DUK_DDDPRINT("end parsing while statement");
}

static void parse_break_or_continue_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	duk_hthread *thr = comp_ctx->thr;
	int is_break = (comp_ctx->curr_token.t == DUK_TOK_BREAK);
	int label_id;
	int label_catch_depth;
	int label_pc;  /* points to LABEL; pc+1 = jump site for break; pc+2 = jump site for continue */
	int label_is_closest;

	advance(comp_ctx);  /* eat 'break' or 'continue' */

	if (comp_ctx->curr_token.t == DUK_TOK_SEMICOLON ||  /* explicit semi follows */
	    comp_ctx->curr_token.lineterm ||                /* automatic semi will be inserted */
	    comp_ctx->curr_token.allow_auto_semi) {         /* automatic semi will be inserted */
		/* break/continue without label */

		lookup_active_label(comp_ctx, DUK_HTHREAD_STRING_EMPTY_STRING(thr), is_break, &label_id, &label_catch_depth, &label_pc, &label_is_closest);
	} else if (comp_ctx->curr_token.t == DUK_TOK_IDENTIFIER) {
		/* break/continue with label (label cannot be a reserved word, production is 'Identifier' */
		DUK_ASSERT(comp_ctx->curr_token.str1 != NULL);
		lookup_active_label(comp_ctx, comp_ctx->curr_token.str1, is_break, &label_id, &label_catch_depth, &label_pc, &label_is_closest);
		advance(comp_ctx);
	} else {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid break/continue label");
	}

	/* Use a fast break/continue when possible.  A fast break/continue is
	 * just a jump to the LABEL break/continue jump slot, which then jumps
	 * to an appropriate place (for break, going through ENDLABEL correctly).
	 * The peephole optimizer will optimize the jump to a direct one.
	 */

	if (label_catch_depth == comp_ctx->curr_func.catch_depth &&
	    label_is_closest) {
		DUK_DDDPRINT("break/continue: is_break=%d, label_id=%d, label_is_closest=%d, "
		             "label_catch_depth=%d, catch_depth=%d "
		             "-> use fast variant (direct jump)",
		             is_break, label_id, label_is_closest, label_catch_depth,
		             comp_ctx->curr_func.catch_depth);

		emit_jump(comp_ctx, label_pc + (is_break ? 1 : 2));
	} else {
		DUK_DDDPRINT("break/continue: is_break=%d, label_id=%d, label_is_closest=%d, "
		             "label_catch_depth=%d, catch_depth=%d "
		             "-> use slow variant (longjmp)",
		             is_break, label_id, label_is_closest, label_catch_depth,
		             comp_ctx->curr_func.catch_depth);

		emit_abc(comp_ctx,
		         is_break ? DUK_OP_BREAK : DUK_OP_CONTINUE,
		         label_id);
	}
}

static void parse_return_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	duk_hthread *thr = comp_ctx->thr;
	int reg_val;
	int ret_flags;

	advance(comp_ctx);  /* eat 'return' */

	/* A 'return' statement is only allowed inside an actual function body,
	 * not as part of eval or global code.
	 */
	if (!comp_ctx->curr_func.is_function) {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid return");
	}

	/* Use a fast return when possible.  A fast return does not cause a longjmp()
	 * unnecessarily.  A fast return can be done when no TCF catchers are active
	 * (this includes 'try' and 'with' statements).  Active label catches do not
	 * prevent a fast return; they're unwound on return automatically.
	 */

	ret_flags = 0;

	if (comp_ctx->curr_token.t == DUK_TOK_SEMICOLON ||  /* explicit semi follows */
	    comp_ctx->curr_token.lineterm ||                /* automatic semi will be inserted */
	    comp_ctx->curr_token.allow_auto_semi) {         /* automatic semi will be inserted */
		DUK_DDDPRINT("empty return value -> undefined");
		reg_val = 0;
	} else {
		int pc_before_expr;
		int pc_after_expr;

		DUK_DDDPRINT("return with a value");

		pc_before_expr = get_current_pc(comp_ctx);
		reg_val = exprtop_toregconst(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
		pc_after_expr = get_current_pc(comp_ctx);

		/* Tail call check: if last opcode emitted was CALL, and
		 * the context allows it, change the CALL to a tailcall.
		 */

		if (comp_ctx->curr_func.catch_depth == 0 &&   /* no catchers */
		    pc_after_expr > pc_before_expr) {         /* at least one opcode emitted */
			duk_compiler_instr *instr;

			instr = get_instr_ptr(comp_ctx, pc_after_expr - 1);
			DUK_ASSERT(instr != NULL);

			if (DUK_DEC_OP(instr->ins) == DUK_OP_CALL) {
				DUK_DDDPRINT("return statement detected a tail call opportunity: "
				             "catch depth is 0, exprtop() emitted >= 1 instructions, "
				             "and last instruction is a CALL "
				             "-> set TAILCALL flag");
				/* just flip the single bit */
				instr->ins |= DUK_ENC_OP_A_B_C(0, DUK_BC_CALL_FLAG_TAILCALL, 0, 0);

				/* no need to emit a RETURN */
				return;
			}
		}

		ret_flags = DUK_BC_RETURN_FLAG_HAVE_RETVAL;
	}

	if (comp_ctx->curr_func.catch_depth == 0) {
		DUK_DDDPRINT("fast return allowed -> use fast return");
		ret_flags |= DUK_BC_RETURN_FLAG_FAST;
	} else {
		DUK_DDDPRINT("fast return not allowed -> use slow return");
	}

	emit_a_b(comp_ctx, DUK_OP_RETURN, ret_flags /*flags*/, reg_val /*reg*/);
}

static void parse_throw_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	int reg_val;

	advance(comp_ctx);  /* eat 'throw' */

	if (comp_ctx->curr_token.t == DUK_TOK_SEMICOLON ||  /* explicit semi follows */
	    comp_ctx->curr_token.lineterm ||                /* automatic semi will be inserted */
	    comp_ctx->curr_token.allow_auto_semi) {         /* automatic semi will be inserted */
		DUK_DDDPRINT("empty throw value -> undefined");
		reg_val = ALLOCTEMP(comp_ctx);
		emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_LDUNDEF, reg_val, 0);
	} else {
		DUK_DDDPRINT("throw with a value");

		/* FIXME: currently must be a register, not a const */
		reg_val = exprtop_toreg(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
	}

	emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_THROW, reg_val, 0);
}

static void parse_try_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int reg_catch;      /* reg_catch+0 and reg_catch+1 are reserved for TRYCATCH */
	int const_varname = 0;
	int trycatch_flags = 0;
	int pc_trycatch = -1;
	int pc_catch = -1;
	int pc_finally = -1;

	/*
	 *  See the following documentation for discussion:
	 *
	 *    doc/execution.txt: control flow details
	 *
	 *  Try, catch, and finally "parts" are Blocks, not Statements, so
	 *  they must always be delimited by curly braces.  This is unlike e.g.
	 *  the if statement, which accepts any Statement.  This eliminates any
	 *  questions of matching parts of nested try statements.  The Block
	 *  parsing is implemented inline here (instead of calling out).
	 *
	 *  Finally part has a 'let scoped' variable, which requires a few kinks
	 *  here.
	 */

	comp_ctx->curr_func.catch_depth++;

	advance(comp_ctx);  /* eat 'try' */

	reg_catch = ALLOCTEMPS(comp_ctx, 2);

	pc_trycatch = get_current_pc(comp_ctx);
	emit_invalid(comp_ctx);  /* TRYCATCH, cannot emit know (not enough info) */
	emit_invalid(comp_ctx);  /* jump for 'catch' case */
	emit_invalid(comp_ctx);  /* jump for 'finally' case or end (if no finally) */

	/* try part */
	advance_expect(comp_ctx, DUK_TOK_LCURLY);
	parse_statements(comp_ctx, 0 /*allow_source_elem*/, 0 /*expect_eof*/);
	/* the DUK_TOK_RCURLY is eaten by parse_statements() */
	emit_extraop_only(comp_ctx, DUK_EXTRAOP_ENDTRY);

	if (comp_ctx->curr_token.t == DUK_TOK_CATCH) {
		/*
		 *  The catch variable must be updated to reflect the new allocated
		 *  register for the duration of the catch clause.  We need to store
		 *  and restore the original value for the varmap entry (if any).
		 */

		/*
		 *  Note: currently register bindings must be fixed for the entire
		 *  function.  So, even though the catch variable is in a register
		 *  we know, we must use an explicit environment record and slow path
		 *  accesses to read/write the catch binding to make closures created
		 *  within the catch clause work correctly.  This restriction should
		 *  be fixable (at least in common cases) later.
		 *
		 *  See: test-dev-bug-catch-binding-2.js.
		 *
		 *  FIXME: improve to get fast path access to most catch clauses.
		 */

		duk_hstring *h_var;
		int varmap_value;  /* for storing/restoring the varmap binding for catch variable */

		DUK_DDDPRINT("stack top at start of catch clause: %d", duk_get_top(ctx));

		trycatch_flags |= DUK_BC_TRYCATCH_FLAG_HAVE_CATCH;

		pc_catch = get_current_pc(comp_ctx);

		advance(comp_ctx);
		advance_expect(comp_ctx, DUK_TOK_LPAREN);

		if (comp_ctx->curr_token.t != DUK_TOK_IDENTIFIER) {
			/* Identifier, i.e. don't allow reserved words */
			goto syntax_error;
		}
		h_var = comp_ctx->curr_token.str1;
		DUK_ASSERT(h_var != NULL);

		duk_push_hstring(ctx, h_var);  /* keep in on valstack, use borrowed ref below */

		if (comp_ctx->curr_func.is_strict &&
		    ((h_var == DUK_HTHREAD_STRING_EVAL(thr)) ||
		     (h_var == DUK_HTHREAD_STRING_LC_ARGUMENTS(thr)))) {
			DUK_DDDPRINT("catch identifier 'eval' or 'arguments' in strict mode -> SyntaxError");
			goto syntax_error;
		}

		duk_dup_top(ctx);
		const_varname = getconst(comp_ctx);
		DUK_DDDPRINT("catch clause, const_varname=0x%08x (%d)", const_varname, const_varname);

		advance(comp_ctx);
		advance_expect(comp_ctx, DUK_TOK_RPAREN);

		advance_expect(comp_ctx, DUK_TOK_LCURLY);

		DUK_DDDPRINT("varmap before modifying for catch clause: %!iT", duk_get_tval(ctx, comp_ctx->curr_func.varmap_idx));

		duk_dup_top(ctx);
		duk_get_prop(ctx, comp_ctx->curr_func.varmap_idx);
		if (duk_is_undefined(ctx, -1)) {
			varmap_value = -2;
		} else if (duk_is_null(ctx, -1)) {
			varmap_value = -1;
		} else {
			DUK_ASSERT(duk_is_number(ctx, -1));
			varmap_value = duk_get_int(ctx, -1);
			DUK_ASSERT(varmap_value >= 0);
		}
		duk_pop(ctx);

#if 0  /* something like this is what we'd like to do, but it doesn't work for closures created inside the catch clause */
		duk_dup_top(ctx);
		duk_push_int(ctx, reg_catch + 0);
		duk_put_prop(ctx, comp_ctx->curr_func.varmap_idx);
#endif
		duk_dup_top(ctx);
		duk_push_null(ctx);
		duk_put_prop(ctx, comp_ctx->curr_func.varmap_idx);

		emit_a_b_c(comp_ctx, DUK_OP_PUTVAR, 0 /*unused */, const_varname /*varname*/, reg_catch + 0 /*value*/);

		DUK_DDDPRINT("varmap before parsing catch clause: %!iT", duk_get_tval(ctx, comp_ctx->curr_func.varmap_idx));

		parse_statements(comp_ctx, 0 /*allow_source_elem*/, 0 /*expect_eof*/);
		/* the DUK_TOK_RCURLY is eaten by parse_statements() */

		if (varmap_value == -2) {
			/* not present */
			duk_del_prop(ctx, comp_ctx->curr_func.varmap_idx);
		} else {
			if (varmap_value == -1) {
				duk_push_null(ctx);
			} else {
				DUK_ASSERT(varmap_value >= 0);
				duk_push_int(ctx, varmap_value);
			}
			duk_put_prop(ctx, comp_ctx->curr_func.varmap_idx);
		}
		/* varname is popped by above code */

		DUK_DDDPRINT("varmap after restore catch clause: %!iT", duk_get_tval(ctx, comp_ctx->curr_func.varmap_idx));

		emit_extraop_only(comp_ctx, DUK_EXTRAOP_ENDCATCH);

		/*
		 *  FIXME: for now, indicate that an expensive catch binding
		 *  declarative environment is always needed.  If we don't
		 *  need it, we don't need the const_varname either.
		 */

		trycatch_flags |= DUK_BC_TRYCATCH_FLAG_CATCH_BINDING;

		DUK_DDDPRINT("stack top at end of catch clause: %d", duk_get_top(ctx));
	}

	if (comp_ctx->curr_token.t == DUK_TOK_FINALLY) {
		trycatch_flags |= DUK_BC_TRYCATCH_FLAG_HAVE_FINALLY;

		pc_finally = get_current_pc(comp_ctx);

		advance(comp_ctx);

		advance_expect(comp_ctx, DUK_TOK_LCURLY);
		parse_statements(comp_ctx, 0 /*allow_source_elem*/, 0 /*expect_eof*/);
		/* the DUK_TOK_RCURLY is eaten by parse_statements() */
		emit_extraop_b(comp_ctx, DUK_EXTRAOP_ENDFIN, reg_catch);  /* rethrow */
	}

	if (!(trycatch_flags & DUK_BC_TRYCATCH_FLAG_HAVE_CATCH) &&
	    !(trycatch_flags & DUK_BC_TRYCATCH_FLAG_HAVE_FINALLY)) {
		/* must have catch and/or finally */
		goto syntax_error;
	}

	patch_trycatch(comp_ctx,
	               pc_trycatch,
	               reg_catch,
	               const_varname,
	               trycatch_flags);

	if (trycatch_flags & DUK_BC_TRYCATCH_FLAG_HAVE_CATCH) {
		DUK_ASSERT(pc_catch >= 0);
		patch_jump(comp_ctx, pc_trycatch + 1, pc_catch);
	}

	if (trycatch_flags & DUK_BC_TRYCATCH_FLAG_HAVE_FINALLY) {
		DUK_ASSERT(pc_finally >= 0);
		patch_jump(comp_ctx, pc_trycatch + 2, pc_finally);
	} else {
		/* without finally, the second jump slot is used to jump to end of stmt */
		patch_jump_here(comp_ctx, pc_trycatch + 2);
	}

	comp_ctx->curr_func.catch_depth--;
	return;

 syntax_error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid try statement");
}

static void parse_with_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res) {
	int pc_trycatch;
	int pc_finished;
	int reg_catch;
	int reg_target;
	int trycatch_flags;

	advance(comp_ctx);  /* eat 'with' */

	reg_catch = ALLOCTEMPS(comp_ctx, 2);

	advance_expect(comp_ctx, DUK_TOK_LPAREN);
	reg_target = exprtop_toregconst(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);
	advance_expect(comp_ctx, DUK_TOK_RPAREN);

	pc_trycatch = get_current_pc(comp_ctx);
	trycatch_flags = DUK_BC_TRYCATCH_FLAG_WITH_BINDING;
	emit_a_b_c(comp_ctx, DUK_OP_TRYCATCH, trycatch_flags /*a*/, reg_catch /*b*/, reg_target /*c*/);
	emit_invalid(comp_ctx);  /* catch jump */
	emit_invalid(comp_ctx);  /* finished jump */

	parse_statement(comp_ctx, res, 0 /*allow_source_elem*/);
	emit_extraop_only(comp_ctx, DUK_EXTRAOP_ENDTRY);

	pc_finished = get_current_pc(comp_ctx);

	patch_jump(comp_ctx, pc_trycatch + 2, pc_finished);
}

static int stmt_label_site(duk_compiler_ctx *comp_ctx, int label_id) {
	/* if a site already exists, nop: max one label site per statement */
	if (label_id >= 0) {
		return label_id;
	}

	label_id = comp_ctx->curr_func.label_next++;
	DUK_DDDPRINT("allocated new label id for label site: %d", label_id);

	emit_abc(comp_ctx, DUK_OP_LABEL, label_id);
	emit_invalid(comp_ctx);
	emit_invalid(comp_ctx);

	return label_id;
}

/* Parse a single statement.
 *
 * Creates a label site (with an empty label) automatically for iteration
 * statements.  Also "peels off" any label statements for explicit labels.
 */
static void parse_statement(duk_compiler_ctx *comp_ctx, duk_ivalue *res, int allow_source_elem) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int dir_prol_at_entry;
	int temp_at_entry;
	int labels_len_at_entry;
	int pc_at_entry;   /* assumed to also be PC of "LABEL" */
	int stmt_id;
	int stmt_flags = 0;
	int label_id = -1;
	int tok;

	RECURSION_INCREASE(comp_ctx, thr);

	temp_at_entry = GETTEMP(comp_ctx);
	pc_at_entry = get_current_pc(comp_ctx);
	labels_len_at_entry = duk_get_length(ctx, comp_ctx->curr_func.labelnames_idx);
	stmt_id = comp_ctx->curr_func.stmt_next++;
	dir_prol_at_entry = comp_ctx->curr_func.in_directive_prologue;

	stmt_id = stmt_id;  /* suppress warning */

	DUK_DDDPRINT("parsing a statement, stmt_id=%d, temp_at_entry=%d, labels_len_at_entry=%d, "
	             "is_strict=%d, in_directive_prologue=%d, catch_depth=%d",
	             stmt_id, temp_at_entry, labels_len_at_entry, comp_ctx->curr_func.is_strict,
	             comp_ctx->curr_func.in_directive_prologue, comp_ctx->curr_func.catch_depth);

	/* The directive prologue flag is cleared by default so that it is
	 * unset for any recursive statement parsing.  It is only "revived"
	 * if a directive is detected.  (We could also make directives only
	 * allowed if 'allow_source_elem' was true.)
	 */
	comp_ctx->curr_func.in_directive_prologue = 0;

 retry_parse:

	DUK_DDDPRINT("try stmt parse, stmt_id=%d, label_id=%d, allow_source_elem=%d, catch_depth=%d",
	             stmt_id, label_id, allow_source_elem, comp_ctx->curr_func.catch_depth);

	/*
	 *  Detect iteration statements; if encountered, establish an
	 *  empty label.
	 */

	tok = comp_ctx->curr_token.t;
	if (tok == DUK_TOK_FOR || tok == DUK_TOK_DO || tok == DUK_TOK_WHILE ||
	    tok == DUK_TOK_SWITCH) {
		DUK_DDDPRINT("iteration/switch statement -> add empty label");

		label_id = stmt_label_site(comp_ctx, label_id);
		add_label(comp_ctx,
		          DUK_HTHREAD_STRING_EMPTY_STRING(thr),
		          pc_at_entry /*pc_label*/,
		          label_id);
	}

	/*
	 *  Main switch for statement / source element type.
	 */

	switch (comp_ctx->curr_token.t) {
	case DUK_TOK_FUNCTION: {
		/*
		 *  Function declaration, function expression, or (non-standard)
		 *  function statement.
		 *
		 *  The E5 specification only allows function declarations at
		 *  the top level (in "source elements").  An ExpressionStatement
		 *  is explicitly not allowed to begin with a "function" keyword
		 *  (E5 Section 12.4).  Hence any non-error semantics for such
		 *  non-top-level statements are non-standard.
		 */

		if (allow_source_elem) {
			/* FunctionDeclaration: not strictly a statement but handled as such */
			int fnum;

			DUK_DDDPRINT("function declaration statement");

			advance(comp_ctx);  /* eat 'function' */
			fnum = parse_function_like_fnum(comp_ctx, 1 /*is_decl*/, 0 /*is_setget*/);

			if (comp_ctx->curr_func.in_scanning) {
				int n;
				duk_hstring *h_funcname;

				duk_get_prop_index(ctx, comp_ctx->curr_func.funcs_idx, fnum);
				duk_get_prop_stridx(ctx, -1, DUK_STRIDX_NAME);  /* -> [ ... func name ] */
				h_funcname = duk_get_hstring(ctx, -1);
				DUK_ASSERT(h_funcname != NULL);

				DUK_DDDPRINT("register function declaration %!O in pass 1, fnum %d", h_funcname, fnum);
				n = duk_get_length(ctx, comp_ctx->curr_func.decls_idx);  /*FIXME: primitive for pushing*/
				duk_push_hstring(ctx, h_funcname);
				duk_put_prop_index(ctx, comp_ctx->curr_func.decls_idx, n);
				duk_push_int(ctx, DUK_DECL_TYPE_FUNC + (fnum << 8));
				duk_put_prop_index(ctx, comp_ctx->curr_func.decls_idx, n + 1);

				duk_pop_n(ctx, 2);
			}

			/* no statement value (unlike function expression) */
			stmt_flags = 0;
			break;
		} else {
			/* FIXME: add support for non-standard function statements and/or
			 * non-top-level function expressions.
			 */

			if (1) {
				/* Standard behavior */
				DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "function declaration not allowed outside of top level");
			} else if (0) {
				/* Non-standard: interpret as a function expression inside an ExpressionStatement */
				DUK_DDDPRINT("function expression (inside an expression statement; non-standard)");
				stmt_flags = (HAS_VAL);  /* FIXME -- e.g. HAS_TERM? */
			} else if (0) {
				/* Non-standard: interpret as a function statement */
				DUK_DDDPRINT("function statement (non-standard)");
				stmt_flags = 0;  /* FIXME */
			}
		}
		DUK_ERROR(thr, DUK_ERR_UNIMPLEMENTED_ERROR, "non-standard function expression/statement unimplemented");
		break;
	}
	case DUK_TOK_LCURLY: {
		DUK_DDDPRINT("block statement");
		advance(comp_ctx);
		parse_statements(comp_ctx, 0 /*allow_source_elem*/, 0 /*expect_eof*/);
		/* the DUK_TOK_RCURLY is eaten by parse_statements() */
		stmt_flags = 0;
		break;
	}
	case DUK_TOK_VAR: {
		DUK_DDDPRINT("variable declaration statement");
		parse_var_statement(comp_ctx, res);
		stmt_flags = HAS_TERM;
		break;
	}
	case DUK_TOK_SEMICOLON: {
		/* empty statement with an explicit semicolon */
		DUK_DDDPRINT("empty statement");
		stmt_flags = HAS_TERM;
		break;
	}
	case DUK_TOK_IF: {
		DUK_DDDPRINT("if statement");
		parse_if_statement(comp_ctx, res);
		stmt_flags = 0;
		break;
	}
	case DUK_TOK_DO: {
		/*
		 *  Do-while statement is mostly trivial, but there is special
		 *  handling for automatic semicolon handling (triggered by the
		 *  ALLOW_AUTO_SEMI_ALWAYS) flag related to a bug filed at:
		 *
		 *    https://bugs.ecmascript.org/show_bug.cgi?id=8
		 *
		 *  See doc/compiler.txt for details.
		 */
		DUK_DDDPRINT("do statement");
		DUK_ASSERT(label_id >= 0);
		update_label_flags(comp_ctx,
		                   label_id,
		                   DUK_LABEL_FLAG_ALLOW_BREAK | DUK_LABEL_FLAG_ALLOW_CONTINUE);
		parse_do_statement(comp_ctx, res, pc_at_entry);
		stmt_flags = HAS_TERM | ALLOW_AUTO_SEMI_ALWAYS;  /* ALLOW_AUTO_SEMI_ALWAYS workaround */
		break;
	}
	case DUK_TOK_WHILE: {
		DUK_DDDPRINT("while statement");
		DUK_ASSERT(label_id >= 0);
		update_label_flags(comp_ctx,
		                   label_id,
		                   DUK_LABEL_FLAG_ALLOW_BREAK | DUK_LABEL_FLAG_ALLOW_CONTINUE);
		parse_while_statement(comp_ctx, res, pc_at_entry);
		stmt_flags = 0;
		break;
	}
	case DUK_TOK_FOR: {
		/*
		 *  For/for-in statement is complicated to parse because
		 *  determining the statement type (three-part for vs. a
		 *  for-in) requires potential backtracking.
		 *
		 *  See the helper for the messy stuff.
		 */
		DUK_DDDPRINT("for/for-in statement");
		DUK_ASSERT(label_id >= 0);
		update_label_flags(comp_ctx,
		                   label_id,
		                   DUK_LABEL_FLAG_ALLOW_BREAK | DUK_LABEL_FLAG_ALLOW_CONTINUE);
		parse_for_statement(comp_ctx, res, pc_at_entry);
		stmt_flags = 0;
		break;
	}
	case DUK_TOK_CONTINUE:
	case DUK_TOK_BREAK: {
		DUK_DDDPRINT("break/continue statement");
		parse_break_or_continue_statement(comp_ctx, res);
		stmt_flags = HAS_TERM | IS_TERMINAL;
		break;
	}
	case DUK_TOK_RETURN: {
		DUK_DDDPRINT("return statement");
		parse_return_statement(comp_ctx, res);
		stmt_flags = HAS_TERM | IS_TERMINAL;
		break;
	}
	case DUK_TOK_WITH: {
		DUK_DDDPRINT("with statement");
		comp_ctx->curr_func.with_depth++;
		parse_with_statement(comp_ctx, res);
		comp_ctx->curr_func.with_depth--;
		stmt_flags = 0;
		break;
	}
	case DUK_TOK_SWITCH: {
		/*
		 *  The switch statement is pretty messy to compile.
		 *  See the helper for details.
		 */
		DUK_DDDPRINT("switch statement");
		DUK_ASSERT(label_id >= 0);
		update_label_flags(comp_ctx,
		                   label_id,
		                   DUK_LABEL_FLAG_ALLOW_BREAK);  /* don't allow continue */
		parse_switch_statement(comp_ctx, res, pc_at_entry);
		stmt_flags = 0;
		break;
	}
	case DUK_TOK_THROW: {
		DUK_DDDPRINT("throw statement");
		parse_throw_statement(comp_ctx, res);
		stmt_flags = HAS_TERM | IS_TERMINAL;
		break;
	}
	case DUK_TOK_TRY: {
		DUK_DDDPRINT("try statement");
		parse_try_statement(comp_ctx, res);
		stmt_flags = 0;
		break;
	}
	case DUK_TOK_DEBUGGER: {
		DUK_DDDPRINT("debugger statement: ignored");
		advance(comp_ctx);
		stmt_flags = HAS_TERM;
		break;
	}
	default: {
		/*
		 *  Else, must be one of:
		 *    - ExpressionStatement, possibly a directive (String)
		 *    - LabelledStatement (Identifier followed by ':')
		 *
		 *  Expressions beginning with 'function' keyword are covered by a case
		 *  above (such expressions are not allowed in standard E5 anyway).
		 *  Also expressions starting with '{' are interpreted as block
		 *  statements.  See E5 Section 12.4.
		 *
		 *  Directive detection is tricky; see E5 Section 14.1 on directive
		 *  prologue.  A directive is an expression statement with a single
		 *  string literal and an explicit or automatic semicolon.  Escape
		 *  characters are significant and no parens etc are allowed:
		 *
		 *    'use strict';          // valid 'use strict' directive
		 *    'use\u0020strict';     // valid directive, not a 'use strict' directive
		 *    ('use strict');        // not a valid directive
		 *
		 *  The expression is determined to consist of a single string literal
		 *  based on expr_nud() and expr_led() call counts.  The string literal
		 *  of a 'use strict' directive is determined to lack any escapes based
		 *  num_escapes count from the lexer.  Note that other directives may be
		 *  allowed to contain escapes, so a directive with escapes does not
		 *  terminate a directive prologue.
		 *
		 *  We rely on the fact that the expression parser will not emit any
		 *  code for a single token expression.  However, it will generate an
		 *  intermediate value which we will then successfully ignore.
		 *
		 *  A similar approach is used for labels.
		 */

		int single_token;

		DUK_DDDPRINT("expression statement");
		exprtop(comp_ctx, res, BP_FOR_EXPR /*rbp_flags*/);

		single_token = (comp_ctx->curr_func.nud_count == 1 &&  /* one token */
		                comp_ctx->curr_func.led_count == 0);   /* no operators */

		if (single_token &&
		    comp_ctx->prev_token.t == DUK_TOK_IDENTIFIER &&
		    comp_ctx->curr_token.t == DUK_TOK_COLON) {
			/*
			 *  Detected label
			 */

			duk_hstring *h_lab;

			/* expected ival */
			DUK_ASSERT(res->t == DUK_IVAL_VAR);
			DUK_ASSERT(res->x1.t == DUK_ISPEC_VALUE);
			DUK_ASSERT(DUK_TVAL_IS_STRING(duk_get_tval(ctx, res->x1.valstack_idx)));
			h_lab = comp_ctx->prev_token.str1;
			DUK_ASSERT(h_lab != NULL);

			DUK_DDDPRINT("explicit label site for label '%!O'", h_lab);

			advance(comp_ctx);  /* eat colon */

			label_id = stmt_label_site(comp_ctx, label_id);

			add_label(comp_ctx,
			          h_lab,
			          pc_at_entry /*pc_label*/,
			          label_id);
	
			/* a statement following a label cannot be a source element
			 * (a function declaration).
			 */
			allow_source_elem = 0;

			DUK_DDDPRINT("label handled, retry statement parsing");
			goto retry_parse;
		}

		stmt_flags = 0;

		if (dir_prol_at_entry &&                           /* still in prologue */
		    single_token &&                                /* single string token */
		    comp_ctx->prev_token.t == DUK_TOK_STRING) {
			/*
			 *  Detected a directive

			 */
			duk_hstring *h_dir;

			/* expected ival */
			DUK_ASSERT(res->t == DUK_IVAL_PLAIN);
			DUK_ASSERT(res->x1.t == DUK_ISPEC_VALUE);
			DUK_ASSERT(DUK_TVAL_IS_STRING(duk_get_tval(ctx, res->x1.valstack_idx)));
			h_dir = comp_ctx->prev_token.str1;
			DUK_ASSERT(h_dir != NULL);

			stmt_flags |= STILL_PROLOGUE;

			/* Note: escaped characters differentiate directives */

			if (comp_ctx->prev_token.num_escapes > 0) {
				DUK_DDDPRINT("directive contains escapes: valid directive "
				             "but we ignore such directives");
			} else {
				/* FIXME: how to compare 'use strict' most compactly?
				 * We don't necessarily want to add it to the built-ins
				 * because it's not needed at run time.
				 */

				if (DUK_HSTRING_GET_BYTELEN(h_dir) == 10 &&
				    strncmp((const char *) DUK_HSTRING_GET_DATA(h_dir), "use strict", 10) == 0) {
					DUK_DDDPRINT("use strict directive detected: strict flag %d -> %d",
					             comp_ctx->curr_func.is_strict, 1);
					comp_ctx->curr_func.is_strict = 1;
				} else {
					DUK_DDPRINT("unknown directive: '%!O', ignoring but not terminating "
					            "directive prologue", (duk_hobject *) h_dir);
				}
			}
		} else {
			DUK_DDDPRINT("non-directive expression statement or no longer in prologue; "
			             "prologue terminated if still active");
                }

		stmt_flags |= HAS_VAL | HAS_TERM;
	}
	}  /* end switch(tok) */

	/*
	 *  Statement value handling.
	 *
	 *  Global code and eval code has an implicit return value
	 *  which comes from the last statement with a value
	 *  (technically a non-"empty" continuation, which is
	 *  different from an empty statement).
	 *
	 *  Since we don't know whether a later statement will
	 *  override the value of the current statement, we need
	 *  to coerce the statement value to a register allocated
	 *  for implicit return values.  In other cases we need
	 *  to coerce the statement value to a plain value to get
	 *  any side effects out (consider e.g. "foo.bar;").
	 */

	/* FIXME: what about statements which leave a half-cooked value in 'res'
	 * but have no stmt value?  Any such statements?
	 */

	if (stmt_flags & HAS_VAL) {
		int reg_stmt_value = comp_ctx->curr_func.reg_stmt_value;
		if (reg_stmt_value >= 0) {
			ivalue_toforcedreg(comp_ctx, res, reg_stmt_value);
		} else {
			ivalue_toplain_ignore(comp_ctx, res);
		}
	} else {
		;
	}

	/*
	 *  Statement terminator check, including automatic semicolon
	 *  handling.  After this step, 'curr_tok' should be the first
	 *  token after a possible statement terminator.
	 */

	if (stmt_flags & HAS_TERM) {
		if (comp_ctx->curr_token.t == DUK_TOK_SEMICOLON) {
			DUK_DDDPRINT("explicit semicolon terminates statement");
			advance(comp_ctx);
		} else {
			if (comp_ctx->curr_token.allow_auto_semi) {
				DUK_DDDPRINT("automatic semicolon terminates statement");
			} else if (stmt_flags & ALLOW_AUTO_SEMI_ALWAYS) {
				/* FIXME: make this lenience dependent on flags or strictness? */
				DUK_DDDPRINT("automatic semicolon terminates statement (allowed for compatibility "
				             "even though no lineterm present before next token)");
			} else {
				DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "unterminated statement");
			}
		}
	} else {
		DUK_DDDPRINT("statement has no terminator");
	}

	/*
	 *  Directive prologue tracking.
	 */

	if (stmt_flags & STILL_PROLOGUE) {
		DUK_DDDPRINT("setting in_directive_prologue");
		comp_ctx->curr_func.in_directive_prologue = 1;
	}

	/*
	 *  Cleanups (all statement parsing flows through here).
	 *
	 *  Pop label site and reset labels.  Reset 'next temp' to value at
	 *  entry to reuse temps.
	 */

	if (label_id >= 0) {
		emit_abc(comp_ctx, DUK_OP_ENDLABEL, label_id);
	}

	SETTEMP(comp_ctx, temp_at_entry);

	reset_labels_to_length(comp_ctx, labels_len_at_entry);

	/* FIXME: return indication of "terminalness" (e.g. a 'throw' is terminal) */

	RECURSION_DECREASE(comp_ctx, thr);
}

#undef HAS_VAL
#undef HAS_TERM
#undef ALLOW_AUTO_SEMI_ALWAYS

/*
 *  Parse a statement list.
 *
 *  Handles automatic semicolon insertion and implicit return value.
 *
 *  Upon entry, 'curr_tok' should contain the first token of the first
 *  statement (parsed in the "allow regexp literal" mode).  Upon exit,
 *  'curr_tok' contains the token following the statement list terminator
 *  (EOF or closing brace).
 */

static void parse_statements(duk_compiler_ctx *comp_ctx, int allow_source_elem, int expect_eof) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_ivalue res_alloc;
	duk_ivalue *res = &res_alloc;

	/* Setup state.  Initial ivalue is 'undefined'. */

	duk_require_stack(ctx, PARSE_STATEMENTS_SLOTS);

	/* FIXME: 'res' setup can be moved to function body level; in fact, two 'res'
	 * intermediate values suffice for parsing of each function.  Nesting is needed
	 * for nested functions (which may occur inside expressions).
	 */

	memset(&res_alloc, 0, sizeof(res_alloc));
	res->t = DUK_IVAL_PLAIN;
	res->x1.t = DUK_ISPEC_VALUE;
	res->x1.valstack_idx = duk_get_top(ctx);
	res->x2.valstack_idx = res->x1.valstack_idx + 1;
	duk_push_undefined(ctx);
	duk_push_undefined(ctx);

	/* Parse statements until a closing token (EOF or '}') is found. */

	for (;;) {
		/* Check whether statement list ends. */

		if (expect_eof) {
			if (comp_ctx->curr_token.t == DUK_TOK_EOF) {
				break;
			}
		} else {
			if (comp_ctx->curr_token.t == DUK_TOK_RCURLY) {
				break;
			}
		}

		/* Check statement type based on the first token type.
		 *
		 * Note: expression parsing helpers expect 'curr_tok' to
		 * contain the first token of the expression upon entry.
		 */

		DUK_DDDPRINT("TOKEN %d (non-whitespace, non-comment)", comp_ctx->curr_token.t);

		parse_statement(comp_ctx, res, allow_source_elem);
	}

	advance(comp_ctx);

	/* Tear down state. */

	duk_pop_2(ctx);
}

/*
 *  Declaration binding instantiation conceptually happens when calling a
 *  function; for us it essentially means that function prologue.  The
 *  conceptual process is described in E5 Section 10.5.
 *
 *  We need to keep track of all encountered identifiers to (1) create an
 *  identifier-to-register map ("varmap"); and (2) detect duplicate
 *  declarations.  Identifiers which are not bound to registers still need
 *  to be tracked for detecting duplicates.  Currently such identifiers
 *  are put into the varmap with a 'null' value, which is later cleaned up.
 *
 *  Some bindings in E5 are not configurable (= deletable) and almost all
 *  are mutable (writable).  Exceptions are:
 * 
 *    - The 'arguments' binding, established only if no shadowing argument
 *      or function declaration exists.  We handle 'arguments' creation
 *      and binding through an explicit slow path environment record.
 *
 *    - The "name" binding for a named function expression.  This is also
 *      handled through an explicit slow path environment record.
 */

/* FIXME: add support for variables to not be register bound always, to 
 * handle cases with a very large number of variables?
 */

static void init_varmap_and_prologue_for_pass2(duk_compiler_ctx *comp_ctx, int *out_stmt_value_reg) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *h_name;
	int configurable_bindings;
	int num_args;
	int num_decls;
	int reg_name;
	int declvar_flags;
	int i;
#ifdef DUK_USE_ASSERTIONS
	int entry_top;
#endif

#ifdef DUK_USE_ASSERTIONS
	entry_top = duk_get_top(ctx);
#endif

	/*
	 *  Preliminaries
	 */

	configurable_bindings = comp_ctx->curr_func.is_eval;
	DUK_DDDPRINT("configurable_bindings=%d", configurable_bindings);

	/* varmap is already in comp_ctx->curr_func.varmap_idx */

	/*
	 *  Function formal arguments, always bound to registers
	 */

	num_args = duk_get_length(ctx, comp_ctx->curr_func.argnames_idx);
	DUK_DDDPRINT("num_args=%d", num_args);
	for (i = 0; i < num_args; i++) {
		duk_get_prop_index(ctx, comp_ctx->curr_func.argnames_idx, i);
		h_name = duk_get_hstring(ctx, -1);
		DUK_ASSERT(h_name != NULL);

		if (comp_ctx->curr_func.is_strict) {
			if (hstring_is_eval_or_arguments(comp_ctx, h_name)) {
				DUK_DDDPRINT("arg named 'eval' or 'arguments' in strict mode -> SyntaxError");
				goto error_argname;
			}
			duk_dup_top(ctx);
			if (duk_has_prop(ctx, comp_ctx->curr_func.varmap_idx)) {
				DUK_DDDPRINT("duplicate arg name in strict mode -> SyntaxError");
				goto error_argname;
			}

			/* Ensure argument name is not a reserved word in current
			 * (final) strictness.  Formal argument parsing may not
			 * catch reserved names if strictness changes during
			 * parsing.
			 *
			 * We only need to do this in strict mode because non-strict
			 * keyword are always detected in formal argument parsing.
			 */

			if (DUK_HSTRING_HAS_STRICT_RESERVED_WORD(h_name)) {
				goto error_argname;
			}
		}
		DUK_ASSERT(!DUK_HSTRING_HAS_RESERVED_WORD(h_name));

		/* overwrite any previous binding of the same name; the effect is
		 * that last argument of a certain name wins.
		 */

		/* only functions can have arguments */
		DUK_ASSERT(comp_ctx->curr_func.is_function);
		duk_push_int(ctx, i);  /* -> [ ... name index ] */
		duk_put_prop(ctx, comp_ctx->curr_func.varmap_idx); /* -> [ ... ] */

		/* no code needs to be emitted, the regs already have values */
	}

	/* use temp_next for tracking register allocations */
	SETTEMP_CHECKMAX(comp_ctx, num_args);

	if (out_stmt_value_reg) {
		*out_stmt_value_reg = ALLOCTEMP(comp_ctx);
	}

	/*
	 *  Function declarations
	 */

	num_decls = duk_get_length(ctx, comp_ctx->curr_func.decls_idx);
	DUK_DDDPRINT("num_decls=%d -> %!T", num_decls, duk_get_tval(ctx, comp_ctx->curr_func.decls_idx));
	for (i = 0; i < num_decls; i += 2) {
		int decl_type;
		int fnum;

		duk_get_prop_index(ctx, comp_ctx->curr_func.decls_idx, i + 1);  /* decl type */
		decl_type = duk_to_int(ctx, -1);
		fnum = decl_type >> 8;  /* FIXME: macros */
		decl_type = decl_type & 0xff;
		duk_pop(ctx);

		if (decl_type != DUK_DECL_TYPE_FUNC) {
			continue;
		}

		duk_get_prop_index(ctx, comp_ctx->curr_func.decls_idx, i);  /* decl name */

		if (comp_ctx->curr_func.is_function) {
			int reg_bind;
			duk_dup_top(ctx);
			if (duk_has_prop(ctx, comp_ctx->curr_func.varmap_idx)) {
				/* shadowed; update value */
				duk_dup_top(ctx);
				duk_get_prop(ctx, comp_ctx->curr_func.varmap_idx);
				reg_bind = duk_to_int(ctx, -1);  /* [ ... name reg_bind ] */
				emit_a_bc(comp_ctx, DUK_OP_CLOSURE, reg_bind, fnum);
			} else {
				/* function: always register bound */
				reg_bind = ALLOCTEMP(comp_ctx);
				emit_a_bc(comp_ctx, DUK_OP_CLOSURE, reg_bind, fnum);
				duk_push_int(ctx, reg_bind);
			}
		} else {
			/* Function declaration for global/eval code is emitted even
			 * for duplicates, because of E5 Section 10.5, step 5.e of
			 * E5.1 (special behavior for variable bound to global object).
			 *
			 * DECLVAR will not re-declare a variable as such, but will
			 * update the binding value.
			 */

			int reg_temp = ALLOCTEMP(comp_ctx);
			duk_dup_top(ctx);
			reg_name = getconst(comp_ctx);
			duk_push_null(ctx);

			emit_a_bc(comp_ctx, DUK_OP_CLOSURE, reg_temp, fnum);

			declvar_flags = DUK_PROPDESC_FLAG_WRITABLE |
			                DUK_PROPDESC_FLAG_ENUMERABLE |
			                DUK_BC_DECLVAR_FLAG_FUNC_DECL;

			if (configurable_bindings) {
				declvar_flags |= DUK_PROPDESC_FLAG_CONFIGURABLE;
			}

			emit_a_b_c(comp_ctx, DUK_OP_DECLVAR, declvar_flags /*flags*/, reg_name /*name*/, reg_temp /*value*/);

			SETTEMP(comp_ctx, reg_temp);  /* forget temp */
		}

		DUK_DDDPRINT("function declaration to varmap: %!T -> %!T", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

		duk_put_prop(ctx, comp_ctx->curr_func.varmap_idx);  /* [ ... name reg/null ] -> [ ... ] */
	}

	/*
	 *  'arguments' binding is special; if a shadowing argument or
	 *  function declaration exists, an arguments object will
	 *  definitely not be needed, regardless of whether the identifier
	 *  'arguments' is referenced inside the function body.
	 */

	if (duk_has_prop_stridx(ctx, comp_ctx->curr_func.varmap_idx, DUK_STRIDX_LC_ARGUMENTS)) {
		DUK_DDDPRINT("'arguments' is shadowed by argument or function declaration "
		             "-> arguments object creation can be skipped");
		comp_ctx->curr_func.is_arguments_shadowed = 1;
	}

	/*
	 *  Variable declarations.
	 *
	 *  Unlike function declarations, variable declaration values don't get
	 *  assigned on entry.  If a binding of the same name already exists, just
	 *  ignore it silently.
	 */

	for (i = 0; i < num_decls; i += 2) {
		int decl_type;

		duk_get_prop_index(ctx, comp_ctx->curr_func.decls_idx, i + 1);  /* decl type */
		decl_type = duk_to_int(ctx, -1);
		decl_type = decl_type & 0xff;
		duk_pop(ctx);

		if (decl_type != DUK_DECL_TYPE_VAR) {
			continue;
		}

		duk_get_prop_index(ctx, comp_ctx->curr_func.decls_idx, i);  /* decl name */

		if (duk_has_prop(ctx, comp_ctx->curr_func.varmap_idx)) {
			/* shadowed, ignore */
		} else {
			duk_get_prop_index(ctx, comp_ctx->curr_func.decls_idx, i);  /* decl name */
			h_name = duk_get_hstring(ctx, -1);
			DUK_ASSERT(h_name != NULL);

			if (h_name == DUK_HTHREAD_STRING_LC_ARGUMENTS(thr) &&
			    !comp_ctx->curr_func.is_arguments_shadowed) {
				/* E5 Section steps 7-8 */
				DUK_DDDPRINT("'arguments' not shadowed by a function declaration, "
				             "but appears as a variable declaration -> treat as "
				             "a no-op for variable declaration purposes");
				duk_pop(ctx);
				continue;
			}

			if (comp_ctx->curr_func.is_function) {
				int reg_bind = ALLOCTEMP(comp_ctx);
				/* no need to init reg, it will be undefined on entry */
				duk_push_int(ctx, reg_bind);
			} else {
				duk_dup_top(ctx);
				reg_name = getconst(comp_ctx);
				duk_push_null(ctx);

				declvar_flags = DUK_PROPDESC_FLAG_WRITABLE |
			                        DUK_PROPDESC_FLAG_ENUMERABLE |
				                DUK_BC_DECLVAR_FLAG_UNDEF_VALUE;
				if (configurable_bindings) {
					declvar_flags |= DUK_PROPDESC_FLAG_CONFIGURABLE;
				}

				emit_a_b_c(comp_ctx, DUK_OP_DECLVAR, declvar_flags /*flags*/, reg_name /*name*/, 0 /*value*/);
			}

			duk_put_prop(ctx, comp_ctx->curr_func.varmap_idx);  /* [ ... name reg/null ] -> [ ... ] */
		}
	}

	/*
	 *  Wrap up
	 */

	DUK_DDDPRINT("varmap: %!T, is_arguments_shadowed=%d",
	             duk_get_tval(ctx, comp_ctx->curr_func.varmap_idx),
	             comp_ctx->curr_func.is_arguments_shadowed);

	DUK_ASSERT_TOP(ctx, entry_top);
	return;

 error_argname:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid arg name");
}

/*
 *  Parse a function-body-like expression (FunctionBody or Program
 *  in E5 grammar) using a two-pass parse.  The productions appear
 *  in the following contexts:
 *
 *    - function expression
 *    - function statement
 *    - function declaration
 *    - getter in object literal
 *    - setter in object literal
 *    - global code
 *    - eval code
 *    - Function constructor body
 *
 *  This function only parses the statement list of the body; the argument
 *  list and possible function name must be initialized by the caller.
 *  For instance, for Function constructor, the argument names are originally
 *  on the value stack.  The parsing of statements ends either at an EOF or
 *  a closing brace; this is controlled by an input flag.
 *
 *  Note that there are many differences affecting parsing and even code
 *  generation:
 *
 *    - Global and eval code have an implicit return value generated
 *      by the last statement; function code does not
 *
 *    - Global code, eval code, and Function constructor body end in
 *      an EOF, other bodies in a closing brace ('}')
 *
 *  Upon entry, 'curr_tok' is ignored and the function will pull in the
 *  first token on its own.  Upon exit, 'curr_tok' is the terminating
 *  token (EOF or closing brace).
 */

static void parse_function_body(duk_compiler_ctx *comp_ctx, int expect_eof, int implicit_return_value) {
	duk_compiler_func *func = &comp_ctx->curr_func;
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int reg_stmt_value = -1;
	duk_lexer_point lex_pt;
	int temp_first;

	DUK_ASSERT(comp_ctx != NULL);
	DUK_ASSERT(func != NULL);

	RECURSION_INCREASE(comp_ctx, thr);

	duk_require_stack(ctx, FUNCTION_BODY_REQUIRE_SLOTS);

	/*
	 *  Store lexer position for a later rewind
	 */

	DUK_LEXER_GETPOINT(&comp_ctx->lex, &lex_pt);

	/*
	 *  Program code (global and eval code) has an implicit return value
	 *  from the last statement value (e.g. eval("1; 2+3;") returns 3).
	 *  This is not the case with functions.  If implicit statement return
	 *  value is requested, all statements are coerced to a register
	 *  allocated here, and used in the implicit return statement below.
	 */

	/* FIXME: this is pointless here because pass 1 is throw-away */
	if (implicit_return_value) {
		reg_stmt_value = ALLOCTEMP(comp_ctx);

		/* If an implicit return value is needed by caller, it must be
		 * initialized to 'undefined' because we don't know whether any
		 * non-empty (where "empty" is a continuation type, and different
		 * from an empty statement) statements will be executed.
		 *
		 * However, since 1st pass is a throwaway one, no need to emit
		 * it here.
		 */
#if 0
		emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_LDUNDEF, 0, 0);
#endif
	}

	/*
	 *  First pass parsing.
	 */

	func->in_directive_prologue = 1;
	func->in_scanning = 1;
	func->may_direct_eval = 0;
	func->id_access_arguments = 0;
	func->id_access_slow = 0;
	func->reg_stmt_value = reg_stmt_value;

	/* Need to set curr_token.t because lexing regexp mode depends on current
	 * token type.  Zero value causes "allow regexp" mode.
	 */
	comp_ctx->curr_token.t = 0;
	advance(comp_ctx);  /* parse_statements() expects curr_tok to be set; parse in "allow regexp literal" mode with current strictness */

	DUK_DDDPRINT("begin 1st pass");
	parse_statements(comp_ctx,
	                 1,             /* allow source elements */
	                 expect_eof);   /* expect EOF instead of } */
	DUK_DDDPRINT("end 1st pass");

	/*
	 *  Rewind lexer.
	 *
	 *  parse_statements() expects curr_tok to be set; parse in "allow regexp
	 *  literal" mode with current strictness.
	 *
	 *  curr_token line number info should be initialized for pass 2 before
	 *  generating prologue, to ensure prologue bytecode gets nice line numbers.
	 */

	DUK_DDDPRINT("rewind lexer");
	DUK_LEXER_SETPOINT(&comp_ctx->lex, &lex_pt);
	comp_ctx->curr_token.t = 0;  /* this is needed for regexp mode */
	advance(comp_ctx);

	/*
	 *  Reset function state and perform register allocation, which creates
	 *  'varmap' for second pass.  Function prologue for variable declarations,
	 *  binding value initializations etc is emitted as a by-product.
	 *
	 *  Strict mode restrictions for duplicate and invalid argument
	 *  names are checked here now that we know whether the function
	 *  is actually strict.  See: test-dev-strict-mode-boundary.js.
	 */

	reset_function_for_pass2(comp_ctx);
	func->in_directive_prologue = 1;
	func->in_scanning = 0;

	/* must be able to emit code, alloc consts, etc. */

	init_varmap_and_prologue_for_pass2(comp_ctx,
	                                 (implicit_return_value ? &reg_stmt_value : NULL));
	func->reg_stmt_value = reg_stmt_value;

	temp_first = GETTEMP(comp_ctx);

	func->temp_first = temp_first;
	func->temp_next = temp_first;
	func->stmt_next = 0;
	func->label_next = 0;

	/* FIXME: init or assert catch depth etc -- all values */
	func->id_access_arguments = 0;
	func->id_access_slow = 0;

	/*
	 *  Check function name validity now that we know strictness.
	 *  This only applies to function declarations and expressions,
	 *  not setter/getter name.
	 *
	 *  See: test-dev-strict-mode-boundary.js
	 */

	if (func->is_function && !func->is_setget && func->h_name != NULL) {
		if (func->is_strict) {
			if (hstring_is_eval_or_arguments(comp_ctx, func->h_name)) {
				DUK_DDDPRINT("func name is 'eval' or 'arguments' in strict mode");
				goto error_funcname;
			}
			if (DUK_HSTRING_HAS_STRICT_RESERVED_WORD(func->h_name)) {
				DUK_DDDPRINT("func name is a reserved word in strict mode");
				goto error_funcname;
			}
		} else {
			if (DUK_HSTRING_HAS_RESERVED_WORD(func->h_name) &&
			    !DUK_HSTRING_HAS_STRICT_RESERVED_WORD(func->h_name)) {
				DUK_DDDPRINT("func name is a reserved word in non-strict mode");
				goto error_funcname;
			}
		}
	}

	/*
	 *  Second pass parsing.
	 */

	if (implicit_return_value) {
		emit_extraop_b_c(comp_ctx, DUK_EXTRAOP_LDUNDEF, 0, 0);
	}

	DUK_DDDPRINT("begin 2nd pass");
	parse_statements(comp_ctx,
	                 1,             /* allow source elements */
	                 expect_eof);   /* expect EOF instead of } */
	DUK_DDDPRINT("end 2nd pass");

	/*
	 *  Emit a final RETURN.
	 *
	 *  It would be nice to avoid emitting an unnecessary "return" opcode
	 *  if the current PC is not reachable.  However, this cannot be reliably
	 *  detected; even if the previous instruction is an unconditional jump,
	 *  there may be a previous jump which jumps to current PC (which is the
	 *  case for iteration and conditional statements, for instance).
	 */

	/* FIXME: request a "last statement is terminal" from parse_statement() and parse_statements();
	 * we could avoid the last RETURN if we could ensure there is no way to get here
	 * (directly or via a jump)
	 */

	DUK_ASSERT(comp_ctx->curr_func.catch_depth == 0);  /* fast returns are always OK here */
	if (reg_stmt_value >= 0) {
		emit_a_b(comp_ctx,
		         DUK_OP_RETURN,
		         DUK_BC_RETURN_FLAG_HAVE_RETVAL | DUK_BC_RETURN_FLAG_FAST /*flags*/,
		         reg_stmt_value /*reg*/);
	} else {
		emit_a_b(comp_ctx,
		         DUK_OP_RETURN,
		         DUK_BC_RETURN_FLAG_FAST /*flags*/,
		         0 /*reg*/);
	}

	/*
	 *  Peephole optimize JUMP chains.
	 */

	peephole_optimize_bytecode(comp_ctx);

	/*
	 *  comp_ctx->curr_func is now ready to be converted into an actual
	 *  function template.
	 */

	RECURSION_DECREASE(comp_ctx, thr);
	return;

 error_funcname:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid function name");
}

/*
 *  Parse a function-like expression:
 *
 *    - function expression
 *    - function declaration
 *    - function statement (non-standard)
 *    - setter/getter
 *
 *  Adds the function to comp_ctx->curr_func function table and returns the
 *  function number.
 *
 *  On entry, curr_token points to:
 *
 *    - the token after 'function' for function expression/declaration/statement
 *    - the token after 'set' or 'get' for setter/getter
 */

/* Parse formals. */
static void parse_function_formals(duk_compiler_ctx *comp_ctx) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	int first = 1;
	int n;

	for (;;) {
		if (comp_ctx->curr_token.t == DUK_TOK_RPAREN) {
			break;
		}

		if (first) {
			/* no comma */
			first = 0;
		} else {
			advance_expect(comp_ctx, DUK_TOK_COMMA);
		}

		/* Note: when parsing a formal list in non-strict context, e.g.
		 * "implements" is parsed as an identifier.  When the function is
		 * later detected to be strict, the argument list must be rechecked
		 * against a larger set of reserved words (that of strict mode).
		 * This is handled by ``parse_function_body()``.  Here we recognize
		 * whatever tokens are considered reserved in current strictness
		 * (which is not always enough).
		 */

		if (comp_ctx->curr_token.t != DUK_TOK_IDENTIFIER) {
			DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "expected identifier");
		}
		DUK_ASSERT(comp_ctx->curr_token.t == DUK_TOK_IDENTIFIER);
		DUK_ASSERT(comp_ctx->curr_token.str1 != NULL);
		DUK_DDDPRINT("formal argument: %!O", comp_ctx->curr_token.str1);

		/* FIXME: append primitive */
		duk_push_hstring(ctx, comp_ctx->curr_token.str1);
		n = duk_get_length(ctx, comp_ctx->curr_func.argnames_idx);
		duk_put_prop_index(ctx, comp_ctx->curr_func.argnames_idx, n);

		advance(comp_ctx);  /* eat identifier */
	}
}

/* Parse a function-like expression, assuming that 'comp_ctx->curr_func' is
 * correctly set up.  Assumes that curr_token is just after 'function' (or
 * 'set'/'get' etc).
 */
static void parse_function_like_raw(duk_compiler_ctx *comp_ctx, int is_decl, int is_setget) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;

	DUK_ASSERT(comp_ctx->curr_func.num_formals == 0);
	DUK_ASSERT(comp_ctx->curr_func.is_function == 1);
	DUK_ASSERT(comp_ctx->curr_func.is_eval == 0);
	DUK_ASSERT(comp_ctx->curr_func.is_global == 0);
	DUK_ASSERT(comp_ctx->curr_func.is_setget == is_setget);
	DUK_ASSERT(comp_ctx->curr_func.is_decl == is_decl);

	/*
	 *  Function name (if any)
	 *
	 *  We don't check for prohibited names here, because we don't
	 *  yet know whether the function will be strict.  Function body
	 *  parsing handles this retroactively.
	 *
	 *  For function expressions and declarations function name must
	 *  be an Identifer (excludes reserved words).  For setter/getter
	 *  it is a PropertyName which allows reserved words and also
	 *  strings and numbers (e.g. "{ get 1() { ... } }").
	 */

	if (is_setget) {
		/* PropertyName -> IdentifierName | StringLiteral | NumericLiteral */
		if (comp_ctx->curr_token.t_nores == DUK_TOK_IDENTIFIER ||
		    comp_ctx->curr_token.t == DUK_TOK_STRING) {
			duk_push_hstring(ctx, comp_ctx->curr_token.str1);       /* keep in valstack */
		} else if (comp_ctx->curr_token.t == DUK_TOK_NUMBER) {
			duk_push_number(ctx, comp_ctx->curr_token.num);
			duk_to_string(ctx, -1);
		} else {
			DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid getter/setter name");
		}
		comp_ctx->curr_func.h_name = duk_get_hstring(ctx, -1);  /* borrowed reference */
		DUK_ASSERT(comp_ctx->curr_func.h_name != NULL);
		advance(comp_ctx);
	} else {
		/* Function name is an Identifier (not IdentifierName), but we get
		 * the raw name (not recognizing keywords) here and perform the name
		 * checks only after pass 1.
		 */
		if (comp_ctx->curr_token.t_nores == DUK_TOK_IDENTIFIER) {
			duk_push_hstring(ctx, comp_ctx->curr_token.str1);       /* keep in valstack */
			comp_ctx->curr_func.h_name = duk_get_hstring(ctx, -1);  /* borrowed reference */
			DUK_ASSERT(comp_ctx->curr_func.h_name != NULL);
			advance(comp_ctx);
		} else {
			/* valstack will be unbalanced, which is OK */
			DUK_ASSERT(!is_setget);
			if (is_decl) {
				DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "function name required");
			}
		}
	}

	DUK_DDDPRINT("function name: %!O", comp_ctx->curr_func.h_name);

	/*
	 *  Formal argument list
	 *
	 *  We don't check for prohibited names or for duplicate argument
	 *  names here, becase we don't yet know whether the function will
	 *  be strict.  Function body parsing handles this retroactively.
	 */

	advance_expect(comp_ctx, DUK_TOK_LPAREN);

	parse_function_formals(comp_ctx);

	DUK_ASSERT(comp_ctx->curr_token.t == DUK_TOK_RPAREN);
	advance(comp_ctx);

	/*
	 *  Parse function body
	 */

	parse_function_body(comp_ctx,
	                    0,   /* expect_eof */
	                    0);  /* implicit_return_value */

	/*
	 *  Convert duk_compiler_func to a function template and add it
	 *  to the parent function table.
	 */

	convert_to_function_template(comp_ctx);  /* -> [ ... func ] */
}

/* Parse an inner function, adding the function template to the current function's
 * function table.  Return a function number to be used by the outer function.
 */
static int parse_function_like_fnum(duk_compiler_ctx *comp_ctx, int is_decl, int is_setget) {
	duk_hthread *thr = comp_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_compiler_func old_func;
	int entry_top;
	int n_funcs;

	/*
	 *  Preliminaries: remember valstack top on entry (to restore it later);
	 *  switch to using a new function.
	 */

	entry_top = duk_get_top(ctx);
	memcpy(&old_func, &comp_ctx->curr_func, sizeof(duk_compiler_func));

	memset(&comp_ctx->curr_func, 0, sizeof(duk_compiler_func));
	init_function_valstack_slots(comp_ctx);
	DUK_ASSERT(comp_ctx->curr_func.num_formals == 0);

	/* inherit initial strictness from parent */
	comp_ctx->curr_func.is_strict = old_func.is_strict;

	comp_ctx->curr_func.is_function = 1;
	comp_ctx->curr_func.is_eval = 0;
	comp_ctx->curr_func.is_global = 0;
	comp_ctx->curr_func.is_setget = is_setget;
	comp_ctx->curr_func.is_decl = is_decl;

	parse_function_like_raw(comp_ctx, is_decl, is_setget);  /* pushes function template */

	/* FIXME: append primitive */
	n_funcs = duk_get_length(ctx, old_func.funcs_idx);

	/* FIXME: placeholder, catches most cases; this limit is actually too tight
	 * because CLOSURE can handle much more.
	 */
	if (n_funcs > 255) {
		DUK_ERROR(comp_ctx->thr, DUK_ERR_INTERNAL_ERROR, "out of funcs");
	}

	(void) duk_put_prop_index(ctx, old_func.funcs_idx, n_funcs);  /* autoincrements length */

	/*
	 *  Cleanup: restore original function, restore valstack state.
	 */
	
	memcpy(&comp_ctx->curr_func, &old_func, sizeof(duk_compiler_func));
	duk_set_top(ctx, entry_top);

	DUK_ASSERT_TOP(ctx, entry_top);

	return n_funcs;
}

#if 0
	/* FIXME: avoid two-pass parsing for parent functions both two passes
	 * (leading to fourfold parsing of second level functions)?  If we want
	 * do fancy parent variable lookups, the parent must be in its second
	 * pass for us to know all statically declared variables and functions.
	 * So perhaps do a single pass if parent is in its first pass, throw
	 * away the results and do two passes on parent's second pass.
	 */
#endif

/*
 *  Compile input string into an executable function template without
 *  arguments.
 *
 *  The string is parsed as the "Program" production of Ecmascript E5.
 *  Compilation context can be either global code or eval code (see E5
 *  Sections 14 and 15.1.2.1).
 *
 *  Input stack:  [ ... sourcecode ]
 *  Output stack: [ ... func_template ]
 */

/* FIXME: source code property */

void duk_js_compile(duk_hthread *thr, int flags) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *h_sourcecode;
	duk_compiler_ctx comp_ctx_alloc;
	duk_compiler_ctx *comp_ctx = &comp_ctx_alloc;
	duk_lexer_point lex_pt_alloc;
	duk_lexer_point *lex_pt = &lex_pt_alloc;
	duk_compiler_func *func = &comp_ctx_alloc.curr_func;
	int entry_top;
	int is_strict;
	int is_eval;
	int is_funcexpr;

	DUK_ASSERT(thr != NULL);

	is_eval = (flags & DUK_JS_COMPILE_FLAG_EVAL ? 1 : 0);
	is_strict = (flags & DUK_JS_COMPILE_FLAG_STRICT ? 1 : 0);
	is_funcexpr = (flags & DUK_JS_COMPILE_FLAG_FUNCEXPR ? 1 : 0);

	/*
	 *  Arguments check
	 */

	entry_top = duk_get_top(ctx);
	h_sourcecode = duk_require_hstring(ctx, -1);
	DUK_ASSERT(entry_top >= 1);

	/*
	 *  Init compiler and lexer contexts
	 */

	memset(comp_ctx, 0, sizeof(*comp_ctx));
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	comp_ctx->thr = NULL;
	comp_ctx->h_filename = NULL;
	comp_ctx->prev_token.str1 = NULL;
	comp_ctx->prev_token.str2 = NULL;
	comp_ctx->curr_token.str1 = NULL;
	comp_ctx->curr_token.str2 = NULL;
#endif

	duk_require_stack(ctx, COMPILE_ENTRY_SLOTS);

	duk_push_new_growable_buffer(ctx, 0);  /* entry_top + 0 */
	duk_push_undefined(ctx);               /* entry_top + 1 */
	duk_push_undefined(ctx);               /* entry_top + 2 */
	duk_push_undefined(ctx);               /* entry_top + 3 */
	duk_push_undefined(ctx);               /* entry_top + 4 */

	/* FIXME: h_filename */
	comp_ctx->thr = thr;
	comp_ctx->tok11_idx = entry_top + 1;
	comp_ctx->tok12_idx = entry_top + 2;
	comp_ctx->tok21_idx = entry_top + 3;
	comp_ctx->tok22_idx = entry_top + 4;
	comp_ctx->recursion_limit = DUK_COMPILER_RECURSION_LIMIT;

	DUK_LEXER_INITCTX(&comp_ctx->lex);   /* just zeroes/NULLs */
	comp_ctx->lex.thr = thr;
	comp_ctx->lex.input = DUK_HSTRING_GET_DATA(h_sourcecode);
	comp_ctx->lex.input_length = DUK_HSTRING_GET_BYTELEN(h_sourcecode);
	comp_ctx->lex.slot1_idx = comp_ctx->tok11_idx;
	comp_ctx->lex.slot2_idx = comp_ctx->tok12_idx;
	comp_ctx->lex.buf_idx = entry_top + 0;
	comp_ctx->lex.buf = (duk_hbuffer_growable *) duk_get_hbuffer(ctx, entry_top + 0);
	DUK_ASSERT(comp_ctx->lex.buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(comp_ctx->lex.buf));

#if 0  /* not needed */
	memset(lex_pt, 0, sizeof(*lex_pt));
#endif
	lex_pt->offset = 0;
	lex_pt->line = 1;
	DUK_LEXER_SETPOINT(&comp_ctx->lex, lex_pt);    /* fills window */

	/*
	 *  Initialize function state for a zero-argument function
	 */

	init_function_valstack_slots(comp_ctx);
	DUK_ASSERT(func->num_formals == 0);

	if (is_funcexpr) {
		/* funcexpr is now used for Function constructor, anonymous */
	} else {
		duk_push_hstring_stridx(ctx, (is_eval ? DUK_STRIDX_EVAL :
		                                        DUK_STRIDX_GLOBAL));
		func->h_name = duk_get_hstring(ctx, -1);
	}

	/*
	 *  Parse a function body or a function-like expression, depending
	 *  on flags.
	 */

	func->is_strict = is_strict;
	func->is_setget = 0;
	func->is_decl = 0;

	if (is_funcexpr) {
		func->is_function = 1;
		func->is_eval = 0;
		func->is_global = 0;

		advance(comp_ctx);  /* init 'curr_token' */
		advance_expect(comp_ctx, DUK_TOK_FUNCTION);
		(void) parse_function_like_raw(comp_ctx,
		                               0,      /* is_decl */
		                               0);     /* is_setget */
	} else {
		func->is_function = 0;
		func->is_eval = is_eval;
		func->is_global = !is_eval;

		parse_function_body(comp_ctx,
		                    1,             /* expect_eof */
		                    1);            /* implicit_return_value */
	}

	/*
	 *  Convert duk_compiler_func to a function template
	 */

	convert_to_function_template(comp_ctx);

	/*
	 *  Mangle stack for result
	 */

	/* [ ... sourcecode (temps) func ] */

	DUK_ASSERT(entry_top - 1 >= 0);
	duk_replace(ctx, entry_top - 1);  /* replace sourcecode with func */
	duk_set_top(ctx, entry_top);

	/* [ ... func ] */

	DUK_ASSERT_TOP(ctx, entry_top);
}

