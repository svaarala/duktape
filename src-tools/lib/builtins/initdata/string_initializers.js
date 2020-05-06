/*
 *  Type declarations and initializers for ROM strings (duk_hstring).
 */

'use strict';

const { bstrToArray } = require('../../util/bstr');
const { stringIsArridx, stringIsHiddenSymbol, stringIsAnySymbol } = require('../../util/string_util');
const { unvalidatedUtf8Length } = require('../../util/utf8');
const { hashStringDense, hashStringSparse } = require('../../strhash/string_hash');
const { numberCompare } = require('../../util/sort');
const { createBareObject } = require('../../util/bare');

// Fixed seed for ROM strings, must match src-input/duk_heap_alloc.c.
const DUK__FIXED_HASH_SEED = 0xabcd1234;

// Get string character .length; must match runtime .length computation.
function getRomCharlen(v) {
    return unvalidatedUtf8Length(v);
}

// Get string hash initializers; need to compute possible string hash variants
// which will match runtime values.
function getStrHash16Macro(val) {
    var hash16le = hashStringDense(val, DUK__FIXED_HASH_SEED, false /*big_endian*/, true /*strhash16*/);
    var hash16be = hashStringDense(val, DUK__FIXED_HASH_SEED, true /*big_endian*/, true /*strhash16*/);
    var hash16sparse = hashStringSparse(val, DUK__FIXED_HASH_SEED, true /*strhash16*/);
    return 'DUK__STRHASH16(' + hash16le + 'U,' + hash16be + 'U,' + hash16sparse + 'U)';
}

function getStrHash32Macro(val) {
    var hash16le = hashStringDense(val, DUK__FIXED_HASH_SEED, false /*big_endian*/, false /*strhash16*/);
    var hash16be = hashStringDense(val, DUK__FIXED_HASH_SEED, true /*big_endian*/, false /*strhash16*/);
    var hash16sparse = hashStringSparse(val, DUK__FIXED_HASH_SEED, false /*strhash16*/);
    return 'DUK__STRHASH32(' + hash16le + 'U,' + hash16be + 'U,' + hash16sparse + 'U)';
}

function emitStringHashMacros(genc) {
    // String hash values depend on endianness and other factors,
    // use an initializer macro to select the appropriate hash.
    genc.emitLine('/* When unaligned access possible, 32-bit values are fetched using host order.');
    genc.emitLine(' * When unaligned access not possible, always simulate little endian order.');
    genc.emitLine(' * See: src-input/duk_util_hashbytes.c:duk_util_hashbytes().');
    genc.emitLine(' */');
    genc.emitLine('#if defined(DUK_USE_STRHASH_DENSE)');
    genc.emitLine('#if defined(DUK_USE_HASHBYTES_UNALIGNED_U32_ACCESS)');
    genc.emitLine('#if defined(DUK_USE_INTEGER_BE)');
    genc.emitLine('#define DUK__STRHASH16(hash16le,hash16be,hash16sparse) (hash16be)');
    genc.emitLine('#define DUK__STRHASH32(hash32le,hash32be,hash32sparse) (hash32be)');
    genc.emitLine('#else');
    genc.emitLine('#define DUK__STRHASH16(hash16le,hash16be,hash16sparse) (hash16le)');
    genc.emitLine('#define DUK__STRHASH32(hash32le,hash32be,hash32sparse) (hash32le)');
    genc.emitLine('#endif');
    genc.emitLine('#else');
    genc.emitLine('#define DUK__STRHASH16(hash16le,hash16be,hash16sparse) (hash16le)');
    genc.emitLine('#define DUK__STRHASH32(hash32le,hash32be,hash32sparse) (hash32le)');
    genc.emitLine('#endif');
    genc.emitLine('#else  /* DUK_USE_STRHASH_DENSE */');
    genc.emitLine('#define DUK__STRHASH16(hash16le,hash16be,hash16sparse) (hash16sparse)');
    genc.emitLine('#define DUK__STRHASH32(hash32le,hash32be,hash32sparse) (hash32sparse)');
    genc.emitLine('#endif  /* DUK_USE_STRHASH_DENSE */');
}
exports.emitStringHashMacros = emitStringHashMacros;

function emitStringInitMacro(genc) {
    // String header initializer macro, takes into account lowmem etc.
    genc.emitLine('#if defined(DUK_USE_HEAPPTR16)');
    genc.emitLine('#if !defined(DUK_USE_REFCOUNT16)');
    genc.emitLine('#error currently assumes DUK_USE_HEAPPTR16 and DUK_USE_REFCOUNT16 are both defined');
    genc.emitLine('#endif');
    genc.emitLine('#if defined(DUK_USE_HSTRING_CLEN)');
    genc.emitLine('#define DUK__STRINIT(heaphdr_flags,refcount,hash32,hash16,blen,clen,next) ' +
                  ' { { (heaphdr_flags) | ((hash16) << 16), DUK__REFCINIT((refcount)), (blen), (duk_hstring *) DUK_LOSE_CONST((next)) }, (clen) }');
    genc.emitLine('#else  /* DUK_USE_HSTRING_CLEN */')
    genc.emitLine('#define DUK__STRINIT(heaphdr_flags,refcount,hash32,hash16,blen,clen,next) ' +
                  ' { { (heaphdr_flags) | ((hash16) << 16), DUK__REFCINIT((refcount)), (blen), (duk_hstring *) DUK_LOSE_CONST((next)) } }');
    genc.emitLine('#endif  /* DUK_USE_HSTRING_CLEN */');
    genc.emitLine('#else  /* DUK_USE_HEAPPTR16 */');
    genc.emitLine('#define DUK__STRINIT(heaphdr_flags,refcount,hash32,hash16,blen,clen,next) ' +
                  ' { { (heaphdr_flags), DUK__REFCINIT((refcount)), (duk_hstring *) DUK_LOSE_CONST((next)) }, (hash32), (blen), (clen) }');
    genc.emitLine('#endif  /* DUK_USE_HEAPPTR16 */');
}
exports.emitStringInitMacro = emitStringInitMacro;

function emitStringDeclarations(genc, strs) {
    // Sort used lengths and declare per-length initializers.
    var lenMap = createBareObject({});
    for (let v of strs) {
        lenMap[v.length] = true;
    }
    var lens = Object.getOwnPropertyNames(lenMap).map((v) => Number(v)).sort(numberCompare);

    lens.forEach((len) => {
        genc.emitLine('typedef struct duk_romstr_' + len + ' duk_romstr_' + len + '; ' +
                      'struct duk_romstr_' + len + ' { duk_hstring hdr; duk_uint8_t data[' + (len + 1) + ']; };');
    });
}
exports.emitStringDeclarations = emitStringDeclarations;

function emitStringInitializer(genc, v, biStrMap, reservedWords, strictReservedWords, romstrNext) {
    let tmp = 'DUK_INTERNAL const duk_romstr_' + v.length + ' ' + biStrMap[v] + ' = {';

    let flags = [ 'DUK_HTYPE_STRING',
                  'DUK_HEAPHDR_FLAG_READONLY',
                  'DUK_HEAPHDR_FLAG_REACHABLE',
                  'DUK_HSTRING_FLAG_PINNED_LITERAL' ];

    let isArridx = stringIsArridx(v);
    let blen = v.length;
    let clen = getRomCharlen(v);

    if (blen === clen) {
        flags.push('DUK_HSTRING_FLAG_ASCII');
    }
    if (isArridx) {
        flags.push('DUK_HSTRING_FLAG_ARRIDX');
    }
    if (stringIsAnySymbol(v)) {
        flags.push('DUK_HSTRING_FLAG_SYMBOL');
    }
    if (stringIsHiddenSymbol(v)) {
        flags.push('DUK_HSTRING_FLAG_HIDDEN');
    }
    if (v === 'eval' || v === 'arguments') {
        flags.push('DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS');
    }
    if (typeof reservedWords[v] !== 'undefined') {
        flags.push('DUK_HSTRING_FLAG_RESERVED_WORD');
    }
    if (typeof strictReservedWords[v] !== 'undefined') {
        flags.push('DUK_HSTRING_FLAG_STRICT_RESERVED_WORD');
    }

    let h_next = 'NULL';
    if (typeof romstrNext[v] !== 'undefined') {
        h_next = '&' + biStrMap[romstrNext[v]];
    }

    let refcount = 1;
    tmp += 'DUK__STRINIT(' + flags.join('|') + ',' + refcount + ',' +
           getStrHash32Macro(v) + ',' + getStrHash16Macro(v) + ',' +
           blen + ',' + clen + ',' + h_next + ')';

    let arr = bstrToArray(v);
    arr.push(0);
    tmp += ',{' + arr.map((v) => '' + v + 'U').join(',') + '}';

    tmp += '};';

    genc.emitLine(tmp);
}
exports.emitStringInitializer = emitStringInitializer;
