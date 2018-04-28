/*
 *  Custom Error properties.
 *
 *  ECMAScript engines vary a lot in what Error properties exist and how they
 *  are implemented (e.g. as own properties, inherited properties, accessors,
 *  enumerability).
 *
 *  Test strictly for the current Duktape behavior, to detect accidental
 *  changes and regressions.
 */

/*---
{
    "custom": true
}
---*/

/*===
own properties
"message"
enumerable properties (including inherited)
all properties (including non-enumerable and inherited)
---
"message"
---
"constructor"
"name"
"message"
"stack"
"fileName"
"lineNumber"
"toString"
---
"constructor"
"__proto__"
"toString"
"toLocaleString"
"valueOf"
"hasOwnProperty"
"isPrototypeOf"
"propertyIsEnumerable"
"__defineGetter__"
"__defineSetter__"
"__lookupGetter__"
"__lookupSetter__"
---
function function false true
function function false true
function function false true
===*/

function test() {
    var err;
    var k;
    var obj;
    var pd;

    err = new Error('my message');

    // Only 'message' is an own property.  Stack trace etc are accessors
    // and back into an internal _tracedata property which cannot normally
    // be accessed from ECMAScript code.

    print('own properties');
    Object.getOwnPropertyNames(err).forEach(function (k) {
        print(JSON.stringify(k));
    });

    // No enumerable properties at all.

    print('enumerable properties (including inherited)');
    for (k in err) {
        print(k);
    }

    // Non-enumerable inherited properties.

    print('all properties (including non-enumerable and inherited)');
    obj = err;
    while (obj) {
        print('---');
        Object.getOwnPropertyNames(obj).forEach(function (k) {
            print(JSON.stringify(k));
        });
        obj = Object.getPrototypeOf(obj);
    }
    print('---');

    // Check a few Duktape 2.x property attributes.
    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'fileName');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'lineNumber');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'stack');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
