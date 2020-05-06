'use strict';

const { BitEncoder } = require('../util/bitencoder');
const { bitpack5BitBstr } = require('../formats/bitpack_5bit');
const { walkObjectsAndProperties, findObjectById, findPropertyByKey, propDefault } = require('./metadata/util');
const { classToNumber } = require('./classnames');
const { assert } = require('../util/assert');
const { hexDecode } = require('../util/hex');
const { shallowCloneArray } = require('../util/clone');
const { createBareObject } = require('../util/bare');
const { shuffleDoubleBytesFromBigEndian } = require('../util/double');

// Default property attributes.
const LENGTH_PROPERTY_ATTRIBUTES = 'c';
//const ACCESSOR_PROPERTY_ATTRIBUTES = 'c';
const DEFAULT_DATA_PROPERTY_ATTRIBUTES = 'wc';
const DEFAULT_FUNC_PROPERTY_ATTRIBUTES = 'wc';

// Encoding constants (must match duk_hthread_builtins.c).
const PROP_FLAGS_BITS = 3;
const LENGTH_PROP_BITS = 3;
const NARGS_BITS = 3;
const PROP_TYPE_BITS = 3;

const NARGS_VARARGS_MARKER = 0x07;

const PROP_TYPE_DOUBLE = 0;
const PROP_TYPE_STRING = 1;
const PROP_TYPE_STRIDX = 2;
const PROP_TYPE_BUILTIN = 3;
const PROP_TYPE_UNDEFINED = 4;
const PROP_TYPE_BOOLEAN_TRUE = 5;
const PROP_TYPE_BOOLEAN_FALSE = 6;
const PROP_TYPE_ACCESSOR = 7;

// Property descriptor flags, must match duk_hobject.h.
const PROPDESC_FLAG_WRITABLE = (1 << 0);
const PROPDESC_FLAG_ENUMERABLE = (1 << 1);
const PROPDESC_FLAG_CONFIGURABLE = (1 << 2);
const PROPDESC_FLAG_ACCESSOR = (1 << 3);

// Encode property flags for RAM initializers.
function encodePropertyFlags(flags) {
    var res = 0;
    var nflags = 0;
    if (flags.indexOf('w') >= 0) {
        nflags++;
        res |= PROPDESC_FLAG_WRITABLE;
    }
    if (flags.indexOf('e') >= 0) {
        nflags++;
        res |= PROPDESC_FLAG_ENUMERABLE;
    }
    if (flags.indexOf('c') >= 0) {
        nflags++;
        res |= PROPDESC_FLAG_CONFIGURABLE;
    }
    if (flags.indexOf('a') >= 0) {
        nflags++;
        res |= PROPDESC_FLAG_ACCESSOR;
    }

    if (nflags !== flags.length) {
        throw new TypeError('invalid property flags: ' + flags);
    }

    return res;
}

// Get helper maps for RAM objects.
function getRamobjNativeFuncMaps(meta) {
    var nativeFound = {};
    var nativeFuncs = [];
    var natfuncNameToNatidx = {};

    nativeFuncs.push(null);  // natidx 0 is reserved for NULL

    walkObjectsAndProperties(meta, (o) => {
        if (typeof o.native !== 'undefined') {
            nativeFound[o.native] = true;
        }
    }, (p, o) => {
        void o;
        let val = p.value;
        if (typeof val === 'object' && val !== null) {
            switch (val.type) {
            case 'accessor':
                if (typeof val.getter_id !== 'undefined') {
                        let getter = findObjectById(meta, val.getter_id);
                        nativeFound[getter.native] = true;
                }
                if (typeof val.setter_id !== 'undefined') {
                        let setter = findObjectById(meta, val.setter_id);
                        nativeFound[setter.native] = true;
                }
                break;
            case 'object':
                {
                    let target = findObjectById(meta, val.id);
                    if (typeof target.native !== 'undefined') {
                        nativeFound[target.native] = true;
                    }
                }
                break;
            case 'lightfunc':
                // No lightfunc support for RAM initializer now.
                break;
            }
        }
    });

    Object.keys(nativeFound).sort().forEach((k, idx) => {
        void idx;
        natfuncNameToNatidx[k] = nativeFuncs.length;
        nativeFuncs.push(k);  // native func names
    });

    return { nativeFuncs, natfuncNameToNatidx };
}
exports.getRamobjNativeFuncMaps = getRamobjNativeFuncMaps;

// Generate bit-packed RAM string init data.
function generateRamStringInitDataBitpacked(meta) {
    var be = new BitEncoder();

    var maxLen = 0;
    var stats = createBareObject({
        numInputBytes: 0,
        numOptimal: 0,
        numLookup1: 0,
        numLookup2: 0,
        numSwitch1: 0,
        numSwitch: 0,
        numEightBit: 0
    });

    for (let strObj of meta.strings_stridx) {
        let s = strObj.str;
        maxLen = Math.max(maxLen, s.length);
        bitpack5BitBstr(be, s, stats);
    }

    // End marker not necessary, C code knows length from define.

    console.debug('RAM string init data: ' + be.getStatsString());
    let res = be.getBytes();

    console.debug(meta.strings_stridx.length + ' ram strings, ' + stats.numInputBytes + ' input data bytes, ' +
                  res.length + ' bytes of string init data, ' + maxLen + ' maximum string length, ' +
                  (res.length * 8 / stats.numInputBytes) + ' bits/char, ' +
                  'encoding stats: ' + JSON.stringify(stats));

    return { data: res, maxLen: maxLen };
}
exports.generateRamStringInitDataBitpacked = generateRamStringInitDataBitpacked;

// Helper to find a property from a property list, remove it from the
// property list, and return the removed property.
function stealProp(props, key, opts) {
    opts = opts || {};
    for (let i = 0; i < props.length; i++) {
        let prop = props[i];
        if (prop.key === key) {
            let isAccessor = (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'accessor');
            if (!isAccessor || opts.allowAccessor) {
                var res = props.splice(i, 1);
                return res[0];
            }
        }
    }
}

// Generate init data for an object, no properties yet.
function generateRamObjectInitDataForObject(meta, be, obj, stringToStridx, natfuncNameToNatidx, objIdToBidx) {
    void objIdToBidx;
    function emitStridx(strval) {
        var stridx = stringToStridx[strval];
        be.varuint(stridx);
    }
    void emitStridx;
    function emitStridxOrString(strval) {
        var stridx = stringToStridx[strval];
        if (typeof stridx === 'number') {
            be.varuint(stridx + 1);
        } else {
            be.varuint(0);
            bitpack5BitBstr(be, strval, null);
        }
    }
    function emitNatidx(nativeName) {
        var natidx = natfuncNameToNatidx[nativeName];
        be.varuint(natidx);
    }

    var classNum = classToNumber(obj.class);
    be.varuint(classNum);

    var props = shallowCloneArray(obj.properties);  // Clone so we can steal.

    var propProto = stealProp(props, 'prototype', { allowAccessor: false });
    void propProto;
    var propConstructor = stealProp(props, 'constructor', { allowAccessor: false });
    void propConstructor;
    var propName = stealProp(props, 'name', { allowAccessor: false });
    var propLength = stealProp(props, 'length', { allowAccessor: false });

    var length = -1;  // default value, signifies varargs
    if (propLength) {
        assert(typeof propLength.value === 'number');
        length = propLength.value;
        be.bits(1, 1);  // flag: have length
        be.bits(length, LENGTH_PROP_BITS);
    } else {
        be.bits(0, 1);  // flag: no length
    }

    // The attributes for 'length' are standard ("none") except for
    // Array.prototype.length which must be writable (this is handled
    // separately in duk_hthread_builtins.c).

    var lenAttrs = LENGTH_PROPERTY_ATTRIBUTES;
    if (propLength) {
        lenAttrs = propLength.attributes;
    }
    if (lenAttrs !== LENGTH_PROPERTY_ATTRIBUTES) {
        // Attributes are assumed to be the same, except for Array.prototype.
        if (obj.class !== 'Array') {  // Array.prototype is the only one with this class
            throw new TypeError('non-default length attributes for unexpected object');
        }
    }

    // For 'Function' classed objects, emit the native function stuff.
    // Unfortunately this is more or less a copy of what we do for
    // function properties now.  This should be addressed if a rework
    // on the init format is done.

    if (obj.class === 'Function') {
        emitNatidx(obj.native);

        // Nargs.
        if (propDefault(obj, 'varargs', false)) {
            be.bits(1, 1);  // flag: non-default nargs
            be.bits(NARGS_VARARGS_MARKER, NARGS_BITS);  // varargs
        } else if (typeof obj.nargs === 'number' && obj.nargs !== length) {
            be.bits(1, 1);  // flag: non-default nargs
            be.bits(obj.nargs, NARGS_BITS);
        } else {
            assert(typeof length === 'number');
            be.bits(0, 1);  // flag: default nargs is OK
        }

        // Function .name.
        assert(propName);
        assert(typeof propName.value === 'string');
        emitStridxOrString(propName.value);

        // All Function-classed global level objects are callable
        // (have [[Call]]) but not all are constructable (have
        // [[Construct]]).  Flag that.
        assert(obj.callable === true);
        if (propDefault(obj, 'constructable', false)) {
            be.bits(1, 1);  // flag: constructable
        } else {
            be.bits(0, 1);  // flag: not constructable
        }
        // DUK_HOBJECT_FLAG_SPECIAL_CALL is handled at runtime without init data.

        // Convert signed magic to 16-bit unsigned for encoding.
        var magic = propDefault(obj, 'magic', 0);
        assert(typeof magic === 'number');
        magic = magic & 0xffff;
        assert(magic >= 0 && magic <= 0xffff);
        be.varuint(magic);
    }
}

// Generate init data for object properties.
function generateRamObjectInitDataForProps(meta, be, obj, stringToStridx, natfuncNameToNatidx, objIdToBidx, doubleByteOrder) {
    var countNormal = 0;
    var countFunction = 0;

    function emitBidx(bi_id) {
        be.varuint(objIdToBidx[bi_id]);
    }
    function emitBidxOrNone(bi_id) {
        if (typeof bi_id === 'string') {
            be.varuint(objIdToBidx[bi_id] + 1);
        } else {
            be.varuint(0);
        }
    }
    function emitStridx(strval) {
        var stridx = stringToStridx[strval];
        be.varuint(stridx);
    }
    function emitStridxOrString(strval) {
        var stridx = stringToStridx[strval];
        if (typeof stridx === 'number') {
            be.varuint(stridx + 1);
        } else {
            be.varuint(0);
            bitpack5BitBstr(be, strval, null);
        }
    }
    function emitNatidx(nativeName) {
        var natidx;
        if (typeof nativeName === 'string') {
            natidx = natfuncNameToNatidx[nativeName];
        } else {
            natidx = 0;  // 0 is NULL in the native functions table, denotes missing function.
        }
        be.varuint(natidx);
    }

    var props = shallowCloneArray(obj.properties);  // Clone so we can steal.

    // Internal prototype: not an actual property so not in property list.
    emitBidxOrNone(obj.internal_prototype);

    // External prototype: encoded specially, steal from property list.
    var propProto = stealProp(props, 'prototype');
    if (propProto) {
        assert(typeof propProto.value === 'object' && propProto.value !== null && propProto.value.type === 'object');
        assert(propProto.attributes === '');
        emitBidxOrNone(propProto.value.id);
    } else {
        emitBidxOrNone(null);
    }

    // External constructor: encoded specially, steal from property list.
    var propConstructor = stealProp(props, 'constructor');
    if (propConstructor) {
        assert(typeof propConstructor.value === 'object' && propConstructor.value !== null && propConstructor.value.type === 'object');
        assert(propConstructor.attributes === 'wc');
        emitBidxOrNone(propConstructor.value.id);
    } else {
        emitBidxOrNone(null);
    }

    // Name: encoded specially for function objects, so steal and ignore here.
    if (obj.class === 'Function') {
        let propName = stealProp(props, 'name', { allowAccessor: false });
        assert(propName);
        assert(typeof propName.value === 'string');
        assert(propName.attributes === 'c');
    }

    // length: encoded specially, so steal and ignore.
    var propLength = stealProp(props, 'length', { allowAccessor: false });
    void propLength;

    // Date.prototype.toGMTString needs special handling and is handled
    // directly in duk_hthread_builtins.c; so steal and ignore here.
    if (obj.id === 'bi_date_prototype') {
        let propToGmtString = stealProp(props, 'toGMTString');
        void propToGmtString;
        // May be missing if built-ins edited.
        console.debug('stole .toGMTString property');
    }

    // Split properties into non-toplevel functions and other properties.
    // This split is a bit arbitrary, but is used to reduce flag bits in
    // the bit stream.
    var values = [];
    var functions = [];
    props.forEach((prop) => {
        if (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'object') {
            var target = findObjectById(meta, prop.value.id);
            assert(target);
            if (typeof target.native === 'string' &&   // native function
                typeof target.bidx === 'undefined') {  // but not a top level built-in
                functions.push(prop);
                return;
            }
        }
        values.push(prop);
    });
    console.debug(obj.id + ': ' + values.length + ' values, ' + functions.length + ' functions');

    // Encode 'values'.
    be.varuint(values.length);

    values.forEach((prop) => {
        var val = prop.value;

        countNormal++;

        // Key.
        emitStridxOrString(prop.key);

        // Attributes.  Attribute check doesn't check for accessor flag; that
        // is now automatically set by C code when value is an accessor type.
        // Accessors must not have 'writable', so they'll always have
        // non-default attributes (less footprint than adding a different
        // default).
        var defaultAttrs = DEFAULT_DATA_PROPERTY_ATTRIBUTES;
        var attrs = propDefault(prop, 'attributes', defaultAttrs);
        attrs = attrs.replace('a', '');  // RAM bitstream doesn't encode the 'accessor' attribute.
        if (attrs !== defaultAttrs) {
            console.debug('non-default attributes for ' + prop.key + ': ' + attrs + ' vs ' + defaultAttrs);
            be.bits(1, 1);  // flag: have custom attributes
            be.bits(encodePropertyFlags(attrs), PROP_FLAGS_BITS);
        } else {
            be.bits(0, 1);  // flag: no custom attributes
        }

        // Value.
        if (val === void 0 || val === null) {
            // RAM format doesn't support "null", use undefined.
            be.bits(PROP_TYPE_UNDEFINED, PROP_TYPE_BITS);
        } else if (typeof val === 'boolean') {
            if (val) {
                be.bits(PROP_TYPE_BOOLEAN_TRUE, PROP_TYPE_BITS);
            } else {
                be.bits(PROP_TYPE_BOOLEAN_FALSE, PROP_TYPE_BITS);
            }
        } else if (typeof val === 'number' || (typeof val === 'object' && val !== null && val.type === 'double')) {
            // Avoid converting a manually specified NaN temporarily into
            // a float to avoid risk of e.g. NaN being replaced by another.
            if (typeof val === 'object') {
                val = hexDecode(val.bytes);
            } else {
                let tmpAb = new ArrayBuffer(8);
                let tmpDv = new DataView(tmpAb);
                tmpDv.setFloat64(0, val);  // big endian
                val = new Uint8Array(tmpAb);
            }
            assert(val instanceof Uint8Array);
            assert(val.length === 8);

            be.bits(PROP_TYPE_DOUBLE, PROP_TYPE_BITS);

            let dataU8 = new Uint8Array(8);
            shuffleDoubleBytesFromBigEndian(val, dataU8, doubleByteOrder);

            be.uint8array(dataU8);
        } else if (typeof val === 'string') {
            let stridx = stringToStridx[val];
            if (typeof stridx === 'number') {
                // String value is in built-in string table -> encode
                // using a string index.  This saves some space,
                // especially for the 'name' property of errors
                // ('EvalError' etc).
                be.bits(PROP_TYPE_STRIDX, PROP_TYPE_BITS);
                emitStridx(val);
            } else {
                // Not in string table, bitpack string value as is.
                be.bits(PROP_TYPE_STRING, PROP_TYPE_BITS);
                bitpack5BitBstr(be, val);
            }
        } else if (typeof val === 'object' && val !== null) {
            if (val.type === 'object') {
                be.bits(PROP_TYPE_BUILTIN, PROP_TYPE_BITS);
                emitBidx(val.id);
            } else if (val.type === 'undefined') {
                be.bits(PROP_TYPE_UNDEFINED, PROP_TYPE_BITS);
            } else if (val.type === 'accessor') {
                be.bits(PROP_TYPE_ACCESSOR, PROP_TYPE_BITS);
                let getterNatfun, setterNatfun;
                let getterMagic = 0, setterMagic = 0;
                if (typeof val.getter_id === 'string') {
                    let getterFn = findObjectById(meta, val.getter_id);
                    getterNatfun = getterFn.native;
                    assert(getterFn.nargs === 0);
                    getterMagic = getterFn.magic;
                }
                if (typeof val.setter_id === 'string') {
                    let setterFn = findObjectById(meta, val.setter_id);
                    setterNatfun = setterFn.native;
                    assert(setterFn.nargs === 1);
                    setterMagic = setterFn.magic;
                }
                if (getterNatfun && setterNatfun) {
                    assert(getterMagic === setterMagic);
                }
                emitNatidx(getterNatfun);
                emitNatidx(setterNatfun);
                be.varuint(getterMagic);
            } else if (val.type === 'lightfunc') {
                console.log('RAM init data format doesn\'t support "lightfunc" now, value replaced with "undefined" for ' + prop.name);
                be.bits(PROP_TYPE_UNDEFINED, PROP_TYPE_BITS);
            } else {
                throw new TypeError('unsupported value');
            }
        } else {
            throw new TypeError('unsupported value');
        }
    });

    // Encode 'functions'.
    be.varuint(functions.length);

    functions.forEach((prop) => {
        var val = prop.value;
        var funObj = findObjectById(meta, val.id);
        assert(funObj);
        var propLen = findPropertyByKey(funObj, 'length');
        assert(propLen);
        assert(typeof propLen.value === 'number');
        var length = propLen.value;

        countFunction++;

        // Key.
        emitStridxOrString(prop.key);

        // Native function.
        emitNatidx(funObj.native);

        // Length.
        be.bits(length, LENGTH_PROP_BITS);

        // Nargs.
        if (propDefault(funObj, 'varargs', false)) {
            be.bits(1, 1);  // flag: non-default nargs
            be.bits(NARGS_VARARGS_MARKER, NARGS_BITS);
        } else if (typeof funObj.nargs === 'number' && funObj.nargs !== length) {
            be.bits(1, 1);  // flag: non-default nargs
            be.bits(funObj.nargs, NARGS_BITS);
        } else {
            be.bits(0, 1);  // flag: default nargs OK
        }

        // Magic.
        // Convert signed magic to 16-bit unsigned for encoding.
        var magic = propDefault(funObj, 'magic', 0);
        assert(typeof magic === 'number');
        magic = magic & 0xffff;
        assert(magic >= 0 && magic <= 0xffff);
        be.varuint(magic);

        // Property attributes.
        var defaultAttrs = DEFAULT_FUNC_PROPERTY_ATTRIBUTES;
        var attrs = propDefault(prop, 'attributes', defaultAttrs);
        attrs = attrs.replace('a', '');  // RAM bitstream doesn't encode the 'accessor' attribute.
        if (attrs !== defaultAttrs) {
            console.debug('non-default attributes for ' + prop.key + ': ' + attrs + ' vs ' + defaultAttrs);
            be.bits(1, 1);  // flag: have custom attributes
            be.bits(encodePropertyFlags(attrs), PROP_FLAGS_BITS);
        } else {
            be.bits(0, 1);  // flag: no custom attributes
        }
    });

    return { countNormal, countFunction };
}

// Generate init data for objects and their properties.
function generateRamObjectInitDataBitpacked(meta, nativeFuncs, natfuncNameToNatidx, doubleByteOrder) {
    // RAM initialization is based on a specially filtered list of top
    // level objects which includes objects with 'bidx' and objects
    // which aren't handled as inline values in the init bitstream.

    var objList = meta.objects_ram_toplevel
    var objIdToBidx = meta._objid_to_ramidx;
    var stringToStridx = meta._plain_to_stridx;

    var be = new BitEncoder();
    var countBuiltins = 0;
    var countNormalProps = 0;
    var countFunctionProps = 0;

    objList.forEach((obj) => {
        countBuiltins++;
        generateRamObjectInitDataForObject(meta, be, obj, stringToStridx, natfuncNameToNatidx, objIdToBidx);
    });
    objList.forEach((obj) => {
        var { countNormal, countFunction } = generateRamObjectInitDataForProps(meta, be, obj, stringToStridx, natfuncNameToNatidx, objIdToBidx, doubleByteOrder);
        countNormalProps += countNormal;
        countFunctionProps += countFunction;
    });

    var data = be.getBytes();
    console.debug('RAM object init data: ' + be.getStatsString());

    console.debug(countBuiltins + ' ram builtins, ' + countNormalProps + ' normal properties, ' +
                  countFunctionProps + ' function properties, ' + data.length + ' bytes of RAM object init data');

    return { data };
}
exports.generateRamObjectInitDataBitpacked = generateRamObjectInitDataBitpacked;

function emitRamObjectNativeFuncDeclarations(gcHdr, ramNativeFuncs) {
    ramNativeFuncs.forEach((fname) => {
        // Visibility depends on whether the function is Duktape internal or user.
        // Use a simple prefix check.
        if (fname === null) {
            // Zero index is special.
            return;
        }
        assert(typeof fname === 'string');
        let visibility = (fname.startsWith('duk_') ? 'DUK_INTERNAL_DECL' : 'extern');
        gcHdr.emitLine(visibility + ' duk_ret_t ' + fname + '(duk_context *ctx);');
    });
}
exports.emitRamObjectNativeFuncDeclarations = emitRamObjectNativeFuncDeclarations;

function emitRamObjectNativeFuncArray(gcSrc, ramNativeFuncs) {
    gcSrc.emitLine('DUK_INTERNAL const duk_c_function duk_bi_native_functions[' + ramNativeFuncs.length + '] = {');
    ramNativeFuncs.forEach((fname, idx) => {
        // The function pointer cast here makes BCC complain about
        // "initializer too complicated", so omit the cast.
        //gcSrc.emitLine('\t(duk_c_function) ' + func + ',');
        let comma = (idx < ramNativeFuncs.length - 1 ? ',' : '');
        if (fname === null) {
            gcSrc.emitLine('\tNULL' + comma);
        } else {
            assert(typeof fname === 'string');
            gcSrc.emitLine('\t' + fname + comma);
        }
    });
    gcSrc.emitLine('};');
}
exports.emitRamObjectNativeFuncArray = emitRamObjectNativeFuncArray;

function emitRamObjectNativeFuncArrayDeclaration(gcSrc, ramNativeFuncs) {
    gcSrc.emitLine('#if !defined(DUK_SINGLE_FILE)');
    gcSrc.emitLine('DUK_INTERNAL_DECL const duk_c_function duk_bi_native_functions[' + ramNativeFuncs.length + '];');
    gcSrc.emitLine('#endif');
}
exports.emitRamObjectNativeFuncArrayDeclaration = emitRamObjectNativeFuncArrayDeclaration;

function emitRamObjectInitData(gcSrc, data) {
    gcSrc.emitArray(data, {
        tableName: 'duk_builtins_data',
        typeName: 'duk_uint8_t',
        useConst: true,
        useCast: false,
        visibility: 'DUK_INTERNAL'
    });
}
exports.emitRamObjectInitData = emitRamObjectInitData;

function emitRamObjectInitDataDeclaration(gcHdr, data) {
    gcHdr.emitLine('#if !defined(DUK_SINGLE_FILE)');
    gcHdr.emitLine('DUK_INTERNAL_DECL const duk_uint8_t duk_builtins_data[' + data.length + '];');
    gcHdr.emitLine('#endif  /* !DUK_SINGLE_FILE */');
    gcHdr.emitDefine('DUK_BUILTINS_DATA_LENGTH', data.length);
}
exports.emitRamObjectInitDataDeclaration = emitRamObjectInitDataDeclaration;

function emitRamStringHeader(gcHdr, data, strMaxLen) {
    gcHdr.emitLine('#if !defined(DUK_SINGLE_FILE)');
    gcHdr.emitLine('DUK_INTERNAL_DECL const duk_uint8_t duk_strings_data[' + data.length + '];');
    gcHdr.emitLine('#endif  /* !DUK_SINGLE_FILE */');
    gcHdr.emitDefine('DUK_STRDATA_MAX_STRLEN', strMaxLen);
    gcHdr.emitDefine('DUK_STRDATA_DATA_LENGTH', data.length);
}
exports.emitRamStringHeader = emitRamStringHeader;

function emitRamStringInitData(gcSrc, data) {
    gcSrc.emitArray(data, {
        tableName: 'duk_strings_data',
        typeName: 'duk_uint8_t',
        useConst: true,
        useCast: false,
        visibility: 'DUK_INTERNAL'
    });
}
exports.emitRamStringInitData = emitRamStringInitData;

function emitRamObjectHeader(gcHdr, meta) {
    var objList = meta.objects_bidx;
    objList.forEach((obj, idx) => {
        var tmp = obj.id.toUpperCase().split('_').slice(1).join('_');  // bi_foo_bar -> FOO_BAR
        var defName = 'DUK_BIDX_' + tmp;
        gcHdr.emitDefine(defName, idx);
    });
    gcHdr.emitDefine('DUK_NUM_BUILTINS', objList.length);
    gcHdr.emitDefine('DUK_NUM_BIDX_BUILTINS', objList.length);  // Objects with 'bidx'
    gcHdr.emitDefine('DUK_NUM_ALL_BUILTINS', meta.objects_ram_toplevel.length);  // Objects with 'bidx' + temps needed in init.
}
exports.emitRamObjectHeader = emitRamObjectHeader;
