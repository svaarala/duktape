/*
 *  Heap compiled function (Ecmascript function) representation.
 *
 *  There is a single data buffer containing the Ecmascript function's
 *  bytecode, constants, and inner functions.
 */

#ifndef DUK_HCOMPILEDFUNCTION_H_INCLUDED
#define DUK_HCOMPILEDFUNCTION_H_INCLUDED

/*
 *  Accessor macros for function specific data areas
 */

/* Note: assumes 'data' is always a fixed buffer */
#define DUK_HCOMPILEDFUNCTION_GET_BUFFER_BASE(h)  \
	DUK_HBUFFER_FIXED_GET_DATA_PTR((duk_hbuffer_fixed *) (h)->data)

#define DUK_HCOMPILEDFUNCTION_GET_CONSTS_BASE(h)  \
	((duk_tval *) DUK_HCOMPILEDFUNCTION_GET_BUFFER_BASE((h)))

#define DUK_HCOMPILEDFUNCTION_GET_FUNCS_BASE(h)  \
	((h)->funcs)

#define DUK_HCOMPILEDFUNCTION_GET_CODE_BASE(h)  \
	((h)->bytecode)

#define DUK_HCOMPILEDFUNCTION_GET_CONSTS_END(h)  \
	((duk_tval *) DUK_HCOMPILEDFUNCTION_GET_FUNCS_BASE((h)))

#define DUK_HCOMPILEDFUNCTION_GET_FUNCS_END(h)  \
	((duk_hobject **) DUK_HCOMPILEDFUNCTION_GET_CODE_BASE((h)))

#define DUK_HCOMPILEDFUNCTION_GET_CODE_END(h)  \
	((duk_instr_t *) (DUK_HBUFFER_FIXED_GET_DATA_PTR((duk_hbuffer_fixed *) (h)->data) + \
	                DUK_HBUFFER_GET_SIZE((h)->data)))

#define DUK_HCOMPILEDFUNCTION_GET_CONSTS_SIZE(h)  \
	( \
	 (duk_size_t) \
	 ( \
	   ((duk_uint8_t *) DUK_HCOMPILEDFUNCTION_GET_CONSTS_END((h))) - \
	   ((duk_uint8_t *) DUK_HCOMPILEDFUNCTION_GET_CONSTS_BASE((h))) \
	 ) \
	)

#define DUK_HCOMPILEDFUNCTION_GET_FUNCS_SIZE(h)  \
	( \
	 (duk_size_t) \
	 ( \
	   ((duk_uint8_t *) DUK_HCOMPILEDFUNCTION_GET_FUNCS_END((h))) - \
	   ((duk_uint8_t *) DUK_HCOMPILEDFUNCTION_GET_FUNCS_BASE((h))) \
	 ) \
	)

#define DUK_HCOMPILEDFUNCTION_GET_CODE_SIZE(h)  \
	( \
	 (duk_size_t) \
	 ( \
	   ((duk_uint8_t *) DUK_HCOMPILEDFUNCTION_GET_CODE_END((h))) - \
	   ((duk_uint8_t *) DUK_HCOMPILEDFUNCTION_GET_CODE_BASE((h))) \
	 ) \
	)

#define DUK_HCOMPILEDFUNCTION_GET_CONSTS_COUNT(h)  \
	((duk_size_t) (DUK_HCOMPILEDFUNCTION_GET_CONSTS_SIZE((h)) / sizeof(duk_tval)))

#define DUK_HCOMPILEDFUNCTION_GET_FUNCS_COUNT(h)  \
	((duk_size_t) (DUK_HCOMPILEDFUNCTION_GET_FUNCS_SIZE((h)) / sizeof(duk_hobject *)))

#define DUK_HCOMPILEDFUNCTION_GET_CODE_COUNT(h)  \
	((duk_size_t) (DUK_HCOMPILEDFUNCTION_GET_CODE_SIZE((h)) / sizeof(duk_instr_t)))


/*
 *  Main struct
 */

struct duk_hcompiledfunction {
	/* shared object part */
	duk_hobject obj;

	/*
	 *  Pointers to function data area for faster access.  Function
	 *  data is a buffer shared between all closures of the same
	 *  "template" function.  The data buffer is always fixed (non-
	 *  dynamic, hence stable), with a layout as follows:
	 *
	 *    constants (duk_tval)
	 *    inner functions (duk_hobject *)
	 *    bytecode (duk_instr_t)
	 *
	 *  Note: bytecode end address can be computed from 'data' buffer
	 *  size.  It is not strictly necessary functionally, assuming
	 *  bytecode never jumps outside its allocated area.  However,
	 *  it's a safety/robustness feature for avoiding the chance of
	 *  executing random data as bytecode due to a compiler error.
	 *
	 *  Note: values in the data buffer must be incref'd (they will
	 *  be decref'd on release) for every compiledfunction referring
	 *  to the 'data' element.
	 */

	duk_hbuffer *data;    /* data area, fixed allocation, stable data ptrs */

	/* no need for constants pointer */
	duk_hobject **funcs;
	duk_instr_t *bytecode;

	/*
	 *  'nregs' registers are allocated on function entry, at most 'nargs'
	 *  are initialized to arguments, and the rest to undefined.  Arguments
	 *  above 'nregs' are not mapped to registers.  All registers in the
	 *  active stack range must be initialized because they are GC reachable.
	 *  'nargs' is needed so that if the function is given more than 'nargs'
	 *  arguments, the additional arguments do not 'clobber' registers
	 *  beyond 'nregs' which must be consistently initialized to undefined.
	 *
	 *  Usually there is no need to know which registers are mapped to
	 *  local variables.  Registers may be allocated to variable in any
	 *  way (even including gaps).  However, a register-variable mapping
	 *  must be the same for the duration of the function execution and
	 *  the register cannot be used for anything else.
	 *
	 *  When looking up variables by name, the '_Varmap' map is used.
	 *  When an activation closes, registers mapped to arguments are
	 *  copied into the environment record based on the same map.  The
	 *  reverse map (from register to variable) is not currently needed
	 *  at run time, except for debugging, so it is not maintained.
	 */

	duk_uint16_t nregs;                /* regs to allocate */
	duk_uint16_t nargs;                /* number of arguments allocated to regs */

	/*
	 *  Additional control information is placed into the object itself
	 *  as internal properties to avoid unnecessary fields for the
	 *  majority of functions.  The compiler tries to omit internal
	 *  control fields when possible.
	 *
	 *  Function templates:
	 *
	 *    {
	 *      name: "func",    // declaration, named function expressions
	 *      fileName: <debug info for creating nice errors>
	 *      _Varmap: { "arg1": 0, "arg2": 1, "varname": 2 },
	 *      _Formals: [ "arg1", "arg2" ],
	 *      _Source: "function func(arg1, arg2) { ... }",
	 *      _Pc2line: <debug info for pc-to-line mapping>,
	 *    }
	 *
	 *  Function instances:
	 *
	 *    {
	 *      length: 2,
	 *      prototype: { constructor: <func> },
	 *      caller: <thrower>,
	 *      arguments: <thrower>,
	 *      name: "func",    // declaration, named function expressions
	 *      fileName: <debug info for creating nice errors>
	 *      _Varmap: { "arg1": 0, "arg2": 1, "varname": 2 },
	 *      _Formals: [ "arg1", "arg2" ],
	 *      _Source: "function func(arg1, arg2) { ... }",
	 *      _Pc2line: <debug info for pc-to-line mapping>,
	 *      _Varenv: <variable environment of closure>,
	 *      _Lexenv: <lexical environment of closure (if differs from _Varenv)>
	 *    }
	 *
	 *  More detailed description of these properties can be found
	 *  in the documentation.
	 */
};

#endif  /* DUK_HCOMPILEDFUNCTION_H_INCLUDED */
