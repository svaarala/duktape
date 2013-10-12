/*
 *  Heap thread object representation.
 *
 *  duk_hthread is also the 'context' (duk_context) for exposed APIs
 *  which mostly operate on the topmost frame of the value stack.
 */

#ifndef DUK_HTHREAD_H_INCLUDED
#define DUK_HTHREAD_H_INCLUDED

/*
 *  Stack constants
 */

#define  DUK_VALSTACK_GROW_STEP          128     /* roughly 1 kiB */
#define  DUK_VALSTACK_SHRINK_THRESHOLD   256     /* roughly 2 kiB */
#define  DUK_VALSTACK_SHRINK_SPARE       64      /* roughly 0.5 kiB */
#define  DUK_VALSTACK_INITIAL_SIZE       64      /* roughly 0.5 kiB -> but rounds up to DUK_VALSTACK_GROW_STEP in practice */
#define  DUK_VALSTACK_INTERNAL_EXTRA     64      /* internal extra elements assumed on function entry,
                                                  * always added to user-defined 'extra' for e.g. the
                                                  * duk_check_stack() call.
                                                  */
#define  DUK_VALSTACK_DEFAULT_MAX        1000000

#define  DUK_CALLSTACK_GROW_STEP         8       /* roughly 256 bytes */
#define  DUK_CALLSTACK_SHRINK_THRESHOLD  16      /* roughly 512 bytes */
#define  DUK_CALLSTACK_SHRINK_SPARE      8       /* roughly 256 bytes */
#define  DUK_CALLSTACK_INITIAL_SIZE      8
#define  DUK_CALLSTACK_DEFAULT_MAX       10000

#define  DUK_CATCHSTACK_GROW_STEP         4      /* roughly 64 bytes */
#define  DUK_CATCHSTACK_SHRINK_THRESHOLD  8      /* roughly 128 bytes */
#define  DUK_CATCHSTACK_SHRINK_SPARE      4      /* roughly 64 bytes */
#define  DUK_CATCHSTACK_INITIAL_SIZE      4
#define  DUK_CATCHSTACK_DEFAULT_MAX       10000

/*
 *  Activation defines
 */

#define  DUK_ACT_FLAG_STRICT          (1 << 0)  /* function executes in strict mode */
#define  DUK_ACT_FLAG_TAILCALLED      (1 << 1)  /* activation has tailcalled one or more times */
#define  DUK_ACT_FLAG_CONSTRUCT       (1 << 2)  /* function executes as a constructor (called via "new") */
#define  DUK_ACT_FLAG_PREVENT_YIELD   (1 << 3)  /* activation prevents yield (native call or "new") */
#define  DUK_ACT_FLAG_DIRECT_EVAL     (1 << 4)  /* activation is a direct eval call */

/*
 *  Catcher defines
 */

/* flags field: LLLLLLFT, L = label (24 bits), F = flags (4 bits), T = type (4 bits) */
#define  DUK_CAT_TYPE_MASK            0x0000000f
#define  DUK_CAT_TYPE_BITS            4
#define  DUK_CAT_LABEL_MASK           0xffffff00
#define  DUK_CAT_LABEL_BITS           24
#define  DUK_CAT_LABEL_SHIFT          8

#define  DUK_CAT_FLAG_CATCH_ENABLED          (1 << 4)   /* catch part will catch */
#define  DUK_CAT_FLAG_FINALLY_ENABLED        (1 << 5)   /* finally part will catch */
#define  DUK_CAT_FLAG_CATCH_BINDING_ENABLED  (1 << 6)   /* request to create catch binding */
#define  DUK_CAT_FLAG_LEXENV_ACTIVE          (1 << 7)   /* catch or with binding is currently active */

#define  DUK_CAT_TYPE_UNKNOWN         0
#define  DUK_CAT_TYPE_TCF             1
#define  DUK_CAT_TYPE_LABEL           2

#define  DUK_CAT_GET_TYPE(c)          ((c)->flags & DUK_CAT_TYPE_MASK)
#define  DUK_CAT_GET_LABEL(c)         (((c)->flags & DUK_CAT_LABEL_MASK) >> DUK_CAT_LABEL_SHIFT)

#define  DUK_CAT_HAS_CATCH_ENABLED(c)           ((c)->flags & DUK_CAT_FLAG_CATCH_ENABLED)
#define  DUK_CAT_HAS_FINALLY_ENABLED(c)         ((c)->flags & DUK_CAT_FLAG_FINALLY_ENABLED)
#define  DUK_CAT_HAS_CATCH_BINDING_ENABLED(c)   ((c)->flags & DUK_CAT_FLAG_CATCH_BINDING_ENABLED)
#define  DUK_CAT_HAS_LEXENV_ACTIVE(c)           ((c)->flags & DUK_CAT_FLAG_LEXENV_ACTIVE)

#define  DUK_CAT_SET_CATCH_ENABLED(c)    do { \
		(c)->flags |= DUK_CAT_FLAG_CATCH_ENABLED; \
	} while (0)
#define  DUK_CAT_SET_FINALLY_ENABLED(c)  do { \
		(c)->flags |= DUK_CAT_FLAG_FINALLY_ENABLED; \
	} while (0)
#define  DUK_CAT_SET_CATCH_BINDING_ENABLED(c)    do { \
		(c)->flags |= DUK_CAT_FLAG_CATCH_BINDING_ENABLED; \
	} while (0)
#define  DUK_CAT_SET_LEXENV_ACTIVE(c)    do { \
		(c)->flags |= DUK_CAT_FLAG_LEXENV_ACTIVE; \
	} while (0)

#define  DUK_CAT_CLEAR_CATCH_ENABLED(c)    do { \
		(c)->flags &= ~DUK_CAT_FLAG_CATCH_ENABLED; \
	} while (0)
#define  DUK_CAT_CLEAR_FINALLY_ENABLED(c)  do { \
		(c)->flags &= ~DUK_CAT_FLAG_FINALLY_ENABLED; \
	} while (0)
#define  DUK_CAT_CLEAR_CATCH_BINDING_ENABLED(c)    do { \
		(c)->flags &= ~DUK_CAT_FLAG_CATCH_BINDING_ENABLED; \
	} while (0)
#define  DUK_CAT_CLEAR_LEXENV_ACTIVE(c)    do { \
		(c)->flags &= ~DUK_CAT_FLAG_LEXENV_ACTIVE; \
	} while (0)

/*
 *  Thread defines
 */

#define  DUK_HTHREAD_GET_STRING(thr,idx)          ((thr)->strs[(idx)])

#define  DUK_HTHREAD_GET_CURRENT_ACTIVATION(thr)  (&(thr)->callstack[(thr)->callstack_top - 1])

/* values for the state field */
#define  DUK_HTHREAD_STATE_INACTIVE     1   /* thread not currently running */
#define  DUK_HTHREAD_STATE_RUNNING      2   /* thread currently running (only one at a time) */
#define  DUK_HTHREAD_STATE_RESUMED      3   /* thread resumed another thread (active but not running) */
#define  DUK_HTHREAD_STATE_YIELDED      4   /* thread has yielded */
#define  DUK_HTHREAD_STATE_TERMINATED   5   /* thread has terminated */

/*
 *  Struct defines
 */

/* Note: it's nice if size is 2^N (now 32 bytes on 32 bit) */
struct duk_activation {
	int flags;
	duk_hobject *func;      /* function being executed; for bound function calls, this is the final, real function */
	duk_hobject *var_env;   /* current variable environment (may be NULL if delayed) */
	duk_hobject *lex_env;   /* current lexical environment (may be NULL if delayed) */
	int pc;                 /* next instruction to execute */

	/* Current 'this' binding is the value just below idx_bottom */

	/* These following are only used for book-keeping of Ecmascript-initiated
	 * calls, to allow returning to an Ecmascript function properly.
	 *
	 * Note: idx_bottom is always set, while idx_retval is only applicable for
	 * activations below the topmost one.  Currently idx_retval for the topmost
	 * activation is considered garbage (and it not initialized on entry or
	 * cleared on return; may contain previous or garbage values).
	 */

	int idx_bottom;         /* Bottom of valstack for this activation, used to reset
	                         * valstack_bottom on return; index is absolute.
	                         * Note: idx_top not needed because top is set to 'nregs'
	                         * always when returning to an Ecmascript activation.
	                         */
	int idx_retval;         /* Return value when returning to this activation
	                         * (points to caller reg, not callee reg); index is absolute
	                         * Note: only set if activation is -not topmost-.
	                         */

	/* Note: earlier, 'this' binding was handled with an index to the
	 * (calling) valstack.  This works for everything except tail
	 * calls, which must not "cumulate" valstack temps.
	 */

	int unused1;  /* pad to 2^N */
};

/* Note: it's nice if size is 2^N (not 4x4 = 16 bytes on 32 bit) */
struct duk_catcher {
	int flags;               /* type and control flags */
	int callstack_index;     /* callstack index of related activation */
	int pc_base;             /* resume execution from pc_base or pc_base+1 */
	int idx_base;            /* idx_base and idx_base+1 get completion value and type */
	duk_hstring *h_varname;  /* borrowed reference to catch variable name (or NULL if none) */
	                         /* (reference is valid as long activation exists) */
};

struct duk_hthread {
	/* shared object part */
	duk_hobject obj;

	/* backpointers */
	duk_heap *heap;

	/* current strictness flag: affects API calls */
	duk_u8 strict;
	duk_u8 state;
	duk_u8 unused1;
	duk_u8 unused2;

	/* sanity limits */
	int valstack_max;
	int callstack_max;
	int catchstack_max;

	/* XXX: valstack, callstack, and catchstack are currently assumed
	 * to have non-NULL pointers.  Relaxing this would not lead to big
	 * benefits (except perhaps for terminated threads).
	 */

	/* value stack: these are expressed as pointers for faster stack manipulation */
	duk_tval *valstack;			/* start of valstack allocation */
	duk_tval *valstack_end;			/* end of valstack allocation (exclusive) */
	duk_tval *valstack_bottom;		/* bottom of current frame */
	duk_tval *valstack_top;			/* top of current frame (exclusive) */

	/* call stack */
	duk_activation *callstack;
	unsigned int callstack_size;		/* allocation size */
	unsigned int callstack_top;		/* next to use, highest used is top - 1 */
	unsigned int callstack_preventcount;	/* number of activation records in callstack preventing a yield */

	/* catch stack */
	duk_catcher *catchstack;
	unsigned int catchstack_size;		/* allocation size */
	unsigned int catchstack_top;		/* next to use, highest used is top - 1 */

	/* yield/resume book-keeping */
	duk_hthread *resumer;			/* who resumed us (if any) */

	/* Builtin-objects; may be shared with other threads, copies of
	 * pointers in duk_heap.  This is rather expensive, currently
	 * 38x4 = 152 bytes.
	 */
	duk_hobject *builtins[DUK_NUM_BUILTINS];

	/* convenience copies from heap/vm for faster access */
	duk_hstring **strs;			/* (from duk_heap) */
};

/*
 *  Prototypes
 */

void duk_hthread_copy_builtin_objects(duk_hthread *thr_from, duk_hthread *thr_to);
void duk_hthread_create_builtin_objects(duk_hthread *thr);
int duk_hthread_init_stacks(duk_heap *heap, duk_hthread *thr);
void duk_hthread_terminate(duk_hthread *thr);

void duk_hthread_callstack_grow(duk_hthread *thr);
void duk_hthread_callstack_shrink_check(duk_hthread *thr);
void duk_hthread_callstack_unwind(duk_hthread *thr, int new_top);
void duk_hthread_catchstack_grow(duk_hthread *thr);
void duk_hthread_catchstack_shrink_check(duk_hthread *thr);
void duk_hthread_catchstack_unwind(duk_hthread *thr, int new_top);

duk_activation *duk_hthread_get_current_activation(duk_hthread *thr);

#endif  /* DUK_HTHREAD_H_INCLUDED */

