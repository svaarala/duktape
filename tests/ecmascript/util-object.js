function getDuktapeInfoField(val, arrIdx, propName) {
    var i = Duktape.info(val);
    if (typeof i === 'object' && i[arrIdx] !== void 0) {
        return i[arrIdx];
    } else if (typeof i === 'object' && i[propName] !== void 0) {
        return i[propName];
    }
    throw new Error('cannot parse Duktape.info() result, propName: ' + propName);
}

// Get object property count.
function getObjectPropertyCount(obj) {
    return getDuktapeInfoField(obj, 6, 'enext');
}

// Get object refcount.
function getObjectRefcount(obj) {
    return getDuktapeInfoField(obj, 2, 'refc');
}

// Get object bytecode size.
function getObjectBytecodeSize(obj) {
    return getDuktapeInfoField(obj, 9, 'bcbytes');
}

// Get object entry part size.
function getObjectEntrySize(obj) {
    return getDuktapeInfoField(obj, 5, 'esize');
}

// Get object entry part 'next' field.
function getObjectEntryNext(obj) {
    return getDuktapeInfoField(obj, 6, 'enext');
}

// Check if value is a lightfunc.
function valueIsLightFunc(val) {
    return getDuktapeInfoField(val, 0, 'type') === 9;
}

// Get public type number for a value.
function getValuePublicType(val) {
    return getDuktapeInfoField(val, 0, 'type');
}

// Get internal type tag of a value; depends on duk_tval packing.
function getValueInternalTag(val) {
    return getDuktapeInfoField(val, 1, 'itag');
}

// String convert a descriptor object.
function propDescToString(pd) {
    function fmt(v) {
        if (typeof v === 'function') {
            return 'function';
        }
        return Duktape.enc('jx', v);
    }

    if (!pd) {
        return 'none';
    }

    var res = [];
    if (pd.value !== void 0) {
        res.push('value=' + fmt(pd.value));
    }
    if (pd.get !== void 0) {
        res.push('get=' + fmt(pd.get));
    }
    if (pd.set !== void 0) {
        res.push('set=' + fmt(pd.set));
    }
    if (pd.writable !== void 0) {
        res.push('writable=' + fmt(pd.writable));
    }
    if (pd.enumerable !== void 0) {
        res.push('enumerable=' + fmt(pd.enumerable));
    }
    if (pd.configurable !== void 0) {
        res.push('configurable=' + fmt(pd.configurable));
    }

    return res.join(',');
}

// Print a descriptor object.
function printPropDesc(pd) {
    print(propDescToString(pd));
}

// Print a key and a related property descriptor.
function printKeyPropDesc(obj, key) {
    var pd = Object.getOwnPropertyDescriptor(obj, key);
    print('key: ' + key + ', desc: ' + propDescToString(pd));
}
