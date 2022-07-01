'use strict';

// Map built-ins class name to a duk_heaphdr htype.
const classToHtypeNum = {
    'Array': 8,
    'Object': 10,
    'Function': 13, // natfunc, assumes built-ins are native for now.
    'Boolean': 16,
    'Date': 17,
    'Error': 18,
    'JSON': 19,
    'Math': 20,
    'Number': 21,
    'RegExp': 22,
    'String': 23,
    'global': 24,
    'Symbol': 25,
    'ObjEnv': 26,
    'Pointer': 28,
    'Thread': 29,
    'Proxy': 30
};

function classToHtypeNumber(x) {
    var res = classToHtypeNum[x];
    if (typeof res !== 'number') {
        throw new TypeError('no htype number for class ' + x);
    }
    return res;
}
exports.classToHtypeNumber = classToHtypeNumber;
