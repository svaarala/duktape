/*
 *  Behavior of lightweight functions in various situations.
 *
 *  Also documents the detailed behavior and limitations of lightfuncs.
 */

/*---
{
    "custom": true,
    "specialoptions": "DUK_OPT_LIGHTFUNC_BUILTINS"
}
---*/

/* Some additional Function.prototype properties expected by tests below. */
try {
    Object.defineProperties(Function.prototype, {
        testWritable: { value: 123, writable: true, enumerable: false, configurable: false },
        testNonWritable: { value: 234, writable: false, enumerable: false, configurable: false }
    });
} catch (e) {
    print(e.stack);
}

/* Shared object value which coerces to 'length' with a side effect.
 * Used to verify string coercion and evaluation order.
 */
var objLengthKey = { toString: function () { print('toString coerced object (return "length")'); return "length"; } };

function getLightFunc() {
    /* When using DUK_OPT_LIGHTFUNC_BUILTINS, all built-ins except just a
     * handful are lightfuncs.
     *
     * Use math.max as a test function: it has a non-zero magic, it has
     * length set to 2, but is still a varargs function.
     */
    return Math.max;
}

function getNormalFunc() {
    /* Even with DUK_OPT_LIGHTFUNC_BUILTINS, the top level constructors
     * are not converted to lightfuncs: they have additional properties
     * like Number.POSITIVE_INFINITY which would be lost.
     */
    return Number;
}

function sanitizeFuncToString(x) {
    /* Escape Ecmascript comments which appear e.g. when coercing function
     * values to string.  Hide lightfunc pointer which is variable data.
     */
    x = String(x);
    x = x.replace('/*', '(*').replace('*/', '*)');
    x = x.replace(/light_[0-9a-fA-F]+_/, 'light_PTR_', x);
    return x;
}

function isLightFunc(x) {
    return Duktape.info(x)[0] == 9;  // tag
}

/*===
light func support test
info.length: 1
typeof: function
[9]
===*/

function lightFuncSupportTest() {
    /* Duktape.info() can be used to detect that light functions are
     * supported.
     */

    var fun = getLightFunc();
    var info = Duktape.info(fun);

    /* Value is primitive (no prop allocations etc) but still callable. */
    print('info.length:', info.length);
    print('typeof:', typeof fun);
    print(Duktape.enc('jx', Duktape.info(fun)));
}

/*===
typeof test
function
===*/

function typeofTest() {
    /* typeof of lightweight and ordinary functions should look the same
     * for transparency, so: 'function'.
     */

    var fun = getLightFunc();

    print(typeof fun);
}

/*===
comparison test
===*/

function comparisonTest() {
    var lf1 = Math.cos;
    var lf2 = Math.sin;
    var lf3 = Array.prototype.forEach;
    var nf1 = Object(lf1);
    var nf2 = Object(lf2);
    var nf3 = Object(lf3);
    var ef1 = function ecmafunc() {};
    var vals = [ lf1, lf2, lf3, nf1, nf2, nf3, ef1 ];
    var i, j;

    /*
     *  Comparison:
     *
     *    - Lightfunc-to-lightfunc comparison compares both Duktape/C
     *      function pointers and the flags value (magic + other flags).
     *      Because the same Duktape/C function is often shared for very
     *      different functions, it's important to include at least magic
     *      in the comparison.
     *
     */

    // FIXME: coerced lightfunc vs. plain lightfunc?  Ordinary functions
    // only compare equal only if the reference is exactly the same, so
    // perhaps the desired result for lightFunc == Object(lightFunc) is false?

    for (i = 0; i < vals.length; i++) {
        for (j = 0; j < vals.length; j++) {
            print(i, j, vals[i] == vals[j], vals[i] === vals[j]);
        }
    }
}

/*===
toString() test
function lightfunc() {(* lightfunc *)}
function lightfunc() {(* lightfunc *)}
function lightfunc() {(* lightfunc *)}
true
true
===*/

function toStringTest() {
    /* String coercion of functions is not strictly defined - so here the
     * coercion output can identify the function as a lightweight function.
     *
     * Because the string coercion output includes Ecmascript comment chars,
     * we need some escaping here.
     */

    var fun = getLightFunc();

    print(sanitizeFuncToString(String(fun)));
    print(sanitizeFuncToString(fun.toString()));
    print(sanitizeFuncToString(Function.prototype.toString.call(fun)));

    /* ToString(fun) and Function.prototype.toString(fun) should match for
     * lightfuncs.
     */
    print(String(fun) === fun.toString());
    print(String(fun) === Function.prototype.toString.call(fun));
}

/*===
toObject() test
no caching: false
length: 2 2
name: lightfunc lightfunc
typeof: function function
internal prototype is Function.prototype: true true
external prototype is not set: true
internal prototypes match: true
external prototypes match (do not exist): true
Math.max test: 9 9
length: 1 1
===*/

function toObjectTest() {
    /* Object coercion creates a normal Duktape/C function object.  A few
     * things to watch out for:
     *
     *   - varargs vs. fixed args
     *
     * The fact that the virtual lightfunc name is copied over to the
     * non-light function is somewhat misleading, but still maybe the
     * best option?
     */

    var lightFunc = Math.max;  // Math.max has 'length' 2, but is varargs
    var normalFunc = Object(lightFunc);

    // Object coercion always results in a new object, there is no "caching"
    print('no caching:', Object(lightFunc) === Object(lightFunc));

    print('length:', lightFunc.length, normalFunc.length);
    print('name:', lightFunc.name, normalFunc.name);
    print('typeof:', typeof lightFunc, typeof normalFunc);
    print('internal prototype is Function.prototype:',
          Object.getPrototypeOf(lightFunc) === Function.prototype,
          Object.getPrototypeOf(normalFunc) === Function.prototype);
    print('external prototype is not set:',
          lightFunc.prototype === undefined);
    print('internal prototypes match:',
          Object.getPrototypeOf(lightFunc) === Object.getPrototypeOf(normalFunc));
    print('external prototypes match (do not exist):',
          lightFunc.prototype === normalFunc.prototype);

    // Although a lightfunc is not extensible, the coercion result is to
    // match behavior for e.g. strings:
    //
    //     > Object.isExtensible(Object('foo'))
    //     true
    print('isExtensible:', Object.isExtensible(lightFunc), Object.isExtensible(normalFunc));

    // Here the max value (9) should be further than what the apparent
    // 'length' is
    print('Math.max test:', lightFunc(1, 5, 7, 4, 9, 2), normalFunc(1, 5, 7, 4, 9, 2));

    // Math.cos has 'length' 1 and is not varargs
    lightFunc = Math.cos;
    normalFunc = Object(lightFunc);
    print('length:', lightFunc.length, normalFunc.length);

    // FIXME: other properties
}

/*===
call and apply test
call
321
apply
987
===*/

function callApplyTest() {
    /* Lightfuncs inherit from Function.prototype (similarly to how plain
     * strings inherit from String.prototype).  This means you can use
     * call() and apply().
     */

    var fun = Math.max;  // length 2, varargs

    print('call');
    print(fun.call('myThis', 123, 321));

    print('apply');
    print(fun.apply('myThis', [ 123, 321, 987, 345 ]));
}

/*===
inherit from Function.prototype test
testValue
===*/

function inheritFromFunctionPrototypeTest() {
    var fun = getLightFunc();

    Function.prototype.inheritTestProperty = 'testValue';
    print(fun.inheritTestProperty);
}

/*===
Object.prototype.toString() test
[object Function]
===*/

function objectPrototypeToStringTest() {
    /* Object.prototype.toString() for a light function should look same as
     * for actual functions to make them as transparent as possible, so:
     * "[object Function]".
     */

    var fun = getLightFunc();

    print(Object.prototype.toString.call(fun));
}

/*===
JSON/JX/JC test
json
undefined
undefined
jx
{_func:true}
{_func:true}
jc
{"_func":true}
{"_func":true}
json
undefined
undefined
jx
{_func:true}
{_func:true}
jc
{"_func":true}
{"_func":true}
json
{"array":[1,null,2,null,3]}
{
    "array": [
        1,
        null,
        2,
        null,
        3
    ]
}
jx
{lf:{_func:true},nf:{_func:true},array:[1,{_func:true},2,{_func:true},3]}
{
    lf: {_func:true},
    nf: {_func:true},
    array: [
        1,
        {_func:true},
        2,
        {_func:true},
        3
    ]
}
jc
{"lf":{"_func":true},"nf":{"_func":true},"array":[1,{"_func":true},2,{"_func":true},3]}
{
    "lf": {"_func":true},
    "nf": {"_func":true},
    "array": [
        1,
        {"_func":true},
        2,
        {"_func":true},
        3
    ]
}
===*/

function jsonJxJcTest() {
    /* There's a separate test for this too, but lightweight functions look
     * like functions in JSON/JX/JC output.
     */

    // FIXME: should JX/JC distinguish?

    var lightFunc = getLightFunc();
    var normalFunc = getNormalFunc();

    var testValue1 = lightFunc;
    var testValue2 = normalFunc;
    var testValue3 = {
        lf: lightFunc,
        nf: normalFunc,
        array: [ 1, lightFunc, 2, normalFunc, 3 ]
    };

    [ testValue1, testValue2, testValue3 ].forEach(function (v) {
        print('json');
        print(JSON.stringify(v));
        print(JSON.stringify(v, null, 4));
        print('jx');
        print(Duktape.enc('jx', v));
        print(Duktape.enc('jx', v, null, 4));
        print('jc');
        print(Duktape.enc('jc', v));
        print(Duktape.enc('jc', v, null, 4));
    });
}

/*===
bound function test
F: function lightfunc() {(* lightfunc *)}
F type tag: 9
G: function lightfunc() {(* bound *)}
G type tag: 6
G.length: 1
H: function light_0805b822_002f() {(* bound *)}
H type tag: 6
H.length: 0
I: function light_0805b822_002f() {(* bound *)}
I type tag: 6
I.length: 0
G(123): 234
G(123,987): 987
===*/

function boundFunctionTest() {
    /* A lightweight function can be bound normally.  The 'length' property
     * must be correctly copied from the virtual 'length' of the lightfunc.
     */

    var F = Math.max;  // length 2, varargs
    print('F:', sanitizeFuncToString(String(F)));
    print('F type tag:', Duktape.info(F)[0]);

    var G = F.bind('myThis', 234);  // bound once
    print('G:', sanitizeFuncToString(String(G)));
    print('G type tag:', Duktape.info(G)[0]);
    print('G.length:', G.length);  // length 1, one argument was bound

    var H = G.bind('foo', 345);  // bound twice
    print('H:', sanitizeFuncToString(String(H)));
    print('H type tag:', Duktape.info(H)[0]);
    print('H.length:', H.length);  // length 0, two arguments are bound

    var I = H.bind('foo', 345);  // bound three times
    print('I:', sanitizeFuncToString(String(I)));
    print('I type tag:', Duktape.info(I)[0]);
    print('I.length:', I.length);  // length 0, doesn't become negative

    // Another simple test
    print('G(123):', G(123));
    print('G(123,987):', G(123, 987));
}

/*===
property get test
length directly: 2
toString coerced object (return "length")
objLengthKey coerced to string: length
toString coerced object (return "length")
length through object coercion: 2
read from length -> 2
read from prototype -> undefined
read from name -> lightfunc
toString coerced object (return "length")
toString coerced object (return "length")
read from length -> 2
read from testWritable -> 123
read from testNonWritable -> 234
read from call -> function lightfunc() {(* lightfunc *)}
read from apply -> function lightfunc() {(* lightfunc *)}
read from nonexistent -> undefined
===*/

function propertyGetTest() {
    var lightFunc = getLightFunc();

    /*
     *  Property get is relatively simple.  Virtual properties are matched
     *  first, and then we continue with Function.prototype for lookup.
     *
     *  An inherited getter is a special case.
     *
     *  NOTE: duk_hobject_props.c must string coerce the key before comparison
     *  because an object may coerce to a virtual key name
     */

    print('length directly:', lightFunc.length);
    print('objLengthKey coerced to string:', String(objLengthKey));
    print('length through object coercion:', lightFunc[objLengthKey]);

    var testKeys = [
      'length', 'prototype', 'name',  // own properties
      objLengthKey,                   // own, object coerces to 'length'
      'testWritable',                 // inherited, writable
      'testNonWritable',              // inherited, non-writable
      'call', 'apply',                // inherited, standard built-in
      'nonexistent'                   // non-existent
    ];

    testKeys.forEach(function (k) {
        try {
            print('read from', k, '->', sanitizeFuncToString(lightFunc[k]));
        } catch (e) {
            print('read from', k, '->', e.name);
        }
    });

    // FIXME: property getter test
}

/*===
property put test
write to length -> silent error
write to prototype -> silent error
write to name -> silent error
toString coerced object (return "length")
toString coerced object (return "length")
write to length -> silent error
write to testWritable -> silent error
write to testNonWritable -> silent error
write to call -> silent error
write to apply -> silent error
write to nonexistent -> silent error
write to length -> TypeError
write to prototype -> TypeError
write to name -> TypeError
toString coerced object (return "length")
toString coerced object (return "length")
write to length -> TypeError
write to testWritable -> TypeError
write to testNonWritable -> TypeError
write to call -> TypeError
write to apply -> TypeError
write to nonexistent -> TypeError
===*/

function propertyPutTest() {
    var lightFunc = getLightFunc();

    /*
     *  The own properties of a lightfunc are not writable.  It is also not
     *  extensible (having no place to write properties), so property writes
     *  fail with TypeError in almost every case:
     *
     *    - Own (virtual) property: not writable
     *    - Inherited property, non-writable: not writable
     *    - Inherited property, writable: not extensible
     *    - Non-existent: not extensible
     *    - Inherited property, setter: *setter must be called*
     *
     *  The setter case is handled by a separate test function.
     */

    var testKeys = [
      'length', 'prototype', 'name',  // own properties
      objLengthKey,                   // own, object coerces to 'length'
      'testWritable',                 // inherited, writable
      'testNonWritable',              // inherited, non-writable
      'call', 'apply',                // inherited, standard built-in
      'nonexistent'                   // non-existent
    ];

    testKeys.forEach(function (k) {
        // non-strict, errors are ignored
        try {
            lightFunc[k] = 'test';
            print('write to', k, '->', 'silent error');
        } catch (e) {
            print('write to', k, '->', e.name);
        }
    });

    testKeys.forEach(function (k) {
        'use strict';  // must be strict to cause throwing
        try {
            lightFunc[k] = 'test';
            print('never here:', k);
        } catch (e) {
            print('write to', k, '->', e.name);
        }
    });
}

/*===
property has test
existence: length -> true
existence: prototype -> false
existence: name -> true
toString coerced object (return "length")
toString coerced object (return "length")
existence: length -> true
existence: testWritable -> true
existence: testNonWritable -> true
existence: call -> true
existence: apply -> true
existence: nonexistent -> false
===*/

function propertyHasTest() {
    var lightFunc = getLightFunc();

    /*
     *  Property existence test must account for own (virtual) properties
     *  and inherited properties.
     */

    var testKeys = [
      'length', 'prototype', 'name', // own
      objLengthKey,                  // own, object coerces to 'length'
      'testWritable',                // inherited
      'testNonWritable',             // inherited
      'call', 'apply',               // inherited
      'nonexistent'                  // non-existent
    ];

    testKeys.forEach(function (k) {
        print('existence:', k, '->', k in lightFunc);
    });
}

/*===
property delete test
delete: length -> false
delete: prototype -> true
delete: name -> false
toString coerced object (return "length")
toString coerced object (return "length")
delete: length -> false
delete: testWritable -> true
delete: testNonWritable -> true
delete: call -> true
delete: apply -> true
delete: nonexistent -> true
delete: length -> TypeError
delete: prototype -> true
delete: name -> TypeError
toString coerced object (return "length")
toString coerced object (return "length")
delete: length -> TypeError
delete: testWritable -> true
delete: testNonWritable -> true
delete: call -> true
delete: apply -> true
delete: nonexistent -> true
non-strict: true
strict: true
===*/

function propertyDeleteTest() {
    var lightFunc = getLightFunc();

    /*
     *  Property deletions only affect own properties.  Since all lightfunc
     *  virtual properties are non-configurable:
     *
     *    - Own (virtual) property: not configurable
     *    - Non-existent property: silent success
     */

    // existence for own properties and a few inherited
    var testKeys = [
      'length', 'prototype', 'name', // own
      objLengthKey,                  // own, object coerces to 'length'
      'testWritable',                // inherited
      'testNonWritable',             // inherited
      'call', 'apply',               // inherited
      'nonexistent'                  // non-existent
    ];

    testKeys.forEach(function (k) {
        // non-strict, errors return false but don't throw
        try {
            print('delete:', k, '->', delete lightFunc[k]);
        } catch (e) {
            print('delete:', k, '->', e.name);
        }
    });

    testKeys.forEach(function (k) {
        'use strict';  // promote to TypeError
        try {
            print('delete:', k, '->', delete lightFunc[k]);
        } catch (e) {
            print('delete:', k, '->', e.name);
        }
    });

    // Deletion of a non-existent property is a silent success in both
    // strict and non-strict mode.  Below this is demonstrated also for
    // normal functions.

    var func = function () {};
    try {
        (function () { print('non-strict:', delete func.nonexistent); })();
    } catch (e) {
        print(e);
    }
    try {
        (function () { 'use strict'; print('strict:', delete func.nonexistent); })();
    } catch (e) {
        print(e);
    }
}

/*===
property accessor this binding test
getter, strict
strict getter "this" binding test
typeof this: function
this == lightFunc: false
this === lightFunc: true
this.name: lightfunc
type tag: 9
getter retval
setter, strict
strict setter "this" binding test
typeof this: function
this == lightFunc: false
this === lightFunc: true
this.name: lightfunc
type tag: 9
getter, non-strict
non-strict getter "this" binding test
typeof this: function
this == lightFunc: false
this === lightFunc: false
this.name: lightfunc
type tag: 6
getter retval
setter, non-strict
non-strict setter "this" binding test
typeof this: function
this == lightFunc: false
this === lightFunc: false
this.name: lightfunc
type tag: 6
===*/

function propertyAccessorThisBindingTest() {
    var lightFunc = getLightFunc();

    /*
     *  If a getter/setter is triggered by reading/writing an inherited
     *  property, the getter/setter 'this' binding is set to the lightfunc
     *  (and not, e.g., Function.prototype).
     *
     *  However, if the setter/getter is non-strict, the this binding is
     *  then object coerced.  This is similar to what happens to a string:
     *
     *      > Object.defineProperty(String.prototype, 'foo', { get: function () { print(typeof this); } });
     *      {}
     *      > Object.defineProperty(String.prototype, 'bar', { get: function () { 'use strict'; print(typeof this); } });
     *      {}
     *      > "foo".foo
     *      object
     *      undefined
     *      > "foo".bar
     *      string
     *      undefined
     */

    Object.defineProperty(Function.prototype, 'testAccessorStrict', {
        get: function () {
            'use strict';
            print('strict getter "this" binding test');
            print('typeof this:', typeof this);
            print('this == lightFunc:', this == lightFunc);
            print('this === lightFunc:', this === lightFunc);
            print('this.name:', sanitizeFuncToString(this.name));
            print('type tag:', Duktape.info(this)[0]);
            return 'getter retval';
        },
        set: function () {
            'use strict';
            print('strict setter "this" binding test');
            print('typeof this:', typeof this);
            print('this == lightFunc:', this == lightFunc);
            print('this === lightFunc:', this === lightFunc);
            print('this.name:', sanitizeFuncToString(this.name));
            print('type tag:', Duktape.info(this)[0]);
        },
        enumerable: false,
        configurable: true
    });

    Object.defineProperty(Function.prototype, 'testAccessorNonStrict', {
        get: function () {
            print('non-strict getter "this" binding test');
            print('typeof this:', typeof this);
            print('this == lightFunc:', this == lightFunc);
            print('this === lightFunc:', this === lightFunc);
            print('this.name:', sanitizeFuncToString(this.name));
            print('type tag:', Duktape.info(this)[0]);
            return 'getter retval';
        },
        set: function () {
            print('non-strict setter "this" binding test');
            print('typeof this:', typeof this);
            print('this == lightFunc:', this == lightFunc);
            print('this === lightFunc:', this === lightFunc);
            print('this.name:', sanitizeFuncToString(this.name));
            print('type tag:', Duktape.info(this)[0]);
        },
        enumerable: false,
        configurable: true
    });

    print('getter, strict');
    print(lightFunc.testAccessorStrict);
    print('setter, strict');
    lightFunc.testAccessorStrict = 123;

    print('getter, non-strict');
    print(lightFunc.testAccessorNonStrict);
    print('setter, non-strict');
    lightFunc.testAccessorNonStrict = 123;
}

/*===
property misc test
===*/

function propertyMiscTest() {
    var lightFunc = getLightFunc();
    var t,  k;

    /*
     *  A lightfunc is considered sealed, frozen, and non-extensible.
     */

    print('isSealed:', Object.isSealed(lightFunc));
    print('isFrozen:', Object.isFrozen(lightFunc));
    print('isExtensible:', Object.isExtensible(lightFunc));

    /*
     *  Enumeration: enumerable keys (both own and inherited).  Because all
     *  own properties are non-enumerable, only enumerable inherited keys
     *  print out here.
     */

    for (k in lightFunc) {
        print('for-in:', JSON.stringify(k));
    }

    /*
     *  Object.keys(): only returns own non-enumerable keys, so -nothing-
     *  prints out here.
     */

    Object.keys(lightFunc).forEach(function (k) {
        print('Object.keys:', JSON.stringify(k));
    });

    /*
     *  Own property names: own property names (virtual properties) print out.
     */

    Object.getOwnPropertyNames(lightFunc).forEach(function (k) {
        print('Object.getOwnPropertyNames:', JSON.stringify(k));
    });

    /*
     *  The virtual 'name' of a lightfunc has the form:
     *
     *      light_<ptr>_<flags>
     *
     *  The flags field includes the magic value and other flags.  The magic
     *  value is an important part of a function identity because it may have
     *  a large impact on the Duktape/C function behavior.
     */

    t = /^light_[0-9a-fA-F]+_[0-9a-fA-F]{4}$/.exec(lightFunc.name);
    print('lightFunc.name matches regexp:', (t !== null));

    t = lightFunc.name.substring(0, 5) + '_' +
        lightFunc.name.substring(6).replace(/[0-9a-fA-F]+/g, 'XXX');
    print('censored lightFunc.name:', t);

    // .prototype maps to Function.prototype
/*
    print('call' in lightFunc);  // inherited
    print('prototype' in lightFunc);  // own property
    print(lightFunc.prototype === Function.prototype);
*/
}

/*===
traceback test
===*/

function tracebackTest() {
    var err;

    try {
        decodeURIComponent('%x');
    } catch (e) {
        err = e;
    }

    // FIXME: validate
    print(err.stack);
}

/*===
Duktape.act() test
===*/

function duktapeActTest() {
    // FIXME: lightfunc in Duktape.act(); how to test, need intermediate function
    // to call Ecmascript function, perhaps ./duk --test

    // FIXME: use forEach for now

    [ 'foo' ].forEach(function callback(x) {
        var e = new Error('for traceback');
        var i;
        var a;

        print(e.stack);

        for (i = -1; ; i--) {
            a = Duktape.act(i);
            if (!a) { break; }
            print(i, Duktape.enc('jx', Object.getOwnPropertyNames(a)), sanitizeFuncToString(a.function.name));
        }
    });
}

function exemptBuiltinsTest() {

    function f(v) {
        return (typeof v) + ' ' + isLightFunc(v);
    }

    // this is converted to a lightfunc; print for ensuring isLightFunc() works
    print('Math.max (is lightfunc):', f(Math.max));

    /*
     *  These specific built-ins properties cannot be converted to lightfuncs
     *  because of internal asserts or because a user may write some property
     *  of the function (e.g. "require.id").
     */

    print('eval:', f(eval));
    print('yield:', f(Duktape.Thread.yield));
    print('resume:', f(Duktape.Thread.resume));
    print('require:', f(require));

    /*
     *  These top-level constructor functions are never converted to lightfuncs
     *  because they have properties that cannot be virtualized.
     */

    print('Object:', f(Object));
    print('Function:', f(Function));
    print('Array:', f(Array));
    print('String:', f(String));
    print('Boolean:', f(Boolean));
    print('Number:', f(Number));
    print('Date:', f(Date));
    print('RegExp:', f(RegExp));
    print('Error:', f(Error));
    print('EvalError:', f(EvalError));
    print('RangeError:', f(RangeError));
    print('ReferenceError:', f(ReferenceError));
    print('SyntaxError:', f(SyntaxError));
    print('TypeError:', f(TypeError));
    print('URIError:', f(URIError));
    print('Proxy:', f(Proxy));

    print('Duktape.Buffer:', f(Duktape.Buffer));
    print('Duktape.Pointer:', f(Duktape.Pointer));
    print('Duktape.Thread:', f(Duktape.Thread));
    print('Duktape.Logger:', f(Duktape.Logger));

    /*
     *  These globals are not functions at all.
     */

    print('Duktape:', f(Duktape));
    print('Math:', f(Math));
    print('JSON:', f(JSON));
}

/*
- Duktape.info() documentation for tags
- Rename lightfunc/LIGHTFUNC to FUNCTION? Cf. POINTER and BUFFER are the
  plain primitive type names, while Pointer and Buffer are Objects
- Portable funcptr printing -- duk_push_funcptr_string(ctx, ptr, sizeof)?
- Some way of reading a function's magic from Ecmascript code?
- Magic rename for release (keep magic API funcs as macros)?
- Bound function object pointing to lightweight func

- interaction with 'caller' property
- list all function properties and add testcase printing for them

- check virtual name of lightfuncs
- if built-ins get special handling for lightfunc name, test for that here

- a in b
- a instanceof b

- equality, plain and strict
*/

try {
    print('light func support test');
    lightFuncSupportTest();
} catch (e) {
    print(e.stack);
}
try {
    print('typeof test');
    typeofTest();
} catch (e) {
    print(e.stack);
}
try {
    print('comparison test');
    comparisonTest();
} catch (e) {
    print(e.stack);
}
try {
    print('toString() test');
    toStringTest();
} catch (e) {
    print(e.stack);
}
try {
    print('toObject() test');
    toObjectTest();
} catch (e) {
    print(e.stack);
}
try {
    print('call and apply test');
    callApplyTest();
} catch (e) {
    print(e.stack);
}
try {
    print('inherit from Function.prototype test');
    inheritFromFunctionPrototypeTest();
} catch (e) {
    print(e.stack);
}
try {
    print('Object.prototype.toString() test');
    objectPrototypeToStringTest();
} catch (e) {
    print(e.stack);
}
try {
    print('JSON/JX/JC test');
    jsonJxJcTest();
} catch (e) {
    print(e.stack);
}
try {
    print('bound function test');
    boundFunctionTest();
} catch (e) {
    print(e.stack);
}
try {
    print('property get test');
    propertyGetTest();
} catch (e) {
    print(e.stack);
}
try {
    print('property put test');
    propertyPutTest();
} catch (e) {
    print(e.stack);
}
try {
    print('property has test');
    propertyHasTest();
} catch (e) {
    print(e.stack);
}
try {
    print('property delete test');
    propertyDeleteTest();
} catch (e) {
    print(e.stack);
}
try {
    print('property accessor this binding test');
    propertyAccessorThisBindingTest();
} catch (e) {
    print(e.stack);
}
try {
    print('property misc test');
    propertyMiscTest();
} catch (e) {
    print(e.stack);
}
try {
    print('traceback test');
    tracebackTest();
} catch (e) {
    print(e.stack);
}
try {
    print('Duktape.act() test');
    duktapeActTest();
} catch (e) {
    print(e.stack);
}
try {
    print('exempt built-ins test');
    exemptBuiltinsTest();
} catch (e) {
    print(e.stack);
}
