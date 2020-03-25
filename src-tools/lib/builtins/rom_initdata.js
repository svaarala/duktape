/*
 *  Built-in object and string declarations and definitions,
 *  intended to be compiled into the code section ("ROM")
 */

'use strict';

const { propDefault, walkObjectsAndProperties } = require('./metadata/util');
const { createBareObject } = require('../util/bare');
const { assert } = require('../util/assert');
const { jsonStringifyAscii } = require('../util/json');
const { classToNumber } = require('./classnames');
const { emitStringHashMacros, emitStringInitMacro, emitStringDeclarations, emitStringInitializer } = require('./initdata/string_initializers');
const { createRomStringTable } = require('./initdata/stringtable');
const { emitPropertyTableStructs, emitPropertyTableForwardDeclarations, emitPropertyTableDefinitions } = require('./initdata/property_table_initializers');
const { emitPropAttrDefines } = require('./initdata/property_attributes');

// Base value for compressed ROM pointers, used range is [ROMPTR_FIRST,0xffff].
// Must match DUK_USE_ROM_PTRCOMP_FIRST (generated header checks).
const ROMPTR_FIRST = 0xf800;  // 2048 pointers; now around ~1000 used
exports.ROMPTR_FIRST = ROMPTR_FIRST;

// ROM string table size.
const ROMSTR_LOOKUP_SIZE = 256;
exports.ROMSTR_LOOKUP_SIZE = ROMSTR_LOOKUP_SIZE;

function getStringRomHash(v) {
    assert(ROMSTR_LOOKUP_SIZE === 256);  // for now
    if (v.length > 0) {
        return (v.charCodeAt(0) + (v.length << 4)) & 0xff;
    } else {
        return (0 + (v.length << 4)) & 0xff;
    }
}

function emitNativeFunctionDeclarationsHeader(genc, meta) {
    var emitted = createBareObject({});  // suppress duplicates
    var funcList = [];

    function emitFunc(fname) {
        if (typeof fname === 'string' && !emitted[fname]) {
            emitted[fname] = true;
            funcList.push(fname);
        }
    }

    walkObjectsAndProperties(meta, (o) => {
        emitFunc(o.native);
    }, (p, o) => {
        void o;
        let v = p.value;
        if (typeof v === 'object' && v !== null && v.type === 'lightfunc') {
            assert(typeof v.native === 'string');
            emitFunc(v.native);
        }
    });

    for (let fname of funcList) {
        // Visibility depends on whether the function is Duktape internal or user.
        // Use a simple prefix for now.
        genc.emitLine((fname.startsWith('duk_') ? 'DUK_INTERNAL_DECL' : 'extern') +
                      ' duk_ret_t ' + fname + '(duk_context *ctx);');
    }
}
exports.emitNativeFunctionDeclarationsHeader = emitNativeFunctionDeclarationsHeader;

// Emit ROM strings source: structs/typedefs and their initializers.
// Separate initialization structs are needed for strings of different
// length.
function emitStringsSource(genc, meta) {
    // Write built-in strings as code section initializers.

    var strs = meta._strings_plain;  // all strings, plain versions
    var reservedWords = meta._is_plain_reserved_word;
    var strictReservedWords = meta._is_plain_strict_reserved_word;
    var strsNeedingStridx = meta.strings_stridx;

    emitStringDeclarations(genc, strs);
    genc.emitLine('');
    emitStringHashMacros(genc);
    emitStringInitMacro(genc);

    // Create ROM string table, using plain strings here.
    assert(ROMSTR_LOOKUP_SIZE === 256);  // currently hardcoded
    var { romstrHash, romstrNext, romstrChainLens } = createRomStringTable(strs, ROMSTR_LOOKUP_SIZE, getStringRomHash);
    console.debug(romstrChainLens);

    var biStrMap = createBareObject({});  // string -> initializer variable name
    strs.forEach((v, idx) => {
        biStrMap[v] = 'duk_str_' + idx;
    });

    // Emit string initializers.  Emit the strings in an order which avoids
    // forward declarations for the h_next link pointers; const forward
    // declarations are a problem in C++.

    genc.emitLine('');
    for (let lst of romstrHash) {
        for (let idx = lst.length - 1; idx >= 0; idx--) {
            let v = lst[idx];
            emitStringInitializer(genc, v, biStrMap, reservedWords, strictReservedWords, romstrNext);
        }
    }

    // Emit the ROM string lookup table used by string interning.
    //
    // cdecl> explain const int * const foo;
    // declare foo as const pointer to const int
    genc.emitLine('');
    genc.emitLine('DUK_INTERNAL const duk_hstring * const duk_rom_strings_lookup[' + romstrHash.length + '] = {');
    for (let lst of romstrHash) {
        if (lst.length === 0) {
            genc.emitLine('\tNULL,');
        } else {
            genc.emitLine('\t(const duk_hstring *) &' + biStrMap[lst[0]] + ',');
        }
    }
    genc.emitLine('};');

    // Emit an array of duk_hstring pointers indexed using DUK_STRIDX_xxx.
    // This will back e.g. DUK_HTHREAD_STRING_XYZ(thr) directly, without
    // needing an explicit array in thr/heap->strs[].
    //
    // cdecl > explain const int * const foo;
    // declare foo as const pointer to const int
    genc.emitLine('');
    genc.emitLine('DUK_INTERNAL const duk_hstring * const duk_rom_strings_stridx[' + strsNeedingStridx.length + '] = {');
    for (let v of strsNeedingStridx) {
        genc.emitLine('\t(const duk_hstring *) &' + biStrMap[v.str] + ',');  // strs_needing_stridx is a list of objects, not plain strings
    }
    genc.emitLine('};');

    return { biStrMap };
}
exports.emitStringsSource = emitStringsSource;

// Emit ROM strings header.
function emitStringsHeader(genc, meta) {
    genc.emitLine('#if !defined(DUK_SINGLE_FILE)');  // C++ static const workaround
    genc.emitLine('DUK_INTERNAL_DECL const duk_hstring * const duk_rom_strings_lookup[' + ROMSTR_LOOKUP_SIZE + '];');
    genc.emitLine('DUK_INTERNAL_DECL const duk_hstring * const duk_rom_strings_stridx[' + meta.strings_stridx.length + '];');
    genc.emitLine('#endif');
}
exports.emitStringsHeader = emitStringsHeader;

function getObjectStructName(o) {
    if (propDefault(o, 'callable', false)) {
        return 'duk_romfun';
    } else if (o.class === 'Array') {
        return 'duk_romarr';
    } else if (o.class === 'ObjEnv') {
        return 'duk_romobjenv';
    } else {
        return 'duk_romobj';
    }
}

function emitObjectForwardDeclarations(genc, meta, objs) {
    // Forward declare all objects so that objects can reference them,
    // e.g. internal prototype reference.

    objs.forEach((o, idx) => {
        // Careful with C++: must avoid redefining a non-extern const.
        // See commentary above for duk_prop_%d forward declarations.
        let structName = getObjectStructName(o);
        genc.emitLine('DUK_EXTERNAL_DECL const ' + structName + ' duk_obj_' + idx + ';');
    });
}

function emitObjectDefinitions(genc, meta, objs, biObjMap, compressRomPtr) {
    // Define objects, reference property tables.  Objects will be
    // logically non-extensible so also leave their extensible flag
    // cleared despite what metadata requests; the runtime code expects
    // ROM objects to be non-extensible.

    objs.forEach((o, idx) => {
        var numProps = o.properties.length;
        var isFunc = propDefault(o, 'callable', false);
        var structName = getObjectStructName(o);
        var parts = [];
        var flags = [];

        parts.push('DUK_EXTERNAL const ' + structName + ' duk_obj_' + idx + ' = ');

        flags.push('DUK_HTYPE_OBJECT');
        flags.push('DUK_HEAPHDR_FLAG_READONLY');
        flags.push('DUK_HEAPHDR_FLAG_REACHABLE');
        if (isFunc) {
            flags.push('DUK_HOBJECT_FLAG_NATFUNC');
            flags.push('DUK_HOBJECT_FLAG_STRICT');
            flags.push('DUK_HOBJECT_FLAG_NEWENV');
        }
        if (propDefault(o, 'callable', false)) {
            flags.push('DUK_HOBJECT_FLAG_CALLABLE');
         }
        if (propDefault(o, 'constructable', false)) {
            flags.push('DUK_HOBJECT_FLAG_CONSTRUCTABLE');
        }
        if (o.class === 'Array') {
            flags.push('DUK_HOBJECT_FLAG_EXOTIC_ARRAY');
        }
        if (propDefault(o, 'special_call', false)) {
            flags.push('DUK_HOBJECT_FLAG_SPECIAL_CALL');
        }
        flags.push('DUK_HOBJECT_CLASS_AS_FLAGS(' + classToNumber(o.class) + ')');

        var refcount = 1;  // refcount is faked to be always 1

        var props = (numProps > 0 ? '&duk_prop_' + idx : 'NULL');
        var propsEnc16 = compressRomPtr(props);

        var iproto = (typeof o.internal_prototype !== 'undefined' ? '&' + biObjMap[o.internal_prototype] : 'NULL');
        var iprotoEnc16 = compressRomPtr(iproto);

        var eSize = numProps;
        var eNext = eSize;
        var aSize = 0;  // never an array part for now
        var hSize = 0;  // never a hash for now; not appropriate for perf relevant builds

        var nativeFunc;
        var nargs;
        var magic;

        if (isFunc) {
            nativeFunc = o.native;
            if (propDefault(o, 'varargs', false)) {
                nargs = 'DUK_VARARGS'
            } else if (typeof o.nargs === 'number') {
                nargs = String(o.nargs);
            } else {
                // 'nargs' should be defaulted from 'length' at metadata load.
                throw new TypeError('internal error, missing nargs for normalized object');
            }
            magic = propDefault(o, 'magic', 0);
            assert(typeof magic === 'number');
            magic = String(magic);
        } else {
            nativeFunc = 'dummy';
            nargs = '0';
            magic = '0';
        }

        assert(aSize === 0);
        assert(hSize === 0);

        var sharedFields = [
            flags.join('|'), refcount, props, propsEnc16,
            iproto, iprotoEnc16, eSize, eNext, aSize, hSize
        ];
        if (isFunc) {
            parts.push('DUK__ROMFUN_INIT(' + sharedFields.concat([
                nativeFunc, nargs, magic
            ]).join(',') + ');');
        } else if (o.class === 'Array') {
            var arrlen = 0;
            parts.push('DUK__ROMARR_INIT(' + sharedFields.concat([
                arrlen
            ]).join(',') + ');');
        } else if (o.class === 'ObjEnv') {
            var objenvTarget = '&' + biObjMap[o.objenv_target];
            var objenvHasThis = o.objenv_has_this;
            parts.push('DUK__ROMOBJENV_INIT(' + sharedFields.concat([
                objenvTarget, objenvHasThis
            ]).join(',') + ');');
        } else {
            parts.push('DUK__ROMOBJ_INIT(' + sharedFields.concat([
            ]).join(',') + ');');
        }

        genc.emitLine(parts.join(''));
    });
}

function emitRomBuiltinsList(genc, meta, objs, biObjMap) {
    // Emit a list of ROM builtins (those objects needing a bidx).
    //
    // cdecl > explain const int * const foo;
    // declare foo as const pointer to const int

    var countBidx = 0;
    for (let o of objs) {
        if (propDefault(o, 'bidx_used', false)) {
            countBidx++;
        }
    }

    genc.emitLine('DUK_INTERNAL const duk_hobject * const duk_rom_builtins_bidx[' + countBidx + '] = {');
    for (let o of objs) {
        if (propDefault(o, 'bidx_used', false)) {
            // For this we want the toplevel objects only.
            genc.emitLine('\t(const duk_hobject *) &' + biObjMap[o.id] + ',');
        }
    }
    genc.emitLine('};');
}

function emitCompressedPointersTable(genc, meta, romptrCompressList) {
    // For ROM pointer compression we'd need a -compile time- variant.
    // The current portable solution is to just assign running numbers
    // to ROM compressed pointers, and provide the table for user pointer
    // compression function.  Much better solutions would be possible,
    // but such solutions are compiler/platform specific.

    // Emit a table of compressed ROM pointers.  We must be able to
    // compress ROM pointers at compile time so we assign running
    // indices to them.  User pointer compression macros must use this
    // array to encode/decode ROM pointers.

    genc.emitLine('#if defined(DUK_USE_ROM_OBJECTS) && defined(DUK_USE_HEAPPTR16)');
    genc.emitLine('DUK_EXTERNAL const void * const duk_rom_compressed_pointers[' + (romptrCompressList.length + 1) + '] = {');
    romptrCompressList.forEach((ptr, idx) => {
        var ptrIndex = ROMPTR_FIRST + idx;
        genc.emitLine('\t(const void *) ' + ptr + ',  /* 0x' + ptrIndex.toString(16) + ' */');
    });
    var romptrLowest = ROMPTR_FIRST;
    var romptrHighest = ROMPTR_FIRST + romptrCompressList.length - 1;
    var spaceLeft = 0xffff - romptrHighest;
    genc.emitLine('\tNULL');  // for convenience
    genc.emitLine('};');
    genc.emitLine('#endif');

    console.debug('' + romptrCompressList.length + ' compressed rom pointers, used range is ' +
                  '[0x' + romptrLowest.toString(16) + ',0x' + romptrHighest.toString(16) + '], ' +
                  spaceLeft + ' space left');
}

// Emit ROM objects source: the object/function headers themselves, property
// table structs for different property table sizes/types, and property table
// initializers.
function emitObjectsSource(genc, meta, biStrMap) {
    var objs = meta.objects;

    // Need string and object maps (id -> C symbol name) early.

    var biObjMap = createBareObject({});  // object id -> initializer variable name
    objs.forEach((o, idx) => {
        biObjMap[o.id] = 'duk_obj_' + idx;
    });

    // Table for compressed ROM pointers; reserve high range of compressed pointer
    // values for this purpose.  This must contain all ROM pointers that might be
    // referenced (all objects, strings, and property tables at least).

    var romptrCompressList = [];
    function compressRomPtr(x) {
        var idx, res;
        assert(typeof x === 'string');
        if (x === 'NULL') {
            return 0;
        }
        idx = romptrCompressList.indexOf(x);
        if (idx < 0) {
            romptrCompressList.push(x);
            res = ROMPTR_FIRST + romptrCompressList.length - 1;
        } else {
            res = ROMPTR_FIRST + idx;
        }
        if (res > 0xffff) {
            throw new TypeError('too many compressed ROM pointers, you may need to tweak ROMPTR_FIRST');
        }
        return res;
    }

    // Add built-in strings and objects to compressed ROM pointers first.

    for (let k of Object.getOwnPropertyNames(biStrMap).sort()) {
        void compressRomPtr('&' + biStrMap[k]);
    }
    for (let k of Object.getOwnPropertyNames(biObjMap).sort()) {
        void compressRomPtr('&' + biObjMap[k]);
    }

    // Emit shorthand defines.
    emitPropAttrDefines(genc);

    // Emit property table structs and forward declarations.

    emitPropertyTableStructs(genc, meta, objs, biStrMap, biObjMap);
    genc.emitLine('');

    emitPropertyTableForwardDeclarations(genc, meta, objs, compressRomPtr);
    genc.emitLine('');

    // Emit object forward declarations and actual definitions.

    emitObjectForwardDeclarations(genc, meta, objs);
    genc.emitLine('');
    emitObjectDefinitions(genc, meta, objs, biObjMap, compressRomPtr);
    genc.emitLine('');

    // Emit property tables, which can reference all strings and objects
    // as they're defined before them.

    emitPropertyTableDefinitions(genc, meta, objs, biStrMap, biObjMap);
    genc.emitLine('');

    // Emit a list of ROM builtins (those objects needing a bidx).

    emitRomBuiltinsList(genc, meta, objs, biObjMap);
    genc.emitLine('');

    // Emit a table of compressed ROM pointers.
    emitCompressedPointersTable(genc, meta, romptrCompressList);

    return { romptrCompressList };
}
exports.emitObjectsSource = emitObjectsSource;

// Emit ROM objects header.
function emitObjectsHeader(genc, meta) {
    var bidx = 0;
    var objs = meta.objects;

    function objIdToDefname(id) {
        // bi_foo_bar => FOO_BAR
        return 'DUK_BIDX_' + id.toUpperCase().split('_').splice(1).join('_');
    }

    for (var o of objs) {
        if (propDefault(o, 'bidx_used', false)) {
            // For this we want the toplevel objects only.
            genc.emitDefine(objIdToDefname(o.id), bidx);
            bidx++;
        }
    }

    var countBidx = bidx;
    genc.emitDefine('DUK_NUM_BUILTINS', countBidx);
    genc.emitDefine('DUK_NUM_BIDX_BUILTINS', countBidx);
    genc.emitDefine('DUK_NUM_ALL_BUILTINS', objs.length);
    genc.emitLine('');
    genc.emitLine('#if !defined(DUK_SINGLE_FILE)');  // C++ static const workaround
    genc.emitLine('DUK_INTERNAL_DECL const duk_hobject * const duk_rom_builtins_bidx[' + countBidx + '];');
    genc.emitLine('#endif');
}
exports.emitObjectsHeader = emitObjectsHeader;

function emitStridxDefinesHeader(gcHdr, meta) {
    var strList = meta.strings_stridx;

    strList.forEach((s, idx) => {
        let defName;

        gcHdr.emitDefine(s.define, idx, jsonStringifyAscii(s.str));
        defName = s.define.replace('_STRIDX','_HEAP_STRING');
        gcHdr.emitDefine(defName + '(heap)', 'DUK_HEAP_GET_STRING((heap),' + s.define + ')');
        defName = s.define.replace('_STRIDX', '_HTHREAD_STRING');
        gcHdr.emitDefine(defName + '(thr)', 'DUK_HTHREAD_GET_STRING((thr),' + s.define + ')');
    });

    var idxStartReserved;
    var idxStartStrictReserved;
    strList.forEach((s, idx) => {
        if (idxStartReserved === void 0 && propDefault(s, 'reserved_word', false)) {
            idxStartReserved = idx;
        }
        if (idxStartStrictReserved === void 0 && propDefault(s, 'future_reserved_word_strict', false)) {
            idxStartStrictReserved = idx;
        }
    });
    assert(idxStartReserved !== void 0);
    assert(idxStartStrictReserved !== void 0);

    gcHdr.emitLine('');
    gcHdr.emitDefine('DUK_HEAP_NUM_STRINGS', strList.length);
    gcHdr.emitDefine('DUK_STRIDX_START_RESERVED', idxStartReserved);
    gcHdr.emitDefine('DUK_STRIDX_START_STRICT_RESERVED', idxStartStrictReserved);
    gcHdr.emitDefine('DUK_STRIDX_END_RESERVED', strList.length, 'exclusive endpoint');
    gcHdr.emitLine('');
    gcHdr.emitLine('/* To convert a heap stridx to a token number, subtract');
    gcHdr.emitLine(' * DUK_STRIDX_START_RESERVED and add DUK_TOK_START_RESERVED.');
    gcHdr.emitLine(' */');
}
exports.emitStridxDefinesHeader = emitStridxDefinesHeader;
