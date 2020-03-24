/*
 *  Type declarations and initializers for ROM objects (duk_hobject).
 */

'use strict';

const { propDefault, findObjectById } = require('../metadata/util');
const { hexDecode } = require('../../util/hex');
const { numberToDoubleHex } = require('../../util/double');
const { assert } = require('../../util/assert');

function emitRefcountInitMacro(genc) {
    genc.emitLine('#if defined(DUK_USE_ASSERTIONS)');
    genc.emitLine('#define DUK__REFCINIT(refc) 0 /*h_assert_refcount*/, (refc) /*actual*/');
    genc.emitLine('#else');
    genc.emitLine('#define DUK__REFCINIT(refc) (refc) /*actual*/');
    genc.emitLine('#endif');
    genc.emitLine('');
}
exports.emitRefcountInitMacro = emitRefcountInitMacro;

function emit32BitPtrCheck(genc) {
    genc.emitLine('#undef DUK__32BITPTR');
    genc.emitLine('#if defined(DUK_UINTPTR_MAX)');
    genc.emitLine('#if (DUK_UINTPTR_MAX <= 0xffffffffUL)');
    genc.emitLine('#define DUK__32BITPTR');
    genc.emitLine('#endif');
    genc.emitLine('#endif');
}
exports.emit32BitPtrCheck = emit32BitPtrCheck;

function emitObjectTypedefs(genc) {
    // Objects and functions are straightforward because they just use the
    // RAM structure which has no dynamic or variable size parts.
    genc.emitLine('typedef struct duk_romobj duk_romobj; struct duk_romobj { duk_hobject hdr; };');
    genc.emitLine('typedef struct duk_romarr duk_romarr; struct duk_romarr { duk_harray hdr; };');
    genc.emitLine('typedef struct duk_romfun duk_romfun; struct duk_romfun { duk_hnatfunc hdr; };');
    genc.emitLine('typedef struct duk_romobjenv duk_romobjenv; struct duk_romobjenv { duk_hobjenv hdr; };');
}

function emitObjectInitializersPtrComp(genc) {
    genc.emitLine('#if !defined(DUK_USE_REFCOUNT16) || defined(DUK_USE_HOBJECT_HASH_PART)');
    genc.emitLine('#error currently assumes DUK_USE_HEAPPTR16 and DUK_USE_REFCOUNT16 are both defined and DUK_USE_HOBJECT_HASH_PART is undefined');
    genc.emitLine('#endif');
    //genc.emitLine('#if !defined(DUK_USE_HEAPPTR_ENC16_STATIC)');
    //genc.emitLine('#error need DUK_USE_HEAPPTR_ENC16_STATIC which provides compile-time pointer compression');
    //genc.emitLine('#endif');

    genc.emitLine('#define DUK__ROMOBJ_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize) ' +
                  ' { { { (heaphdr_flags), DUK__REFCINIT((refcount)), 0, 0, (props_enc16) }, (iproto_enc16), (esize), (enext), (asize) } }');

    genc.emitLine('#define DUK__ROMARR_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize,length) ' +
                  ' { { { { (heaphdr_flags), DUK__REFCINIT((refcount)), 0, 0, (props_enc16) }, (iproto_enc16), (esize), (enext), (asize) }, (length), 0 /*length_nonwritable*/ } }');

    genc.emitLine('#define DUK__ROMFUN_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize,nativefunc,nargs,magic) ' +
                  ' { { { { (heaphdr_flags), DUK__REFCINIT((refcount)), 0, 0, (props_enc16) }, (iproto_enc16), (esize), (enext), (asize) }, (nativefunc), (duk_int16_t) (nargs), (duk_int16_t) (magic) } }');

    genc.emitLine('#define DUK__ROMOBJENV_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize,target,has_this) ' +
                  ' { { { { (heaphdr_flags), DUK__REFCINIT((refcount)), 0, 0, (props_enc16) }, (iproto_enc16), (esize), (enext), (asize) }, (duk_hobject *) DUK_LOSE_CONST(target), (has_this) } }');
}

function emitObjectInitializersNoPtrComp(genc) {
    genc.emitLine('#define DUK__ROMOBJ_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize) ' +
                  ' { { { (heaphdr_flags), DUK__REFCINIT((refcount)), NULL, NULL }, (duk_uint8_t *) DUK_LOSE_CONST(props), (duk_hobject *) DUK_LOSE_CONST(iproto), (esize), (enext), (asize), (hsize) } }');

    genc.emitLine('#define DUK__ROMARR_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize,length) ' +
                  ' { { { { (heaphdr_flags), DUK__REFCINIT((refcount)), NULL, NULL }, (duk_uint8_t *) DUK_LOSE_CONST(props), (duk_hobject *) DUK_LOSE_CONST(iproto), (esize), (enext), (asize), (hsize) }, (length), 0 /*length_nonwritable*/ } }');

    genc.emitLine('#define DUK__ROMFUN_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize,nativefunc,nargs,magic) ' +
                  ' { { { { (heaphdr_flags), DUK__REFCINIT((refcount)), NULL, NULL }, (duk_uint8_t *) DUK_LOSE_CONST(props), (duk_hobject *) DUK_LOSE_CONST(iproto), (esize), (enext), (asize), (hsize) }, (nativefunc), (duk_int16_t) (nargs), (duk_int16_t) (magic) } }');

    genc.emitLine('#define DUK__ROMOBJENV_INIT(heaphdr_flags,refcount,props,props_enc16,iproto,iproto_enc16,esize,enext,asize,hsize,target,has_this) ' +
                  ' { { { { (heaphdr_flags), DUK__REFCINIT((refcount)), NULL, NULL }, (duk_uint8_t *) DUK_LOSE_CONST(props), (duk_hobject *) DUK_LOSE_CONST(iproto), (esize), (enext), (asize), (hsize) }, (duk_hobject *) DUK_LOSE_CONST(target), (has_this) } }');
}

function emitObjectInitializers(genc) {
    // Emit object/function initializer which is aware of options affecting
    // the header.  Heap next/prev pointers are always NULL.

    genc.emitLine('#if defined(DUK_USE_HEAPPTR16)');
    emitObjectInitializersPtrComp(genc);
    genc.emitLine('#else  /* DUK_USE_HEAPPTR16 */');
    emitObjectInitializersNoPtrComp(genc);
    genc.emitLine('#endif  /* DUK_USE_HEAPPTR16 */');
}

function emitFunctionPointerTypedef(genc) {
    // Initializer typedef for a dummy function pointer.  ROM support assumes
    // function pointers are the same size as void *.  Using a dummy function
    // pointer type avoids function pointer to normal pointer cast which emits
    // warnings.

    genc.emitLine('typedef void (*duk_rom_funcptr)(void);');
}

function emitTvalStructsPackedTval(genc) {
    genc.emitLine('#if !defined(DUK__32BITPTR)');
    genc.emitLine('#error internal error, packed duk_tval requires 32-bit pointers');
    genc.emitLine('#endif');

    genc.emitLine('typedef struct duk_rom_tval_undefined duk_rom_tval_undefined;');
    genc.emitLine('typedef struct duk_rom_tval_null duk_rom_tval_null;');
    genc.emitLine('typedef struct duk_rom_tval_lightfunc duk_rom_tval_lightfunc;');
    genc.emitLine('typedef struct duk_rom_tval_boolean duk_rom_tval_boolean;');
    genc.emitLine('typedef struct duk_rom_tval_number duk_rom_tval_number;');
    genc.emitLine('typedef struct duk_rom_tval_object duk_rom_tval_object;');
    genc.emitLine('typedef struct duk_rom_tval_string duk_rom_tval_string;');
    genc.emitLine('typedef struct duk_rom_tval_accessor duk_rom_tval_accessor;');
    genc.emitLine('struct duk_rom_tval_number { duk_uint8_t bytes[8]; };');
    genc.emitLine('struct duk_rom_tval_accessor { const duk_hobject *get; const duk_hobject *set; };');

    genc.emitLine('#if defined(DUK_USE_DOUBLE_LE)');
    genc.emitLine('struct duk_rom_tval_object { const void *ptr; duk_uint32_t hiword; };');
    genc.emitLine('struct duk_rom_tval_string { const void *ptr; duk_uint32_t hiword; };');
    genc.emitLine('struct duk_rom_tval_undefined { const void *ptr; duk_uint32_t hiword; };');
    genc.emitLine('struct duk_rom_tval_null { const void *ptr; duk_uint32_t hiword; };');
    genc.emitLine('struct duk_rom_tval_lightfunc { duk_rom_funcptr ptr; duk_uint32_t hiword; };');
    genc.emitLine('struct duk_rom_tval_boolean { duk_uint32_t dummy; duk_uint32_t hiword; };');
    genc.emitLine('#elif defined(DUK_USE_DOUBLE_BE)');
    genc.emitLine('struct duk_rom_tval_object { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_string { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_undefined { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_null { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_lightfunc { duk_uint32_t hiword; duk_rom_funcptr ptr; };');
    genc.emitLine('struct duk_rom_tval_boolean { duk_uint32_t hiword; duk_uint32_t dummy; };');
    genc.emitLine('#elif defined(DUK_USE_DOUBLE_ME)');
    genc.emitLine('struct duk_rom_tval_object { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_string { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_undefined { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_null { duk_uint32_t hiword; const void *ptr; };');
    genc.emitLine('struct duk_rom_tval_lightfunc { duk_uint32_t hiword; duk_rom_funcptr ptr; };');
    genc.emitLine('struct duk_rom_tval_boolean { duk_uint32_t hiword; duk_uint32_t dummy; };');
    genc.emitLine('#else');
    genc.emitLine('#error invalid endianness defines');
    genc.emitLine('#endif');
}

function emitTvalStructsUnpackedTval(genc) {
    genc.emitLine('#if defined(DUK_USE_UNION_INITIALIZERS)');

    // For C99 and C++20, preferred because involves no memory layout hacks.
    // Many (most?) C++ compilers support designated initializer syntax
    // before C++20.
    genc.emitLine('typedef duk_tval duk_rom_tval_undefined;');
    genc.emitLine('typedef duk_tval duk_rom_tval_null;');
    genc.emitLine('typedef duk_tval duk_rom_tval_boolean;');
    genc.emitLine('typedef duk_tval duk_rom_tval_number;');
    genc.emitLine('typedef duk_tval duk_rom_tval_object;');
    genc.emitLine('typedef duk_tval duk_rom_tval_string;');
    genc.emitLine('typedef duk_tval duk_rom_tval_lightfunc;');
    genc.emitLine('typedef duk_propvalue duk_rom_tval_accessor;');

    genc.emitLine('#else  /* DUK_USE_UNION_INITIALIZERS */');

    // For pre-C99 and pre-C++20.  Here we assume either 32-bit or 64-bit
    // types, and must make some guesses about memory layout.
    //
    // Unpacked initializers are written assuming normal struct alignment
    // rules so that sizeof(duk_tval) == 16.  32-bit pointers need special
    // handling to ensure the individual initializers pad to 16 bytes as
    // necessary.
    genc.emitLine('typedef struct duk_rom_tval_undefined duk_rom_tval_undefined;');
    genc.emitLine('typedef struct duk_rom_tval_null duk_rom_tval_null;');
    genc.emitLine('typedef struct duk_rom_tval_boolean duk_rom_tval_boolean;');
    genc.emitLine('typedef struct duk_rom_tval_number duk_rom_tval_number;');
    genc.emitLine('typedef struct duk_rom_tval_object duk_rom_tval_object;');
    genc.emitLine('typedef struct duk_rom_tval_string duk_rom_tval_string;');
    genc.emitLine('typedef struct duk_rom_tval_lightfunc duk_rom_tval_lightfunc;');
    genc.emitLine('typedef struct duk_rom_tval_accessor duk_rom_tval_accessor;');
    genc.emitLine('struct duk_rom_tval_undefined { duk_small_uint_t tag; duk_small_uint_t extra; duk_uint8_t bytes[8]; };');
    genc.emitLine('struct duk_rom_tval_null { duk_small_uint_t tag; duk_small_uint_t extra; duk_uint8_t bytes[8]; };');
    genc.emitLine('struct duk_rom_tval_boolean { duk_small_uint_t tag; duk_small_uint_t extra; duk_uint32_t val; duk_uint32_t unused; };');
    genc.emitLine('struct duk_rom_tval_number { duk_small_uint_t tag; duk_small_uint_t extra; duk_uint8_t bytes[8]; };');
    genc.emitLine('#if defined(DUK__32BITPTR)');
    genc.emitLine('struct duk_rom_tval_object { duk_small_uint_t tag; duk_small_uint_t extra; const duk_heaphdr *val; const void *dummy; };');
    genc.emitLine('struct duk_rom_tval_string { duk_small_uint_t tag; duk_small_uint_t extra; const duk_heaphdr *val; const void *dummy; };');
    genc.emitLine('struct duk_rom_tval_lightfunc { duk_small_uint_t tag; duk_small_uint_t extra; duk_rom_funcptr ptr; const void *dummy; };');
    genc.emitLine('struct duk_rom_tval_accessor { const duk_hobject *get; const duk_hobject *set; const void *dummy1; const void *dummy2; };');
    genc.emitLine('#else  /* DUK__32BITPTR */');
    genc.emitLine('struct duk_rom_tval_object { duk_small_uint_t tag; duk_small_uint_t extra; const duk_heaphdr *val; };');
    genc.emitLine('struct duk_rom_tval_string { duk_small_uint_t tag; duk_small_uint_t extra; const duk_heaphdr *val; };');
    genc.emitLine('struct duk_rom_tval_lightfunc { duk_small_uint_t tag; duk_small_uint_t extra; duk_rom_funcptr ptr; };');
    genc.emitLine('struct duk_rom_tval_accessor { const duk_hobject *get; const duk_hobject *set; };');
    genc.emitLine('#endif  /* DUK__32BITPTR */');

    genc.emitLine('#endif  /* DUK_USE_UNION_INITIALIZERS */');
}

function emitTvalStructs(genc) {
    // Emit duk_tval structs.  This is messy with packed/unpacked duk_tval,
    // endianness variants, pointer sizes, designed initializer support, etc.

    genc.emitLine('#if defined(DUK_USE_PACKED_TVAL)');
    emitTvalStructsPackedTval(genc);
    genc.emitLine('#else  /* DUK_USE_PACKED_TVAL */');
    emitTvalStructsUnpackedTval(genc);
    genc.emitLine('#endif  /* DUK_USE_PACKED_TVAL */');
}

function emitDoubleInitializer(genc) {
    // Double initializer byte shuffle macro to handle byte orders
    // without duplicating the entire initializers.

    genc.emitLine('#if defined(DUK_USE_DOUBLE_LE)');
    genc.emitLine('#define DUK__DBLBYTES(a,b,c,d,e,f,g,h) { (h), (g), (f), (e), (d), (c), (b), (a) }');
    genc.emitLine('#elif defined(DUK_USE_DOUBLE_BE)');
    genc.emitLine('#define DUK__DBLBYTES(a,b,c,d,e,f,g,h) { (a), (b), (c), (d), (e), (f), (g), (h) }');
    genc.emitLine('#elif defined(DUK_USE_DOUBLE_ME)');
    genc.emitLine('#define DUK__DBLBYTES(a,b,c,d,e,f,g,h) { (d), (c), (b), (a), (h), (g), (f), (e) }');
    genc.emitLine('#else');
    genc.emitLine('#error invalid endianness defines');
    genc.emitLine('#endif');
    genc.emitLine('');
}

function emitTvalInitializersPackedTval(genc) {
    genc.emitLine('#define DUK__TVAL_NUMBER(hostbytes) { hostbytes }');  // bytes already in host order
    genc.emitLine('#if defined(DUK_USE_DOUBLE_LE)');
    genc.emitLine('#define DUK__TVAL_UNDEFINED() { (const void *) NULL, (DUK_TAG_UNDEFINED << 16) }');
    genc.emitLine('#define DUK__TVAL_NULL() { (const void *) NULL, (DUK_TAG_NULL << 16) }');
    genc.emitLine('#define DUK__TVAL_LIGHTFUNC(func,flags) { (duk_rom_funcptr) (func), (DUK_TAG_LIGHTFUNC << 16) + (flags) }');
    genc.emitLine('#define DUK__TVAL_BOOLEAN(bval) { 0, (DUK_TAG_BOOLEAN << 16) + (bval) }');
    genc.emitLine('#define DUK__TVAL_OBJECT(ptr) { (const void *) (ptr), (DUK_TAG_OBJECT << 16) }');
    genc.emitLine('#define DUK__TVAL_STRING(ptr) { (const void *) (ptr), (DUK_TAG_STRING << 16) }');
    genc.emitLine('#elif defined(DUK_USE_DOUBLE_BE)');
    genc.emitLine('#define DUK__TVAL_UNDEFINED() { (DUK_TAG_UNDEFINED << 16), (const void *) NULL }');
    genc.emitLine('#define DUK__TVAL_NULL() { (DUK_TAG_NULL << 16), (const void *) NULL }');
    genc.emitLine('#define DUK__TVAL_LIGHTFUNC(func,flags) { (DUK_TAG_LIGHTFUNC << 16) + (flags), (duk_rom_funcptr) (func) }');
    genc.emitLine('#define DUK__TVAL_BOOLEAN(bval) { (DUK_TAG_BOOLEAN << 16) + (bval), 0 }');
    genc.emitLine('#define DUK__TVAL_OBJECT(ptr) { (DUK_TAG_OBJECT << 16), (const void *) (ptr) }');
    genc.emitLine('#define DUK__TVAL_STRING(ptr) { (DUK_TAG_STRING << 16), (const void *) (ptr) }');
    genc.emitLine('#elif defined(DUK_USE_DOUBLE_ME)');
    genc.emitLine('#define DUK__TVAL_UNDEFINED() { (DUK_TAG_UNDEFINED << 16), (const void *) NULL }');
    genc.emitLine('#define DUK__TVAL_NULL() { (DUK_TAG_NULL << 16), (const void *) NULL }');
    genc.emitLine('#define DUK__TVAL_LIGHTFUNC(func,flags) { (DUK_TAG_LIGHTFUNC << 16) + (flags), (duk_rom_funcptr) (func) }');
    genc.emitLine('#define DUK__TVAL_BOOLEAN(bval) { (DUK_TAG_BOOLEAN << 16) + (bval), 0 }');
    genc.emitLine('#define DUK__TVAL_OBJECT(ptr) { (DUK_TAG_OBJECT << 16), (const void *) (ptr) }');
    genc.emitLine('#define DUK__TVAL_STRING(ptr) { (DUK_TAG_STRING << 16), (const void *) (ptr) }');
    genc.emitLine('#else');
    genc.emitLine('#error invalid endianness defines');
    genc.emitLine('#endif');
    genc.emitLine('#define DUK__TVAL_ACCESSOR(getter,setter) { (const duk_hobject *) (getter), (const duk_hobject *) (setter) }');
}

function emitTvalInitializersUnpackedTval(genc) {
    genc.emitLine('#if defined(DUK_USE_UNION_INITIALIZERS)');

    genc.emitLine('#define DUK__TVAL_NUMBER(hostbytes) { .t=DUK_TAG_NUMBER, .v_extra=0, .v={ .bytes=hostbytes } }');  // bytes already in host order
    genc.emitLine('#define DUK__TVAL_UNDEFINED() { .t=DUK_TAG_UNDEFINED, .v_extra=0, .v={ .bytes={0,0,0,0,0,0,0,0} } }');
    genc.emitLine('#define DUK__TVAL_NULL() { .t=DUK_TAG_NULL, .v_extra=0, .v={ .bytes={0,0,0,0,0,0,0,0} } }');
    genc.emitLine('#define DUK__TVAL_BOOLEAN(bval) { .t=DUK_TAG_BOOLEAN, .v_extra=0, .v={ .i=(bval) } }');
    genc.emitLine('#define DUK__TVAL_OBJECT(ptr) { .t=DUK_TAG_OBJECT, .v_extra=0, .v={ .hobject=(duk_hobject *) DUK_LOSE_CONST(ptr) } }');
    genc.emitLine('#define DUK__TVAL_STRING(ptr) { .t=DUK_TAG_STRING, .v_extra=0, .v={ .hstring=(duk_hstring *) DUK_LOSE_CONST(ptr) } }');
    genc.emitLine('#define DUK__TVAL_LIGHTFUNC(func,flags) { .t=DUK_TAG_LIGHTFUNC, .v_extra=(flags), .v={ .lightfunc=(duk_rom_funcptr) (func) } }');
    genc.emitLine('#define DUK__TVAL_ACCESSOR(getter,setter) { .a={ .get=(duk_hobject *) DUK_LOSE_CONST(getter), .set=(duk_hobject *) DUK_LOSE_CONST(setter) } }');

    genc.emitLine('#else  /* DUK_USE_UNION_INITIALIZERS */');

    genc.emitLine('#define DUK__TVAL_NUMBER(hostbytes) { DUK_TAG_NUMBER, 0, hostbytes }');  // bytes already in host order
    genc.emitLine('#define DUK__TVAL_UNDEFINED() { DUK_TAG_UNDEFINED, 0, {0,0,0,0,0,0,0,0} }');
    genc.emitLine('#define DUK__TVAL_NULL() { DUK_TAG_NULL, 0, {0,0,0,0,0,0,0,0} }');
    genc.emitLine('#define DUK__TVAL_BOOLEAN(bval) { DUK_TAG_BOOLEAN, 0, (bval), 0 }');
    genc.emitLine('#if defined(DUK__32BITPTR)');
    genc.emitLine('#define DUK__TVAL_OBJECT(ptr) { DUK_TAG_OBJECT, 0, (const duk_heaphdr *) (ptr), (const void *) NULL }');
    genc.emitLine('#define DUK__TVAL_STRING(ptr) { DUK_TAG_STRING, 0, (const duk_heaphdr *) (ptr), (const void *) NULL }');
    genc.emitLine('#define DUK__TVAL_LIGHTFUNC(func,flags) { DUK_TAG_LIGHTFUNC, (flags), (duk_rom_funcptr) (func), (const void *) NULL }');
    genc.emitLine('#define DUK__TVAL_ACCESSOR(getter,setter) { (const duk_hobject *) (getter), (const duk_hobject *) (setter), (const void *) NULL, (const void *) NULL }');
    genc.emitLine('#else  /* DUK__32BITPTR */');
    genc.emitLine('#define DUK__TVAL_OBJECT(ptr) { DUK_TAG_OBJECT, 0, (const duk_heaphdr *) (ptr) }');
    genc.emitLine('#define DUK__TVAL_STRING(ptr) { DUK_TAG_STRING, 0, (const duk_heaphdr *) (ptr) }');
    genc.emitLine('#define DUK__TVAL_LIGHTFUNC(func,flags) { DUK_TAG_LIGHTFUNC, (flags), (duk_rom_funcptr) (func) }');
    genc.emitLine('#define DUK__TVAL_ACCESSOR(getter,setter) { (const duk_hobject *) (getter), (const duk_hobject *) (setter) }');
    genc.emitLine('#endif  /* DUK__32BITPTR */');

    genc.emitLine('#endif  /* DUK_USE_UNION_INITIALIZERS */');
}

// Emit duk_tval initializer literal macros.
function emitTvalInitializers(genc) {
    genc.emitLine('#if defined(DUK_USE_PACKED_TVAL)');
    emitTvalInitializersPackedTval(genc);
    genc.emitLine('#else  /* DUK_USE_PACKED_TVAL */');
    emitTvalInitializersUnpackedTval(genc);
    genc.emitLine('#endif  /* DUK_USE_PACKED_TVAL */');
}

// Emit ROM objects initialized types and macros.
function emitInitializerTypesAndMacrosSource(genc) {
    emitDoubleInitializer(genc);
    emitObjectTypedefs(genc);
    emitObjectInitializers(genc);
    emitFunctionPointerTypedef(genc);
    emitTvalStructs(genc);
    emitTvalInitializers(genc);
}
exports.emitInitializerTypesAndMacrosSource = emitInitializerTypesAndMacrosSource;

// Portable and exact float initializer.
function getDoubleBytesInitializer(val) {
    assert(typeof val === 'string' && val.length == 16);
    var tmp = hexDecode(val).map((t) => '' + t + 'U');
    return 'DUK__DBLBYTES(' + tmp.join(',') + ')';
}

function getTvalNumberInitializer(val) {
    return 'DUK__TVAL_NUMBER(' + getDoubleBytesInitializer(val) + ')';
}

// Get an initializer type and initializer literal for a specified value
// (expressed in YAML metadata format).  The types and initializers depend
// on declarations emitted before the initializers, and in most cases use
// a macro to hide the selection between several initializer variants.
function getValueInitializer(meta, p, biStrMap, biObjMap) {
    assert(typeof p === 'object' && p !== null, 'value must be an object');
    var v = p.value;
    assert(typeof v !== 'undefined', 'p.value must not be undefined');

    var initType, initLit;

    if (v === null) {
        initType = 'duk_rom_tval_null';
        initLit = 'DUK__TVAL_NULL()';
    } else if (typeof v === 'boolean') {
        initType = 'duk_rom_tval_boolean';
        initLit = 'DUK__TVAL_BOOLEAN(' + (v ? '1' : '0') + ')';
    } else if (typeof v === 'number') {
        let fval = numberToDoubleHex(v);
        initType = 'duk_rom_tval_number';
        initLit = getTvalNumberInitializer(fval);
    } else if (typeof v === 'string') {
        let str = biStrMap[v];
        assert(typeof str === 'string', 'str must be a string');
        initType = 'duk_rom_tval_string';
        initLit = 'DUK__TVAL_STRING(&' + str + ')';
    } else if (typeof v === 'object' && v !== null) {
        switch (v.type) {
        case 'double': {
            initType = 'duk_rom_tval_number'
            initLit = getTvalNumberInitializer(v.bytes);
            break;
        }
        case 'undefined': {
            initType = 'duk_rom_tval_undefined';
            initLit = 'DUK__TVAL_UNDEFINED()';
            break;
        }
        case 'null': {
            initType = 'duk_rom_tval_null';
            initLit = 'DUK__TVAL_NULL()';
            break;
        }
        case 'object': {
            let obj = biObjMap[v.id];
            assert(typeof obj === 'string', 'obj must be a string');
            initType = 'duk_rom_tval_object';
            initLit = 'DUK__TVAL_OBJECT(&' + obj + ')';
            break;
        }
        case 'accessor': {
            let getterRef = 'NULL';
            let setterRef = 'NULL';

            if (typeof v.getter_id !== 'undefined') {
                assert(typeof v.getter_id === 'string', 'getter_id must be string');
                let getterObject = findObjectById(meta, v.getter_id);
                assert(typeof getterObject === 'object' && getterObject !== null, 'getterObject must exist');
                let obj = biObjMap[getterObject.id];
                assert(typeof obj === 'string', 'getter obj must be string');
                getterRef = '&' + obj;
            }

            if (typeof v.setter_id !== 'undefined') {
                assert(typeof v.setter_id === 'string', 'setter_id must be string');
                let setterObject = findObjectById(meta, v.setter_id);
                assert(typeof setterObject === 'object' && setterObject !== null, 'setterObject must exist');
                let obj = biObjMap[setterObject.id];
                assert(typeof obj === 'string', 'setter obj must be string');
                setterRef = '&' + obj;
            }

            initType = 'duk_rom_tval_accessor';
            initLit = 'DUK__TVAL_ACCESSOR(' + getterRef + ',' + setterRef + ')';
            break;
        }
        case 'lightfunc': {
            // Match DUK_LFUNC_FLAGS_PACK() in duk_tval.h.
            let lfLength, lfNargs, lfMagic;

            if (typeof v.length !== 'undefined') {
                assert(typeof v.length === 'number' && v.length >= 0 && v.length <= 15);
                lfLength = Math.floor(v.length);
            } else {
                lfLength = 0;
            }

            if (propDefault(v, 'varargs', true)) {
                lfNargs = 15;  // varargs marker
            } else if (typeof v.nargs !== 'undefined') {
                assert(typeof v.nargs === 'number' && v.nargs >= 0 && v.nargs <= 14);
                lfNargs = Math.floor(v.nargs);
            } else {
                throw new TypeError('missing .nargs for lightfunc value');
            }

            if (typeof v.magic !== 'undefined') {
                assert(typeof v.magic === 'number' && v.magic >= -0x80 && v.magic <= 0x7f);
                lfMagic = Math.floor(v.magic) & 0xff;  // convert to 0x00-0xff
            } else {
                lfMagic = 0;
            }
            assert(lfMagic >= 0x00 && lfMagic <= 0xff);
            assert(lfLength >= 0x00 && lfLength <= 0x0f);
            assert(lfNargs >= 0x00 && lfNargs <= 0x0f);
            let lfFlags = (lfMagic << 8) + (lfLength << 4) + lfNargs;

            assert(typeof v.native === 'string');
            initType = 'duk_rom_tval_lightfunc';
            initLit = 'DUK__TVAL_LIGHTFUNC(' + v.native + ',' + lfFlags + 'L)';
            break;
        }
        default: {
            throw new TypeError('invalid value type: ' + v.type);
        }
        }
    } else {
        throw new TypeError('invalid value type');
    }

    return { initType, initLit };
}

// Helpers to get either initializer type or value only (not both).
function getValueInitializerType(meta, val, biStrMap, biObjMap) {
    let { initType } = getValueInitializer(meta, val, biStrMap, biObjMap);
    return initType;
}
exports.getValueInitializerType = getValueInitializerType;

function getValueInitializerLiteral(meta, val, biStrMap, biObjMap) {
    let { initLit } = getValueInitializer(meta, val, biStrMap, biObjMap);
    return initLit;
}
exports.getValueInitializerLiteral = getValueInitializerLiteral;
