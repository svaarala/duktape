'use strict';

// Class names, numeric indices must match duk_hobject.h class numbers.
const classNames = [
    'Unused',
    'Object',
    'Array',
    'Function',
    'Arguments',
    'Boolean',
    'Date',
    'Error',
    'JSON',
    'Math',
    'Number',
    'RegExp',
    'String',
    'global',
    'Symbol',
    'ObjEnv',
    'DecEnv',
    'Pointer',
    'Thread'
    // Remaining class names are not currently needed.
];
const classToNum = {};
classNames.forEach((n, idx) =>{
    classToNum[n] = idx;
});

function classToNumber(x) {
    return classToNum[x];
}
exports.classToNumber = classToNumber;
