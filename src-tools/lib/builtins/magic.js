'use strict';

const { assert } = require('../util/assert');
const { createBareObject } = require('../util/bare');
const { isWholeFinite } = require('../util/double');

const mathOneargMagic = createBareObject({
    fabs: 0,    // BI_MATH_FABS_IDX
    acos: 1,    // BI_MATH_ACOS_IDX
    asin: 2,    // BI_MATH_ASIN_IDX
    atan: 3,    // BI_MATH_ATAN_IDX
    ceil: 4,    // BI_MATH_CEIL_IDX
    cos: 5,     // BI_MATH_COS_IDX
    exp: 6,     // BI_MATH_EXP_IDX
    floor: 7,   // BI_MATH_FLOOR_IDX
    log: 8,     // BI_MATH_LOG_IDX
    round: 9,   // BI_MATH_ROUND_IDX
    sin: 10,    // BI_MATH_SIN_IDX
    sqrt: 11,   // BI_MATH_SQRT_IDX
    tan: 12,    // BI_MATH_TAN_IDX
    cbrt: 13,   // BI_MATH_CBRT_IDX
    log2: 14,   // BI_MATH_LOG2_IDX
    log10: 15,  // BI_MATH_LOG10_IDX
    trunc: 16   // BI_MATH_TRUNC_IDX
});

const mathTwoargMagic = createBareObject({
    atan2: 0,  // BI_MATH_ATAN2_IDX
    pow: 1     // BI_MATH_POW_IDX
});

const arrayIterMagic = createBareObject({
    every: 0,    // BI_ARRAY_ITER_EVERY
    some: 1,     // BI_ARRAY_ITER_SOME
    forEach: 2,  // BI_ARRAY_ITER_FOREACH
    map: 3,      // BI_ARRAY_ITER_MAP
    filter: 4    // BI_ARRAY_ITER_FILTER
});

// Magic value for typedarray/node.js buffer read field operations.
function magicReadField(elem, signed, bigendian, typedarray) {
    assert(typeof signed === 'boolean');
    assert(typeof bigendian === 'boolean');
    assert(typeof typedarray === 'boolean');

    // Must match duk__FLD_xxx in duk_bi_buffer.c.
    var elemNum = createBareObject({
        '8bit': 0,
        '16bit': 1,
        '32bit': 2,
        'float': 3,
        'double': 4,
        'varint': 5
    })[elem];
    var signedNum = (signed ? 1 : 0);
    var bigendianNum = (bigendian ? 1 : 0);
    var typedarrayNum = (typedarray ? 1 : 0);
    return elemNum + (signedNum << 4) + (bigendianNum << 3) + (typedarrayNum << 5);
}
exports.magicReadField = magicReadField;

// Magic value for typedarray/node.js buffer write field operations.
function magicWriteField(elem, signed, bigendian, typedarray) {
    return magicReadField(elem, signed, bigendian, typedarray);
}
exports.magicWriteField = magicWriteField;

// Magic value for typedarray constructors.
function magicTypedArrayConstructor(elem, shift) {
    // Must match duk_hbufobj.h header.
    var elemNum = createBareObject({
        'uint8': 0,
        'uint8clamped': 1,
        'int8': 2,
        'uint16': 3,
        'int16': 4,
        'uint32': 5,
        'int32': 6,
        'float32': 7,
        'float64': 8
    })[elem];
    return (elemNum << 2) + shift;
}
exports.magicTypedArrayConstructor = magicTypedArrayConstructor;

function resolveMagic(elem, objIdToBidx) {
    var tmp;

    console.debug('resolve magic:', elem);

    if (elem === void 0 || elem === null) {
        return 0;
    }
    if (typeof elem === 'number') {
        elem = { type: 'plain', value: elem };
    }
    if (!(typeof elem === 'object' && elem !== null)) {
        throw new TypeError('invalid magic');
    }

    switch (elem.type) {
    case 'bidx': {
        // Maps to thr->builtins[].
        tmp = objIdToBidx[elem.id];
        if (typeof tmp !== 'number') {
            throw new TypeError('invalid bidx magic: ' + elem.id);
        }
        return tmp;
    }
    case 'plain': {
        let v = elem.value;
        assert(typeof v === 'number');
        if (!(isWholeFinite(v) && v >= -0x8000 && v <= 0x7fff)) {
            throw new TypeError('invalid plain value for magic: ' + v);
        }
        return v;
    }
    case 'math_onearg': {
        tmp = mathOneargMagic[elem.funcname];
        if (typeof tmp !== 'number') {
            throw new TypeError('invalid math_onearg magic: ' + elem.funcname);
        }
        return tmp;
    }
    case 'math_twoarg': {
        tmp = mathTwoargMagic[elem.funcname];
        if (typeof tmp !== 'number') {
            throw new TypeError('invalid math_twoarg magic: ' + elem.funcname);
        }
        return tmp;
    }
    case 'array_iter': {
        tmp = arrayIterMagic[elem.funcname];
        if (typeof tmp !== 'number') {
            throw new TypeError('invalid array_iter magic: ' + elem.funcname);
        }
        return tmp;
    }
    case 'typedarray_constructor': {
        return magicTypedArrayConstructor(elem.elem, elem.shift);
    }
    case 'buffer_readfield': {
        return magicReadField(elem.elem, elem.signed, elem.bigendian, elem.typedarray);
    }
    case 'buffer_writefield': {
        return magicWriteField(elem.elem, elem.signed, elem.bigendian, elem.typedarray);
    }
    default: {
        throw new TypeError('invalid magic type: ' + elem.type);
    }
    }
}
exports.resolveMagic = resolveMagic;
