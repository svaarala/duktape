/*
 *  Check built-in objects and their properties in detail.
 *
 *  There are small deviations from the E5 specification in all of the
 *  Javascript engines tested, so expect to see some differences to the
 *  required output when testing.
 */

/*
 *  These are from some random versions of the Javascript engines.
 *
 *  Smjs issues
 *
 *    - None that I can see :)
 *
 *  V8 (Node) issues
 *
 *    - Error object 'name' properties are not writable or configurable as they
 *      should be.  This is not explicitly stated, but: (1) the default attributes
 *      are stated in E5 Section 15 (last paragraph): Writable, !Enumerable, Configurable;
 *      and (2) the wording in e.g. E5 Sections 15.11.4.2 and 15.11.7.9 is that the
 *      *initial* value of 'name' and 'message' is something.  This suggests they are
 *      at least writable.
 *
 *    - Error.prototype has a 'message' property but specific built-in error prototypes
 *      (such as TypeError.prototype) do not.  They should, see E5 Section 15.11.7.10.
 *
 *    - Not a bug as such: global object prototype does not match Rhino or Smjs
 *      which both use Object.prototype.  However, global object prototype is
 *      implementation specific, so this is OK.
 *
 *  Rhino issues
 *
 *    - Global object 'RegExp' property has incorrect attributes.  It should be
 *      writable and configurable (but not enumerable); instead it is only
 *      configurable (not writable or enumerable).
 *
 *    - RegExp.length is 0, when it should be 2.  E5 Section 15.10.5.
 *
 *    - Date.length is 1, when it should be 7.  E5 Section 15.9.4.
 *
 *    - Error object 'name' and 'message' properties have incorrect attributes.
 *      They should be writable and configurable (but not enumerable); instead,
 *      they are writable, enumerable, and configurable.
 */

function getGlobalObject() {
    return this;
}

var obj_data = [
    {
        obj: getGlobalObject(),
        name: 'Global',
        proto: 'Object.prototype',    // implementation defined
                                      //  Smjs: Object.prototype
                                      //  Rhino: Object.prototype
                                      //  Node (V8): *not* Object.prototype, but Object.prototype is in prototype chain
        class: 'global',              // implementation defined
                                      //  Smjs: 'global'
                                      //  Rhino: 'global'
                                      //  V8: 'global'
        props: [
            { key: 'NaN', attrs: '' },
            { key: 'Infinity', attrs: '' },
            { key: 'undefined', attrs: '' },
            { key: 'eval', attrs: 'wc' },
            { key: 'parseInt', attrs: 'wc' },
            { key: 'parseFloat', attrs: 'wc' },
            { key: 'isNaN', attrs: 'wc' },
            { key: 'isFinite', attrs: 'wc' },
            { key: 'decodeURI', attrs: 'wc' },
            { key: 'decodeURIComponent', attrs: 'wc' },
            { key: 'encodeURI', attrs: 'wc' },
            { key: 'encodeURIComponent', attrs: 'wc' },
            { key: 'Object', attrs: 'wc' },
            { key: 'Function', attrs: 'wc' },
            { key: 'Array', attrs: 'wc' },
            { key: 'String', attrs: 'wc' },
            { key: 'Boolean', attrs: 'wc' },
            { key: 'Number', attrs: 'wc' },
            { key: 'Date', attrs: 'wc' },
            { key: 'RegExp', attrs: 'wc' },
            { key: 'Error', attrs: 'wc' },
            { key: 'EvalError', attrs: 'wc' },
            { key: 'RangeError', attrs: 'wc' },
            { key: 'ReferenceError', attrs: 'wc' },
            { key: 'SyntaxError', attrs: 'wc' },
            { key: 'TypeError', attrs: 'wc' },
            { key: 'URIError', attrs: 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: Object,
        name: 'Object',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 1 },
            { key: 'prototype', attrs: '' },
            { key: 'getPrototypeOf', attrs: 'wc' },
            { key: 'getOwnPropertyDescriptor', attrs: 'wc' },
            { key: 'getOwnPropertyNames', attrs: 'wc' },
            { key: 'create', attrs: 'wc' },
            { key: 'defineProperty', attrs: 'wc' },
            { key: 'seal', attrs: 'wc' },
            { key: 'freeze', attrs: 'wc' },
            { key: 'preventExtensions', attrs: 'wc' },
            { key: 'isSealed', attrs: 'wc' },
            { key: 'isFrozen', attrs: 'wc' },
            { key: 'isExtensible', attrs: 'wc' },
            { key: 'keys', attrs: 'wc' },
        ],
        noprops: [
        ],
    },
    {
        obj: Object.prototype,
        name: 'Object.prototype',
        proto: null,
        class: 'Object',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
            { key: 'toLocaleString', 'attrs': 'wc' },
            { key: 'valueOf', 'attrs': 'wc' },
            { key: 'hasOwnProperty', 'attrs': 'wc' },
            { key: 'isPrototypeOf', 'attrs': 'wc' },
            { key: 'propertyIsEnumerable', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: Function,
        name: 'Function',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 1 },
            { key: 'prototype', 'attrs': '' },
            { key: 'length', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: Function.prototype,
        name: 'Function.prototype',
        proto: 'Object.prototype',
        class: 'Function',  // E5 Section 15.3.4
        props: [
            { key: 'length', 'attrs': '', value: 0 },  // E5 Section 15.3.4
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
            { key: 'apply', 'attrs': 'wc' },
            { key: 'call', 'attrs': 'wc' },
            { key: 'bind', 'attrs': 'wc' },
        ],
        noprops: [
        ],
    },
    {
        obj: Array,
        name: 'Array',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 1 },
            { key: 'prototype', 'attrs': '' },
            { key: 'isArray', 'attrs': 'wc' },
        ],
        noprops: [
        ],
    },
    {
        obj: Array.prototype,
        name: 'Array.prototype',
        proto: 'Object.prototype',
        class: 'Array',
        props: [
            { key: 'length', 'attrs': 'w', value: 0 },  // virtual attribute (Array.prototype is an array instance),
                                                        // E5 Sections 15.4.4, 15.4.5.2
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
            { key: 'toLocaleString', 'attrs': 'wc' },
            { key: 'concat', 'attrs': 'wc' },
            { key: 'join', 'attrs': 'wc' },
            { key: 'pop', 'attrs': 'wc' },
            { key: 'push', 'attrs': 'wc' },
            { key: 'reverse', 'attrs': 'wc' },
            { key: 'shift', 'attrs': 'wc' },
            { key: 'slice', 'attrs': 'wc' },
            { key: 'sort', 'attrs': 'wc' },
            { key: 'splice', 'attrs': 'wc' },
            { key: 'unshift', 'attrs': 'wc' },
            { key: 'indexOf', 'attrs': 'wc' },
            { key: 'lastIndexOf', 'attrs': 'wc' },
            { key: 'every', 'attrs': 'wc' },
            { key: 'some', 'attrs': 'wc' },
            { key: 'forEach', 'attrs': 'wc' },
            { key: 'map', 'attrs': 'wc' },
            { key: 'filter', 'attrs': 'wc' },
            { key: 'reduce', 'attrs': 'wc' },
            { key: 'reduceRight', 'attrs': 'wc' },
        ],
        noprops: [
        ],
    },
    {
        obj: String,
        name: 'String',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 1 },
            { key: 'prototype', 'attrs': '' },
            { key: 'fromCharCode', 'attrs': 'wc' },
        ],
        noprops: [
        ],
    },
    {
        obj: String.prototype,
        name: 'String.prototype',
        proto: 'Object.prototype',
        class: 'String',  // E5 Section 15.5.4
        props: [
            { key: 'length', 'attrs': '', value: 0 },    // virtual attribute (String.prototype is a String instance),
                                                         // E5 Sections 15.5.4, 15.5.5.1; immutable
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
            { key: 'valueOf', 'attrs': 'wc' },
            { key: 'charAt', 'attrs': 'wc' },
            { key: 'charCodeAt', 'attrs': 'wc' },
            { key: 'concat', 'attrs': 'wc' },
            { key: 'indexOf', 'attrs': 'wc' },
            { key: 'lastIndexOf', 'attrs': 'wc' },
            { key: 'localeCompare', 'attrs': 'wc' },
            { key: 'match', 'attrs': 'wc' },
            { key: 'replace', 'attrs': 'wc' },
            { key: 'search', 'attrs': 'wc' },
            { key: 'slice', 'attrs': 'wc' },
            { key: 'split', 'attrs': 'wc' },
            { key: 'substring', 'attrs': 'wc' },
            { key: 'toLowerCase', 'attrs': 'wc' },
            { key: 'toLocaleLowerCase', 'attrs': 'wc' },
            { key: 'toUpperCase', 'attrs': 'wc' },
            { key: 'toLocaleUpperCase', 'attrs': 'wc' },
            { key: 'trim', 'attrs': 'wc' },
        ],
        noprops: [
        ],
    },
    {
        obj: Boolean,
        name: 'Boolean',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: Boolean.prototype,
        name: 'Boolean.prototype',
        proto: 'Object.prototype',
        class: 'Boolean',  // E5 Section 15.6.4
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
            { key: 'valueOf', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: Number,
        name: 'Number',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 1 },
            { key: 'prototype', 'attrs': '' },
            { key: 'MAX_VALUE', 'attrs': '' },
            { key: 'MIN_VALUE', 'attrs': '' },
            { key: 'NaN', 'attrs': '' },
            { key: 'NEGATIVE_INFINITY', 'attrs': '' },
            { key: 'POSITIVE_INFINITY', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: Number.prototype,
        name: 'Number.prototype',
        proto: 'Object.prototype',
        class: 'Number',  // E5 Section 15.7.4
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
            { key: 'toLocaleString', 'attrs': 'wc' },
            { key: 'valueOf', 'attrs': 'wc' },
            { key: 'toFixed', 'attrs': 'wc' },
            { key: 'toExponential', 'attrs': 'wc' },
            { key: 'toPrecision', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: Math,
        name: 'Math',
        proto: 'Object.prototype',
        class: 'Math',
        props: [
            { key: 'E', 'attrs': '' },
            { key: 'LN10', 'attrs': '' },
            { key: 'LN2', 'attrs': '' },
            { key: 'LOG2E', 'attrs': '' },
            { key: 'LOG10E', 'attrs': '' },
            { key: 'PI', 'attrs': '' },
            { key: 'SQRT1_2', 'attrs': '' },
            { key: 'SQRT2', 'attrs': '' },
            { key: 'abs', 'attrs': 'wc' },
            { key: 'acos', 'attrs': 'wc' },
            { key: 'asin', 'attrs': 'wc' },
            { key: 'atan', 'attrs': 'wc' },
            { key: 'atan2', 'attrs': 'wc' },
            { key: 'ceil', 'attrs': 'wc' },
            { key: 'cos', 'attrs': 'wc' },
            { key: 'exp', 'attrs': 'wc' },
            { key: 'floor', 'attrs': 'wc' },
            { key: 'log', 'attrs': 'wc' },
            { key: 'max', 'attrs': 'wc' },
            { key: 'min', 'attrs': 'wc' },
            { key: 'pow', 'attrs': 'wc' },
            { key: 'random', 'attrs': 'wc' },
            { key: 'round', 'attrs': 'wc' },
            { key: 'sin', 'attrs': 'wc' },
            { key: 'sqrt', 'attrs': 'wc' },
            { key: 'tan', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: Date,
        name: 'Date',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 7 },  // E5 Section 15.9.4
            { key: 'prototype', 'attrs': '' },
            { key: 'parse', 'attrs': 'wc' },
            { key: 'UTC', 'attrs': 'wc' },
            { key: 'now', 'attrs': 'wc' },
        ],
        noprops: [
        ],
    },
    {
        obj: Date.prototype,
        name: 'Date.prototype',
        proto: 'Object.prototype',
        class: 'Date',  // E5 Section 15.9.5
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
            { key: 'toDateString', 'attrs': 'wc' },
            { key: 'toTimeString', 'attrs': 'wc' },
            { key: 'toLocaleDateString', 'attrs': 'wc' },
            { key: 'toLocaleTimeString', 'attrs': 'wc' },
            { key: 'valueOf', 'attrs': 'wc' },
            { key: 'getTime', 'attrs': 'wc' },
            { key: 'getFullYear', 'attrs': 'wc' },
            { key: 'getUTCFullYear', 'attrs': 'wc' },
            { key: 'getMonth', 'attrs': 'wc' },
            { key: 'getUTCMonth', 'attrs': 'wc' },
            { key: 'getDate', 'attrs': 'wc' },
            { key: 'getUTCDate', 'attrs': 'wc' },
            { key: 'getDay', 'attrs': 'wc' },
            { key: 'getUTCDay', 'attrs': 'wc' },
            { key: 'getHours', 'attrs': 'wc' },
            { key: 'getUTCHours', 'attrs': 'wc' },
            { key: 'getMinutes', 'attrs': 'wc' },
            { key: 'getUTCMinutes', 'attrs': 'wc' },
            { key: 'getSeconds', 'attrs': 'wc' },
            { key: 'getUTCSeconds', 'attrs': 'wc' },
            { key: 'getMilliseconds', 'attrs': 'wc' },
            { key: 'getUTCMilliseconds', 'attrs': 'wc' },
            { key: 'getTimezoneOffset', 'attrs': 'wc' },
            { key: 'setTime', 'attrs': 'wc' },
            { key: 'setMilliseconds', 'attrs': 'wc' },
            { key: 'setUTCMilliseconds', 'attrs': 'wc' },
            { key: 'setSeconds', 'attrs': 'wc' },
            { key: 'setUTCSeconds', 'attrs': 'wc' },
            { key: 'setMinutes', 'attrs': 'wc' },
            { key: 'setUTCMinutes', 'attrs': 'wc' },
            { key: 'setHours', 'attrs': 'wc' },
            { key: 'setUTCHours', 'attrs': 'wc' },
            { key: 'setDate', 'attrs': 'wc' },
            { key: 'setUTCDate', 'attrs': 'wc' },
            { key: 'setMonth', 'attrs': 'wc' },
            { key: 'setUTCMonth', 'attrs': 'wc' },
            { key: 'setFullYear', 'attrs': 'wc' },
            { key: 'setUTCFullYear', 'attrs': 'wc' },
            { key: 'toUTCString', 'attrs': 'wc' },
            { key: 'toISOString', 'attrs': 'wc' },
            { key: 'toJSON', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: RegExp,
        name: 'RegExp',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', attrs: '', value: 2 },  // E5 Section 15.10.5
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: RegExp.prototype,
        name: 'RegExp.prototype',
        proto: 'Object.prototype',
        class: 'RegExp',  // E5 Section 15.10.6
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'exec', 'attrs': 'wc' },
            { key: 'test', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: Error,
        name: 'Error',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', 'attrs': '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: Error.prototype,
        name: 'Error.prototype',
        proto: 'Object.prototype',
        class: 'Error',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'name', 'attrs': 'wc' },
            { key: 'message', 'attrs': 'wc' },
            { key: 'toString', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: EvalError,
        name: 'EvalError',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', 'attrs': '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: EvalError.prototype,
        name: 'EvalError.prototype',
        proto: 'Error.prototype',
        class: 'Error',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'name', 'attrs': 'wc' },
            { key: 'message', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: RangeError,
        name: 'RangeError',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', 'attrs': '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: RangeError.prototype,
        name: 'RangeError.prototype',
        proto: 'Error.prototype',
        class: 'Error',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'name', 'attrs': 'wc' },
            { key: 'message', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: ReferenceError,
        name: 'ReferenceError',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', 'attrs': '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: ReferenceError.prototype,
        name: 'ReferenceError.prototype',
        proto: 'Error.prototype',
        class: 'Error',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'name', 'attrs': 'wc' },
            { key: 'message', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: SyntaxError,
        name: 'SyntaxError',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', 'attrs': '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: SyntaxError.prototype,
        name: 'SyntaxError.prototype',
        proto: 'Error.prototype',
        class: 'Error',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'name', 'attrs': 'wc' },
            { key: 'message', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: TypeError,
        name: 'TypeError',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', 'attrs': '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: TypeError.prototype,
        name: 'TypeError.prototype',
        proto: 'Error.prototype',
        class: 'Error',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'name', 'attrs': 'wc' },
            { key: 'message', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: URIError,
        name: 'URIError',
        proto: 'Function.prototype',
        class: 'Function',
        props: [
            { key: 'length', 'attrs': '', value: 1 },
            { key: 'prototype', 'attrs': '' },
        ],
        noprops: [
        ],
    },
    {
        obj: URIError.prototype,
        name: 'URIError.prototype',
        proto: 'Error.prototype',
        class: 'Error',
        props: [
            { key: 'constructor', 'attrs': 'wc' },
            { key: 'name', 'attrs': 'wc' },
            { key: 'message', 'attrs': 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
    {
        obj: JSON,
        name: 'JSON',
        proto: 'Object.prototype',
        class: 'JSON',
        props: [
            { key: 'parse', attrs: 'wc' },
            { key: 'stringify', attrs: 'wc' },
        ],
        noprops: [
            { key: 'length' },
        ],
    },
];

// E5 Section 15.2.4.2
function getObjectClass(obj) {
    // trickery to bind 'this' properly
    var func = Object.prototype.toString.bind(obj);

    var t = func();
    var r = /^\[object\s(.*?)\]$/;
    var m = r.exec(t);
    if (!m) {
        return 'none';
    } else {
        return m[1];
    }
}

function getObjectByName(objName) {
    var i;
    for (i = 0; i < obj_data.length; i++) {
        var obj = obj_data[i];
        if (obj.name == objName) {
            return obj;
        }
    }
    return null;
}

function printObject(obj) {
    var i;
    var t = 'OBJECT:';

    t += ' ';
    t += '"' + obj.name + '"';

    t += ' ';
    if (Object.isSealed(obj.obj)) {
        t += 'sealed';
    } else {
        t += '!sealed';
    }

    t += ' ';
    if (Object.isFrozen(obj.obj)) {
        t += 'frozen';
    } else {
        t += '!frozen';
    }

    t += ' ';
    if (Object.isExtensible(obj.obj)) {
        t += 'extensible';
    } else {
        t += '!extensible';
    }

    print (t);

    var iproto = Object.getPrototypeOf(obj.obj);
    var p_obj = null;
    if (obj.proto) {
        p_obj = getObjectByName(obj.proto);
    }
    t = 'PROTOTYPE: ';
    if (p_obj) {
        t += '"' + p_obj.name + '"';
    } else {
        t += 'null';
    }
    if ((iproto === null && p_obj === null) || (p_obj !== null && p_obj.obj === iproto)) {
        // ok
    } else {
        t += ' INCORRECT';
    }

    print (t);

    t = 'CLASS: ';
    var cl = getObjectClass(obj.obj);
    t += cl;
    if (obj.class !== cl) {
        t += ' INCORRECT, expected: ' + obj.class;
    }
    print (t);

    for (i = 0; i < obj.props.length; i++) {
        var p = obj.props[i];
        var pd = Object.getOwnPropertyDescriptor(obj.obj, p.key);
        var at = '';

        t = 'PROPERTY: ';
        t += '"' + p.key + '"';

        if (!pd) {
            t += ' ';
            t += 'MISSING'
            print(t);
            continue;
        }

        t += ' ';
        if (pd.writable) {
            at += 'w';
            t += 'writable';
        } else {
            t += '!writable';
        }

        t += ' ';
        if (pd.enumerable) {
            at += 'e';
            t += 'enumerable';
        } else {
            t += '!enumerable';
        }

        t += ' ';
        if (pd.configurable) {
            at += 'c';
            t += 'configurable';
        } else {
            t += '!configurable';
        }

        if (at != p.attrs) {
            t += ' ';
            t += 'ATTRIBUTES-INCORRECT';
        }

        if (p.value !== undefined) {
            if (p.value !== pd.value) {
                t += ' VALUE-INCORRECT';
            }
        }

        print (t);
    }

    for (i = 0; i < obj.noprops.length; i++) {
        var p = obj.noprops[i];
        var pd = Object.getOwnPropertyDescriptor(obj.obj, p.key);

        t = 'NOPROPERTY: ';

        t += '"' + p.key + '"';

        t += ' ';
        if (pd) {
            t += 'UNEXPECTEDLY-FOUND';
            t += ' ';
            t += JSON.stringify(pd);
        } else {
            // ok
        }

        print (t);
    }
}

/*===
OBJECT: "Global" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: global
PROPERTY: "NaN" !writable !enumerable !configurable
PROPERTY: "Infinity" !writable !enumerable !configurable
PROPERTY: "undefined" !writable !enumerable !configurable
PROPERTY: "eval" writable !enumerable configurable
PROPERTY: "parseInt" writable !enumerable configurable
PROPERTY: "parseFloat" writable !enumerable configurable
PROPERTY: "isNaN" writable !enumerable configurable
PROPERTY: "isFinite" writable !enumerable configurable
PROPERTY: "decodeURI" writable !enumerable configurable
PROPERTY: "decodeURIComponent" writable !enumerable configurable
PROPERTY: "encodeURI" writable !enumerable configurable
PROPERTY: "encodeURIComponent" writable !enumerable configurable
PROPERTY: "Object" writable !enumerable configurable
PROPERTY: "Function" writable !enumerable configurable
PROPERTY: "Array" writable !enumerable configurable
PROPERTY: "String" writable !enumerable configurable
PROPERTY: "Boolean" writable !enumerable configurable
PROPERTY: "Number" writable !enumerable configurable
PROPERTY: "Date" writable !enumerable configurable
PROPERTY: "RegExp" writable !enumerable configurable
PROPERTY: "Error" writable !enumerable configurable
PROPERTY: "EvalError" writable !enumerable configurable
PROPERTY: "RangeError" writable !enumerable configurable
PROPERTY: "ReferenceError" writable !enumerable configurable
PROPERTY: "SyntaxError" writable !enumerable configurable
PROPERTY: "TypeError" writable !enumerable configurable
PROPERTY: "URIError" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "Object" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable
PROPERTY: "getPrototypeOf" writable !enumerable configurable
PROPERTY: "getOwnPropertyDescriptor" writable !enumerable configurable
PROPERTY: "getOwnPropertyNames" writable !enumerable configurable
PROPERTY: "create" writable !enumerable configurable
PROPERTY: "defineProperty" writable !enumerable configurable
PROPERTY: "seal" writable !enumerable configurable
PROPERTY: "freeze" writable !enumerable configurable
PROPERTY: "preventExtensions" writable !enumerable configurable
PROPERTY: "isSealed" writable !enumerable configurable
PROPERTY: "isFrozen" writable !enumerable configurable
PROPERTY: "isExtensible" writable !enumerable configurable
PROPERTY: "keys" writable !enumerable configurable

OBJECT: "Object.prototype" !sealed !frozen extensible
PROTOTYPE: null
CLASS: Object
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
PROPERTY: "toLocaleString" writable !enumerable configurable
PROPERTY: "valueOf" writable !enumerable configurable
PROPERTY: "hasOwnProperty" writable !enumerable configurable
PROPERTY: "isPrototypeOf" writable !enumerable configurable
PROPERTY: "propertyIsEnumerable" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "Function" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable
PROPERTY: "length" !writable !enumerable !configurable

OBJECT: "Function.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
PROPERTY: "apply" writable !enumerable configurable
PROPERTY: "call" writable !enumerable configurable
PROPERTY: "bind" writable !enumerable configurable

OBJECT: "Array" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable
PROPERTY: "isArray" writable !enumerable configurable

OBJECT: "Array.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: Array
PROPERTY: "length" writable !enumerable !configurable
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
PROPERTY: "toLocaleString" writable !enumerable configurable
PROPERTY: "concat" writable !enumerable configurable
PROPERTY: "join" writable !enumerable configurable
PROPERTY: "pop" writable !enumerable configurable
PROPERTY: "push" writable !enumerable configurable
PROPERTY: "reverse" writable !enumerable configurable
PROPERTY: "shift" writable !enumerable configurable
PROPERTY: "slice" writable !enumerable configurable
PROPERTY: "sort" writable !enumerable configurable
PROPERTY: "splice" writable !enumerable configurable
PROPERTY: "unshift" writable !enumerable configurable
PROPERTY: "indexOf" writable !enumerable configurable
PROPERTY: "lastIndexOf" writable !enumerable configurable
PROPERTY: "every" writable !enumerable configurable
PROPERTY: "some" writable !enumerable configurable
PROPERTY: "forEach" writable !enumerable configurable
PROPERTY: "map" writable !enumerable configurable
PROPERTY: "filter" writable !enumerable configurable
PROPERTY: "reduce" writable !enumerable configurable
PROPERTY: "reduceRight" writable !enumerable configurable

OBJECT: "String" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable
PROPERTY: "fromCharCode" writable !enumerable configurable

OBJECT: "String.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: String
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
PROPERTY: "valueOf" writable !enumerable configurable
PROPERTY: "charAt" writable !enumerable configurable
PROPERTY: "charCodeAt" writable !enumerable configurable
PROPERTY: "concat" writable !enumerable configurable
PROPERTY: "indexOf" writable !enumerable configurable
PROPERTY: "lastIndexOf" writable !enumerable configurable
PROPERTY: "localeCompare" writable !enumerable configurable
PROPERTY: "match" writable !enumerable configurable
PROPERTY: "replace" writable !enumerable configurable
PROPERTY: "search" writable !enumerable configurable
PROPERTY: "slice" writable !enumerable configurable
PROPERTY: "split" writable !enumerable configurable
PROPERTY: "substring" writable !enumerable configurable
PROPERTY: "toLowerCase" writable !enumerable configurable
PROPERTY: "toLocaleLowerCase" writable !enumerable configurable
PROPERTY: "toUpperCase" writable !enumerable configurable
PROPERTY: "toLocaleUpperCase" writable !enumerable configurable
PROPERTY: "trim" writable !enumerable configurable

OBJECT: "Boolean" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "Boolean.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: Boolean
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
PROPERTY: "valueOf" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "Number" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable
PROPERTY: "MAX_VALUE" !writable !enumerable !configurable
PROPERTY: "MIN_VALUE" !writable !enumerable !configurable
PROPERTY: "NaN" !writable !enumerable !configurable
PROPERTY: "NEGATIVE_INFINITY" !writable !enumerable !configurable
PROPERTY: "POSITIVE_INFINITY" !writable !enumerable !configurable

OBJECT: "Number.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: Number
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
PROPERTY: "toLocaleString" writable !enumerable configurable
PROPERTY: "valueOf" writable !enumerable configurable
PROPERTY: "toFixed" writable !enumerable configurable
PROPERTY: "toExponential" writable !enumerable configurable
PROPERTY: "toPrecision" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "Math" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: Math
PROPERTY: "E" !writable !enumerable !configurable
PROPERTY: "LN10" !writable !enumerable !configurable
PROPERTY: "LN2" !writable !enumerable !configurable
PROPERTY: "LOG2E" !writable !enumerable !configurable
PROPERTY: "LOG10E" !writable !enumerable !configurable
PROPERTY: "PI" !writable !enumerable !configurable
PROPERTY: "SQRT1_2" !writable !enumerable !configurable
PROPERTY: "SQRT2" !writable !enumerable !configurable
PROPERTY: "abs" writable !enumerable configurable
PROPERTY: "acos" writable !enumerable configurable
PROPERTY: "asin" writable !enumerable configurable
PROPERTY: "atan" writable !enumerable configurable
PROPERTY: "atan2" writable !enumerable configurable
PROPERTY: "ceil" writable !enumerable configurable
PROPERTY: "cos" writable !enumerable configurable
PROPERTY: "exp" writable !enumerable configurable
PROPERTY: "floor" writable !enumerable configurable
PROPERTY: "log" writable !enumerable configurable
PROPERTY: "max" writable !enumerable configurable
PROPERTY: "min" writable !enumerable configurable
PROPERTY: "pow" writable !enumerable configurable
PROPERTY: "random" writable !enumerable configurable
PROPERTY: "round" writable !enumerable configurable
PROPERTY: "sin" writable !enumerable configurable
PROPERTY: "sqrt" writable !enumerable configurable
PROPERTY: "tan" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "Date" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable
PROPERTY: "parse" writable !enumerable configurable
PROPERTY: "UTC" writable !enumerable configurable
PROPERTY: "now" writable !enumerable configurable

OBJECT: "Date.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: Date
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
PROPERTY: "toDateString" writable !enumerable configurable
PROPERTY: "toTimeString" writable !enumerable configurable
PROPERTY: "toLocaleDateString" writable !enumerable configurable
PROPERTY: "toLocaleTimeString" writable !enumerable configurable
PROPERTY: "valueOf" writable !enumerable configurable
PROPERTY: "getTime" writable !enumerable configurable
PROPERTY: "getFullYear" writable !enumerable configurable
PROPERTY: "getUTCFullYear" writable !enumerable configurable
PROPERTY: "getMonth" writable !enumerable configurable
PROPERTY: "getUTCMonth" writable !enumerable configurable
PROPERTY: "getDate" writable !enumerable configurable
PROPERTY: "getUTCDate" writable !enumerable configurable
PROPERTY: "getDay" writable !enumerable configurable
PROPERTY: "getUTCDay" writable !enumerable configurable
PROPERTY: "getHours" writable !enumerable configurable
PROPERTY: "getUTCHours" writable !enumerable configurable
PROPERTY: "getMinutes" writable !enumerable configurable
PROPERTY: "getUTCMinutes" writable !enumerable configurable
PROPERTY: "getSeconds" writable !enumerable configurable
PROPERTY: "getUTCSeconds" writable !enumerable configurable
PROPERTY: "getMilliseconds" writable !enumerable configurable
PROPERTY: "getUTCMilliseconds" writable !enumerable configurable
PROPERTY: "getTimezoneOffset" writable !enumerable configurable
PROPERTY: "setTime" writable !enumerable configurable
PROPERTY: "setMilliseconds" writable !enumerable configurable
PROPERTY: "setUTCMilliseconds" writable !enumerable configurable
PROPERTY: "setSeconds" writable !enumerable configurable
PROPERTY: "setUTCSeconds" writable !enumerable configurable
PROPERTY: "setMinutes" writable !enumerable configurable
PROPERTY: "setUTCMinutes" writable !enumerable configurable
PROPERTY: "setHours" writable !enumerable configurable
PROPERTY: "setUTCHours" writable !enumerable configurable
PROPERTY: "setDate" writable !enumerable configurable
PROPERTY: "setUTCDate" writable !enumerable configurable
PROPERTY: "setMonth" writable !enumerable configurable
PROPERTY: "setUTCMonth" writable !enumerable configurable
PROPERTY: "setFullYear" writable !enumerable configurable
PROPERTY: "setUTCFullYear" writable !enumerable configurable
PROPERTY: "toUTCString" writable !enumerable configurable
PROPERTY: "toISOString" writable !enumerable configurable
PROPERTY: "toJSON" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "RegExp" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "RegExp.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: RegExp
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "exec" writable !enumerable configurable
PROPERTY: "test" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "Error" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "Error.prototype" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: Error
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "name" writable !enumerable configurable
PROPERTY: "message" writable !enumerable configurable
PROPERTY: "toString" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "EvalError" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "EvalError.prototype" !sealed !frozen extensible
PROTOTYPE: "Error.prototype"
CLASS: Error
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "name" writable !enumerable configurable
PROPERTY: "message" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "RangeError" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "RangeError.prototype" !sealed !frozen extensible
PROTOTYPE: "Error.prototype"
CLASS: Error
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "name" writable !enumerable configurable
PROPERTY: "message" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "ReferenceError" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "ReferenceError.prototype" !sealed !frozen extensible
PROTOTYPE: "Error.prototype"
CLASS: Error
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "name" writable !enumerable configurable
PROPERTY: "message" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "SyntaxError" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "SyntaxError.prototype" !sealed !frozen extensible
PROTOTYPE: "Error.prototype"
CLASS: Error
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "name" writable !enumerable configurable
PROPERTY: "message" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "TypeError" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "TypeError.prototype" !sealed !frozen extensible
PROTOTYPE: "Error.prototype"
CLASS: Error
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "name" writable !enumerable configurable
PROPERTY: "message" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "URIError" !sealed !frozen extensible
PROTOTYPE: "Function.prototype"
CLASS: Function
PROPERTY: "length" !writable !enumerable !configurable
PROPERTY: "prototype" !writable !enumerable !configurable

OBJECT: "URIError.prototype" !sealed !frozen extensible
PROTOTYPE: "Error.prototype"
CLASS: Error
PROPERTY: "constructor" writable !enumerable configurable
PROPERTY: "name" writable !enumerable configurable
PROPERTY: "message" writable !enumerable configurable
NOPROPERTY: "length" 

OBJECT: "JSON" !sealed !frozen extensible
PROTOTYPE: "Object.prototype"
CLASS: JSON
PROPERTY: "parse" writable !enumerable configurable
PROPERTY: "stringify" writable !enumerable configurable
NOPROPERTY: "length" 
===*/

/* XXX: the expected value needs to be verified against the specification */

function checkBuiltins() {
    var i;
    for (i = 0; i < obj_data.length; i++) {
        var t = obj_data[i];
        if (i > 0) {
            print('');
        };
        printObject(t);
    }
}

try {
    checkBuiltins();
} catch (e) {
    print(e.name, e.message);
}
