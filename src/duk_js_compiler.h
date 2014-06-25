/*
 *  Ecmascript compiler.
 */

#ifndef DUK_JS_COMPILER_H_INCLUDED
#define DUK_JS_COMPILER_H_INCLUDED

/* ecmascript compiler limits */
#if defined(DUK_USE_DEEP_C_STACK)
#define DUK_COMPILER_RECURSION_LIMIT       2500
#else
#define DUK_COMPILER_RECURSION_LIMIT       50
#endif
#define DUK_COMPILER_TOKEN_LIMIT           100000000  /* 1e8: protects against deeply nested inner functions */

/* maximum loopcount for peephole optimization */
#define DUK_COMPILER_PEEPHOLE_MAXITER      3

/* maximum bytecode length in instructions */
#define DUK_COMPILER_MAX_BYTECODE_LENGTH   (256 * 1024 * 1024)  /* 1 GB */

/*
 *  Compiler intermediate values
 *
 *  Intermediate values describe either plain values (e.g. strings or
 *  numbers) or binary operations which have not yet been coerced into
 *  either a left-hand-side or right-hand-side role (e.g. object property).
 */

#define DUK_IVAL_NONE          0   /* no value */
#define DUK_IVAL_PLAIN         1   /* register, constant, or value */
#define DUK_IVAL_ARITH         2   /* binary arithmetic; DUK_OP_ADD, DUK_OP_EQ, other binary ops */
#define DUK_IVAL_PROP          3   /* property access */
#define DUK_IVAL_VAR           4   /* variable access */

#define DUK_ISPEC_NONE         0   /* no value */
#define DUK_ISPEC_VALUE        1   /* value resides in 'valstack_idx' */
#define DUK_ISPEC_REGCONST     2   /* value resides in a register or constant */

/* bit mask which indicates that a regconst is a constant instead of a register */
#define DUK_JS_CONST_MARKER    0x80000000

typedef struct {
	int t;                      /* DUK_ISPEC_XXX */
	int regconst;
	int valstack_idx;           /* always set; points to a reserved valstack slot */
} duk_ispec;

typedef struct {
	/*
	 *  PLAIN: x1
	 *  ARITH: x1 <op> x2
	 *  PROP: x1.x2
	 *  VAR: x1 (name)
	 */

	int t;                      /* DUK_IVAL_XXX */
	int op;                     /* bytecode opcode for binary ops */
	duk_ispec x1;
	duk_ispec x2;
} duk_ivalue;

/*
 *  Bytecode instruction representation during compilation
 *
 *  Contains the actual instruction and (optionally) debug info.
 */

struct duk_compiler_instr {
	duk_instr ins;
#if defined(DUK_USE_PC2LINE)
	duk_uint32_t line;
#endif
};

/*
 *  Compiler state
 */

#define DUK_LABEL_FLAG_ALLOW_BREAK       (1 << 0)
#define DUK_LABEL_FLAG_ALLOW_CONTINUE    (1 << 1)

#define DUK_DECL_TYPE_VAR                0
#define DUK_DECL_TYPE_FUNC               1

/* FIXME: optimize to 16 bytes */
typedef struct {
	int flags;
	int label_id;           /* numeric label_id */
	duk_hstring *h_label;   /* borrowed label name */
	int catch_depth;        /* catch depth at point of definition */
	int pc_label;           /* pc of label statement:
	                         * pc+1: break jump site
	                         * pc+2: continue jump site
	                         */

	/* Fast jumps (which avoid longjmp) jump directly to the jump sites
	 * which are always known even while the iteration/switch statement
	 * is still being parsed.  A final peephole pass "straightens out"
	 * the jumps.
	 */
} duk_labelinfo;

/* Compiling state of one function, eventually converted to duk_hcompiledfunction */
struct duk_compiler_func {
	/* These pointers are at the start of the struct so that they pack
	 * nicely.  Mixing pointers and integer values is bad on some
	 * platforms (e.g. if int is 32 bits and pointers are 64 bits).
	 */

	duk_hstring *h_name;                /* function name (borrowed reference), ends up in _name */
	duk_hbuffer_dynamic *h_code;        /* C array of duk_compiler_instr */
	duk_hobject *h_consts;              /* array */
	duk_hobject *h_funcs;               /* array of function templates: [func1, offset1, line1, func2, offset2, line2]
	                                     * offset/line points to closing brace to allow skipping on pass 2
	                                     */
	duk_hobject *h_decls;               /* array of declarations: [ name1, val1, name2, val2, ... ]
	                                     * valN = (typeN) | (fnum << 8), where fnum is inner func number (0 for vars)
	                                     * record function and variable declarations in pass 1
	                                     */
	duk_hobject *h_labelnames;          /* array of active label names */
	duk_hbuffer_dynamic *h_labelinfos;  /* C array of duk_labelinfo */
	duk_hobject *h_argnames;            /* array of formal argument names (-> _formals) */
	duk_hobject *h_varmap;              /* variable map for pass 2 (identifier -> register number or null (unmapped)) */

	int is_function;                    /* is an actual function (not global/eval code) */
	int is_eval;                        /* is eval code */
	int is_global;                      /* is global code */
	int is_setget;                      /* is a setter/getter */
	int is_decl;                        /* is a function declaration (as opposed to function expression) */
	int is_strict;                      /* function is strict */
	int is_notail;                      /* function must not be tailcalled */
	int in_directive_prologue;          /* parsing in "directive prologue", recognize directives */
	int in_scanning;                    /* parsing in "scanning" phase (first pass) */
	int may_direct_eval;                /* function may call direct eval */
	int id_access_arguments;            /* function refers to 'arguments' identifier */
	int id_access_slow;                 /* function makes one or more slow path accesses */
	int is_arguments_shadowed;          /* argument/function declaration shadows 'arguments' */
	int needs_shuffle;                  /* function needs shuffle registers */
	int num_formals;                    /* number of formal arguments */
	int reg_stmt_value;                 /* register for writing value of 'non-empty' statements (global or eval code) */

	int reject_regexp_in_adv;           /* reject RegExp literal on next advance() call; needed for handling IdentifierName productions */

	int code_idx;
	int consts_idx;
	int funcs_idx;
	int fnum_next;
	int decls_idx;
	int labelnames_idx;
	int labelinfos_idx;
	int argnames_idx;
	int varmap_idx;

	/* temp reg handling */
	int temp_first;                     /* first register that is a temporary (below: variables) */
	int temp_next;                      /* next temporary register to allocate */
	int temp_max;                       /* highest value of temp_reg (temp_max - 1 is highest used reg) */

	/* shuffle registers if large number of regs/consts */
	int shuffle1;
	int shuffle2;
	int shuffle3;

	/* statement id allocation (running counter) */
	int stmt_next;

	/* label handling */
	int label_next;

	/* catch stack book-keeping */
	int catch_depth;                    /* catch stack depth */

	/* with stack book-keeping (affects identifier lookups) */
	int with_depth;

	/* stats for current expression being parsed */
	int nud_count;
	int led_count;
	int paren_level;                    /* parenthesis count, 0 = top level */
	int expr_lhs;                       /* expression is left-hand-side compatible */
	int allow_in;                       /* current paren level allows 'in' token */
};

struct duk_compiler_ctx {
	duk_hthread *thr;

	/* filename being compiled (ends up in functions' '_filename' property) */
	duk_hstring *h_filename;            /* borrowed reference */

	/* lexing (tokenization) state (contains two valstack slot indices) */
	duk_lexer_ctx lex;

	/* current and previous token for parsing */
	duk_token prev_token;
	duk_token curr_token;
	int tok11_idx;                      /* curr_token slot1 (matches 'lex' slot1_idx) */
	int tok12_idx;                      /* curr_token slot2 (matches 'lex' slot2_idx) */
	int tok21_idx;                      /* prev_token slot1 */
	int tok22_idx;                      /* prev_token slot2 */

	/* recursion limit */
	int recursion_depth;
	int recursion_limit;

	/* current function being compiled (embedded instead of pointer for more compact access) */
	duk_compiler_func curr_func;
};

/*
 *  Prototypes
 */

#define DUK_JS_COMPILE_FLAG_EVAL      (1 << 0)  /* source is eval code (not program) */
#define DUK_JS_COMPILE_FLAG_STRICT    (1 << 1)  /* strict outer context */
#define DUK_JS_COMPILE_FLAG_FUNCEXPR  (1 << 2)  /* source is a function expression (used for Function constructor) */

void duk_js_compile(duk_hthread *thr, const duk_uint8_t *src_buffer, duk_size_t src_length, duk_small_int_t flags);

#endif  /* DUK_JS_COMPILER_H_INCLUDED */

