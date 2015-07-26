/*
 *  Behavior of lightweight functions in various situations.
 *
 *  Also documents the detailed behavior and limitations of lightfuncs.
 *  Specific limitations that matter have their own testcases (e.g.
 *  setter/getter limitations).
 *
 *  Exhaustive built-in method tests don't all make practical sense (e.g.
 *  using a lightfunc as a 'this' binding for Array methods or giving a
 *  lightfunc as an argument to new Date()), but they document what
 *  currently happens and ensure there are no assertion failures or such.
 *  These checks are not 100%, e.g. if 'this' and arguments may be lightfuncs,
 *  giving two lightfuncs may cause a TypeError for 'this' validation and
 *  never ensure that the other argument is handled correctly.
 */

/*---
{
    "custom": true
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

function sanitizeLfunc(x) {
    /* Escape Ecmascript comments which appear e.g. when coercing function
     * values to string.  Hide lightfunc pointer which is variable data.
     */
    x = String(x);
    x = x.replace(/\/\*/g, '(*').replace(/\*\//g, '*)');
    x = x.replace(/light_[0-9a-fA-F]+_/g, 'light_PTR_', x);
    x = x.replace(/LIGHT_[0-9a-fA-F]+_/g, 'LIGHT_PTR_', x);
    return x;
}

function isLightFunc(x) {
    return Duktape.info(x)[0] == 9;  // tag
}

function printTypedJx(v, name) {
    print(name + ':', typeof v, Duktape.enc('jx', v));
}

function testTypedJx(func, name) {
    var res;
    name = name || func.name || '???';
    try {
        res = func();
        print(name + ':', typeof res, Duktape.enc('jx', res));
    } catch (e) {
        print(name + ':', e.name);
        //print('*** ' + e);
    }
}

function sanitizeTraceback(x) {
    x = x.replace(/\/tmp\/.*?:/g, 'TESTCASE:');
    x = x.replace(/:\d+/g, ':NNN')
    x = x.replace(/\/\*/g, '(*').replace(/\*\//g, '*)');
    x = x.replace(/light_[0-9a-fA-F]+_/g, 'light_PTR_', x);
    x = x.replace(/LIGHT_[0-9a-fA-F]+_/g, 'LIGHT_PTR_', x);
    return x;
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

try {
    print('light func support test');
    lightFuncSupportTest();
} catch (e) {
    print(e.stack || e);
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

try {
    print('typeof test');
    typeofTest();
} catch (e) {
    print(e.stack || e);
}

/*===
property assignment test
strPlainNonStrict: success
strPlainStrict: TypeError
strObjectNonStrict: success
strObjectStrict: success
strObjectNonextNonStrict: success
strObjectNonextStrict: TypeError
lfuncNonStrict1: success
lfuncNonStrict2: success
lfuncStrict1: TypeError
lfuncStrict2: TypeError
===*/

function propertyAssignmentTest() {
    /*
     *  Trying to assign a property value for a plain string value is a
     *  silent error in non-strict mode and TypeError in strict mode.
     */

    var strPlainNonStrict = function () {
        var str = 'foo';
        str.foo = 'bar';
    };
    var strPlainStrict = function () {
        'use strict';
        var str = 'foo';
        str.foo = 'bar';
    };
    try {
        strPlainNonStrict();
        print('strPlainNonStrict:', 'success');
    } catch (e) {
        print('strPlainNonStrict:', e.name);
    }

    try {
        strPlainStrict();
        print('strPlainStrict:', 'success');
    } catch (e) {
        print('strPlainStrict:', e.name);
    }

    /*
     *  Plain strings are not a good analogy for lightfunc behavior because
     *  lightfuncs try to behave like full objects.  A closer analogy would
     *  be how a String object behaves.
     *
     *  Here the property assignment succeeds because new String() returns
     *  an extensible object.
     */

    var strObjectNonStrict = function () {
        var str = new String('foo');
        str.foo = 'bar';
    };
    var strObjectStrict = function () {
        'use strict';
        var str = new String('foo');
        str.foo = 'bar';
    };
    try {
        strObjectNonStrict();
        print('strObjectNonStrict:', 'success');
    } catch (e) {
        print('strObjectNonStrict:', e.name);
    }

    try {
        strObjectStrict();
        print('strObjectStrict:', 'success');
    } catch (e) {
        print('strObjectStrict:', e.name);
    }

    /*
     *  Even closer analogy would be a new String() object which was made
     *  non-extensible.
     */

    var strObjectNonextNonStrict = function () {
        var str = new String('foo');
        Object.preventExtensions(str);
        str.foo = 'bar';
    };
    var strObjectNonextStrict = function () {
        'use strict';
        var str = new String('foo');
        Object.preventExtensions(str);
        str.foo = 'bar';
    };
    try {
        strObjectNonextNonStrict();
        print('strObjectNonextNonStrict:', 'success');
    } catch (e) {
        print('strObjectNonextNonStrict:', e.name);
    }

    try {
        strObjectNonextStrict();
        print('strObjectNonextStrict:', 'success');
    } catch (e) {
        print('strObjectNonextStrict:', e.name);
    }

    /*
     *  Because a lightfunc is not extensible and all own properties are
     *  non-configurable, the best behavior is silent success in non-strict
     *  mode (same happens e.g. when writing over 'length' of a String)
     *  and a TypeError in strict mode.
     *
     *  This is now the behavior but error messages are not ideal.  When
     *  the virtual property exists, the error is "not writable"; when it
     *  doesn't exist, the error is "invalid base value" ("not extensible"
     *  would be better).
     *
     *  Another quite justifiable behavior would be to pretend as if the
     *  lightfunc was coerced to a full Function object before assignment.
     *  The coerced Function would now be extensible, so it would be possible
     *  to write new properties with no error.  The current assignment behavior
     *  is not fully consistent with Object.defineProperty() behavior.
     */

    var lfuncNonStrict1 = function () {
        var lfunc = Math.max;
        lfunc.name = 'foo';
    }
    var lfuncNonStrict2 = function () {
        var lfunc = Math.max;
        lfunc.nonexistent = 123;
    }
    var lfuncStrict1 = function () {
        'use strict';
        var lfunc = Math.max;
        lfunc.name = 'foo';
    }
    var lfuncStrict2 = function () {
        'use strict';
        var lfunc = Math.max;
        lfunc.nonexistent = 123;
    }

    try {
        lfuncNonStrict1();
        print('lfuncNonStrict1:', 'success');
    } catch (e) {
        print('lfuncNonStrict1:', e.name);
    }
    try {
        lfuncNonStrict2();
        print('lfuncNonStrict2:', 'success');
    } catch (e) {
        print('lfuncNonStrict2:', e.name);
    }
    try {
        lfuncStrict1();
        print('lfuncStrict1:', 'success');
    } catch (e) {
        print('lfuncStrict1:', e.name);
        //print(e.stack);
    }
    try {
        lfuncStrict2();
        print('lfuncStrict2:', 'success');
    } catch (e) {
        print('lfuncStrict2:', e.name);
        //print(e.stack);
    }
}

try {
    print('property assignment test');
    propertyAssignmentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
instanceof test
{} instanceof lightfunc: TypeError
false
{} instanceof func-wo-prototype: TypeError
lightFunc instanceof Function: true
lightFunc instanceof Number: false
lightFunc instanceof Object: true
===*/

function instanceofTest() {
    var obj = {};
    var lightFunc = getLightFunc();
    var func;

    /*
     *  When a lightweight function is on the right hand side, step 3
     *  of the algorithm in E5.1 Section 15.3.5.3 step will always
     *  throw a TypeError: lightweight functions don't have a virtual
     *  'prototype' property.
     */

    try {
        print('{} instanceof lightfunc:', obj instanceof lightFunc);
    } catch (e) {
        print('{} instanceof lightfunc:', e.name);
    }

    /*
     *  The same happens for an ordinary function which doesn't have a
     *  'prototype' property.  To demonstrate this we need a function
     *  without a 'prototype' property: normal Ecmascript functions
     *  always have the property and it's not configurable so we can't
     *  delete it.  Luckily many non-constructor built-ins don't have
     *  the property so we can use one of them.
     */

    func = Math.cos;
    print('prototype' in func);
    try {
        print('{} instanceof func-wo-prototype:', obj instanceof func);
    } catch (e) {
        print('{} instanceof func-wo-prototype:', e.name);
    }

    /*
     *  When a lightfunc appears on the left-hand side, the prototype
     *  walk begins with Function.prototype.  Note that 'instanceof'
     *  never compares the original lhs value, it begins its walk from
     *  lhs's internal prototype.
     */

    try {
        print('lightFunc instanceof Function:', lightFunc instanceof Function);
    } catch (e) {
        print('lightFunc instanceof Function:', e.name);
    }

    try {
        print('lightFunc instanceof Number:', lightFunc instanceof Number);
    } catch (e) {
        print('lightFunc instanceof Number:', e.name);
    }

    try {
        print('lightFunc instanceof Object:', lightFunc instanceof Object);
    } catch (e) {
        print('lightFunc instanceof Object:', e.name);
    }
}

try {
    print('instanceof test');
    instanceofTest();
} catch (e) {
    print(e.stack || e);
}

/*===
comparison test
0 0 true true
0 1 false false
0 2 false false
0 3 false false
0 4 false false
0 5 false false
0 6 false false
1 0 false false
1 1 true true
1 2 false false
1 3 false false
1 4 false false
1 5 false false
1 6 false false
2 0 false false
2 1 false false
2 2 true true
2 3 false false
2 4 false false
2 5 false false
2 6 false false
3 0 false false
3 1 false false
3 2 false false
3 3 true true
3 4 false false
3 5 false false
3 6 false false
4 0 false false
4 1 false false
4 2 false false
4 3 false false
4 4 true true
4 5 false false
4 6 false false
5 0 false false
5 1 false false
5 2 false false
5 3 false false
5 4 false false
5 5 true true
5 6 false false
6 0 false false
6 1 false false
6 2 false false
6 3 false false
6 4 false false
6 5 false false
6 6 true true
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
     *    - Lightfunc never compares equal to an ordinary Function, even
     *      when the Function was created as ToObject(lightFunc).  This
     *      mimics the general behavior for Function objects, which always
     *      compare strictly by reference.
     */

    // XXX: samevalue

    for (i = 0; i < vals.length; i++) {
        for (j = 0; j < vals.length; j++) {
            print(i, j, vals[i] == vals[j], vals[i] === vals[j]);
        }
    }
}

try {
    print('comparison test');
    comparisonTest();
} catch (e) {
    print(e.stack || e);
}

/*===
arithmetic test
string: testfunction light_PTR_0511() {(* light *)}function light_PTR_0a11() {(* light *)}
string: function light_PTR_0511() {(* light *)}function light_PTR_0a11() {(* light *)}
string: function foo() {(* ecmascript *)}function bar() {(* ecmascript *)}
===*/

function arithmeticTest() {
    function p(x) {
        print(typeof x + ': ' + sanitizeLfunc(x));
    }

    p('test' + Math.cos + Math.sin);
    p(Math.cos + Math.sin);
    p((function foo(){}) + (function bar(){}));
}

try {
    print('arithmetic test');
    arithmeticTest();
} catch (e) {
    print(e.stack || e);
}

/*===
toString() test
function light_PTR_002f() {(* light *)}
function light_PTR_002f() {(* light *)}
function light_PTR_002f() {(* light *)}
true
true
===*/

function toStringTest() {
    /* String coercion of functions is not strictly defined - so here the
     * coercion output can identify the function as a lightweight function.
     *
     * Because the string coercion output includes Ecmascript comment chars
     * and a variable pointer, we need sanitization before printing.
     */

    var fun = getLightFunc();

    print(sanitizeLfunc(String(fun)));
    print(sanitizeLfunc(fun.toString()));
    print(sanitizeLfunc(Function.prototype.toString.call(fun)));

    /* ToString(fun) and Function.prototype.toString(fun) should match for
     * lightfuncs.
     */
    print(String(fun) === fun.toString());
    print(String(fun) === Function.prototype.toString.call(fun));
}

try {
    print('toString() test');
    toStringTest();
} catch (e) {
    print(e.stack || e);
}

/*===
toObject() test
caching: false
length: 2 2
name: light_PTR_002f light_PTR_002f
typeof: function function
internal prototype is Function.prototype: true true
external prototype is not set: true
internal prototypes match: true
external prototypes match (do not exist): true
isExtensible: false true
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
    print('caching:', Object(lightFunc) === Object(lightFunc));

    print('length:', lightFunc.length, normalFunc.length);
    print('name:', sanitizeLfunc(lightFunc.name), sanitizeLfunc(normalFunc.name));
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

    // XXX: other properties
}

try {
    print('toObject() test');
    toObjectTest();
} catch (e) {
    print(e.stack || e);
}

/*===
toBoolean() test
true
true
===*/

function toBooleanTest() {
    var lfunc = Math.cos;
    var nfunc = function foo() {};

    print(Boolean(lfunc));
    print(Boolean(nfunc));
}

try {
    print('toBoolean() test');
    toBooleanTest();
} catch (e) {
    print(e.stack || e);
}

/*===
toBuffer() test
buffer: function light_PTR_0511() {(* light *)}
buffer: function light_PTR_0a11() {(* light *)}
===*/

function toBufferTest() {
    /* Duktape.Buffer(v) does -not- implement the same semantics as the
     * ToBuffer() coercion provided by duk_to_buffer() API call.  The API
     * ToBuffer() is not directly available, but Duktape.enc('base64', ...)
     * will (currently) first call a duk_to_buffer() on the argument so we
     * can use that to get at ToBuffer().
     */

    function tobuf(x) {
        return Duktape.dec('base64', Duktape.enc('base64', x));
    }
    function printbuf(x) {
        var tmp = [];
        var i;
        tmp.push(typeof x + ': ');  // avoid printing length, depends on ptr length
        for (i = 0; i < x.length; i++) {
            if (x[i] >= 0x20 && x[i] <= 0x7e) {
                tmp.push(String.fromCharCode(x[i]));
            } else {
                tmp.push('<' + Number(x[i]).toString(16) + '>');
            }
        }
        print(sanitizeLfunc(tmp.join('')));
    }

    printbuf(tobuf(Math.cos));
    printbuf(tobuf(Math.sin));
}

try {
    print('toBuffer() test');
    toBufferTest();
} catch (e) {
    print(e.stack || e);
}

/*===
toPointer() test
pointer null
object null
===*/

function toPointerTest() {
    var lfunc = Math.cos;
    var t;

    t = Duktape.Pointer(lfunc);
    print(typeof t, t);

    t = new Duktape.Pointer(lfunc);
    print(typeof t, t);
}

try {
    print('toPointer() test');
    toPointerTest();
} catch (e) {
    print(e.stack || e);
}

/*===
number coercion test
NaN
NaN
0
0
0
0
0
0
===*/

function numberCoercionTest() {
    var lfunc = Math.cos;
    var nfunc = function foo(){};

    // ToNumber(): NaN
    print(Number(lfunc));
    print(Number(nfunc));

    // ToInteger(): positive zero
    // XXX: no test, where to get ToInteger() result most easily?

    // ToInt32(), ToUint32(), ToUint16(): positive zero
    print(lfunc >>> 0);  // ToUint32
    print(nfunc >>> 0);
    print(lfunc >> 0);   // ToInt32
    print(nfunc >> 0);
    print(String.fromCharCode(lfunc).charCodeAt(0));  // ToUint16
    print(String.fromCharCode(nfunc).charCodeAt(0));

}

try {
    print('number coercion test');
    numberCoercionTest();
} catch (e) {
    print(e.stack || e);
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

try {
    print('call and apply test');
    callApplyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
this coercion test
function true
function true
===*/

function thisCoercionTest() {
    var lfunc = Math.cos;

    // Strict functions get 'this' as is as normal, including lightfuncs.

    function myStrict() {
        'use strict';
        print(typeof this, isLightFunc(this));
    }
    myStrict.call(lfunc);

    // The 'this' binding of a non-strict function is not further coerced if
    // it is an object.  There are two logical behaviors here for lightfuncs:
    // either we (1) treat them like objects and don't coerce them; or (2)
    // coerce them forcibly to a fully fledged object.
    //
    // Current behavior is (1) so the 'this' binding should also be lightfunc
    // in myNonStrict.

    function myNonStrict() {
        print(typeof this, isLightFunc(this));
    }
    myNonStrict.call(lfunc);
}

try {
    print('this coercion test');
    thisCoercionTest();
} catch (e) {
    print(e.stack || e);
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

try {
    print('inherit from Function.prototype test');
    inheritFromFunctionPrototypeTest();
} catch (e) {
    print(e.stack || e);
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

try {
    print('Object.prototype.toString() test');
    objectPrototypeToStringTest();
} catch (e) {
    print(e.stack || e);
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
{"array":[100,null,200,null,300]}
{
    "array": [
        100,
        null,
        200,
        null,
        300
    ]
}
jx
{lf:{_func:true},nf:{_func:true},array:[100,{_func:true},200,{_func:true},300]}
{
    lf: {_func:true},
    nf: {_func:true},
    array: [
        100,
        {_func:true},
        200,
        {_func:true},
        300
    ]
}
jc
{"lf":{"_func":true},"nf":{"_func":true},"array":[100,{"_func":true},200,{"_func":true},300]}
{
    "lf": {"_func":true},
    "nf": {"_func":true},
    "array": [
        100,
        {"_func":true},
        200,
        {"_func":true},
        300
    ]
}
json
"toJsonRetval"
"toJsonRetval"
jx
"toJsonRetval"
"toJsonRetval"
jc
"toJsonRetval"
"toJsonRetval"
json
"toJsonRetval"
"toJsonRetval"
jx
"toJsonRetval"
"toJsonRetval"
jc
"toJsonRetval"
"toJsonRetval"
json
{"lf":"toJsonRetval","nf":"toJsonRetval","array":[100,"toJsonRetval",200,"toJsonRetval",300]}
{
    "lf": "toJsonRetval",
    "nf": "toJsonRetval",
    "array": [
        100,
        "toJsonRetval",
        200,
        "toJsonRetval",
        300
    ]
}
jx
{lf:"toJsonRetval",nf:"toJsonRetval",array:[100,"toJsonRetval",200,"toJsonRetval",300]}
{
    lf: "toJsonRetval",
    nf: "toJsonRetval",
    array: [
        100,
        "toJsonRetval",
        200,
        "toJsonRetval",
        300
    ]
}
jc
{"lf":"toJsonRetval","nf":"toJsonRetval","array":[100,"toJsonRetval",200,"toJsonRetval",300]}
{
    "lf": "toJsonRetval",
    "nf": "toJsonRetval",
    "array": [
        100,
        "toJsonRetval",
        200,
        "toJsonRetval",
        300
    ]
}
json
0
0
jx
0
0
jc
0
0
json
0
0
jx
0
0
jc
0
0
json
{"lf":null,"nf":null,"array":[100,1,200,3,300]}
{
    "lf": null,
    "nf": null,
    "array": [
        100,
        1,
        200,
        3,
        300
    ]
}
jx
{lf:NaN,nf:NaN,array:[100,1,200,3,300]}
{
    lf: NaN,
    nf: NaN,
    array: [
        100,
        1,
        200,
        3,
        300
    ]
}
jc
{"lf":{"_nan":true},"nf":{"_nan":true},"array":[100,1,200,3,300]}
{
    "lf": {"_nan":true},
    "nf": {"_nan":true},
    "array": [
        100,
        1,
        200,
        3,
        300
    ]
}
===*/

function jsonJxJcTest() {
    /* There's a separate test for this too, but lightweight functions look
     * like functions in JSON/JX/JC output.
     */

    var lightFunc = getLightFunc();
    var normalFunc = getNormalFunc();

    var testValue1 = lightFunc;
    var testValue2 = normalFunc;
    var testValue3 = {
        lf: lightFunc,
        nf: normalFunc,
        array: [ 100, lightFunc, 200, normalFunc, 300 ]
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

    /* toJSON() should work, and is inherited from Function.prototype.
     * XXX: right now the 'this' binding will be a lightfunc coerced
     * to a normal function, so 'toJsonRetval' is returned.
     */

    Function.prototype.toJSON = function (key) {
        //print('toJSON, this-is-lightfunc:', isLightFunc(this), 'key:', key);
        if (isLightFunc(this)) {
            return 'toJsonLightfuncRetval';
        }
        return 'toJsonRetval';
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

    delete Function.prototype.toJSON;

    /* The toJSON function itself can also be a lightfunc.  Below we use
     * Math.min() as the toJSON() function: it will return:
     *   - NaN when the key is a non-empty string
     *   - 0 when the key is an empty string (the top level 'holder' key
     *     is an empty string)
     *   - key as is, if the key is a number (array case)
     */

    Function.prototype.toJSON = Math.min;

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

    delete Function.prototype.toJSON;
}

try {
    print('JSON/JX/JC test');
    jsonJxJcTest();
} catch (e) {
    print(e.stack || e);
}

/*===
bound function test
F: function light_PTR_002f() {(* light *)}
F type tag: 9
G: function light_PTR_002f() {(* bound *)}
G type tag: 6
G.length: 1
H: function light_PTR_002f() {(* bound *)}
H type tag: 6
H.length: 0
I: function light_PTR_002f() {(* bound *)}
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
    print('F:', sanitizeLfunc(String(F)));
    print('F type tag:', Duktape.info(F)[0]);

    // 'G' will be a full Function object but will still have a lightfunc
    // 'name' as a result of ToObject coercion, which is intentional but
    // somewhat confusing.  This would be fixable in the ToObject() coercion.
    var G = F.bind('myThis', 234);  // bound once
    print('G:', sanitizeLfunc(String(G)));
    print('G type tag:', Duktape.info(G)[0]);
    print('G.length:', G.length);  // length 1, one argument was bound

    var H = G.bind('foo', 345);  // bound twice
    print('H:', sanitizeLfunc(String(H)));
    print('H type tag:', Duktape.info(H)[0]);
    print('H.length:', H.length);  // length 0, two arguments are bound

    var I = H.bind('foo', 345);  // bound three times
    print('I:', sanitizeLfunc(String(I)));
    print('I type tag:', Duktape.info(I)[0]);
    print('I.length:', I.length);  // length 0, doesn't become negative

    // Another simple test
    print('G(123):', G(123));
    print('G(123,987):', G(123, 987));
}

try {
    print('bound function test');
    boundFunctionTest();
} catch (e) {
    print(e.stack || e);
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
read from name -> light_PTR_002f
toString coerced object (return "length")
toString coerced object (return "length")
read from length -> 2
read from testWritable -> 123
read from testNonWritable -> 234
read from call -> function light_PTR_001f() {(* light *)}
read from apply -> function light_PTR_0022() {(* light *)}
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
            print('read from', k, '->', sanitizeLfunc(lightFunc[k]));
        } catch (e) {
            print('read from', k, '->', e.name);
        }
    });
}

try {
    print('property get test');
    propertyGetTest();
} catch (e) {
    print(e.stack || e);
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

try {
    print('property put test');
    propertyPutTest();
} catch (e) {
    print(e.stack || e);
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

try {
    print('property has test');
    propertyHasTest();
} catch (e) {
    print(e.stack || e);
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

try {
    print('property delete test');
    propertyDeleteTest();
} catch (e) {
    print(e.stack || e);
}

/*===
property accessor this binding test
getter, strict
strict getter "this" binding test
typeof this: function
this == lightFunc: true
this === lightFunc: true
this.name: light_PTR_002f
type tag: 9
getter retval
setter, strict
strict setter "this" binding test
typeof this: function
this == lightFunc: true
this === lightFunc: true
this.name: light_PTR_002f
type tag: 9
getter, non-strict
non-strict getter "this" binding test
typeof this: function
this == lightFunc: true
this === lightFunc: true
this.name: light_PTR_002f
type tag: 9
getter retval
setter, non-strict
non-strict setter "this" binding test
typeof this: function
this == lightFunc: true
this === lightFunc: true
this.name: light_PTR_002f
type tag: 9
===*/

function propertyAccessorThisBindingTest() {
    var lightFunc = getLightFunc();

    /*
     *  If a getter/setter is triggered by reading/writing an inherited
     *  property, the getter/setter 'this' binding is set to the lightfunc
     *  (and not, e.g., Function.prototype).
     *
     *  Because a lightfunc behaves like a full Function object, it is
     *  not coerced with ToObject() even when the accessor is non-strict.
     */

    Object.defineProperty(Function.prototype, 'testAccessorStrict', {
        get: function () {
            'use strict';
            print('strict getter "this" binding test');
            print('typeof this:', typeof this);
            print('this == lightFunc:', this == lightFunc);
            print('this === lightFunc:', this === lightFunc);
            print('this.name:', sanitizeLfunc(this.name));
            print('type tag:', Duktape.info(this)[0]);
            return 'getter retval';
        },
        set: function () {
            'use strict';
            print('strict setter "this" binding test');
            print('typeof this:', typeof this);
            print('this == lightFunc:', this == lightFunc);
            print('this === lightFunc:', this === lightFunc);
            print('this.name:', sanitizeLfunc(this.name));
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
            print('this.name:', sanitizeLfunc(this.name));
            print('type tag:', Duktape.info(this)[0]);
            return 'getter retval';
        },
        set: function () {
            print('non-strict setter "this" binding test');
            print('typeof this:', typeof this);
            print('this == lightFunc:', this == lightFunc);
            print('this === lightFunc:', this === lightFunc);
            print('this.name:', sanitizeLfunc(this.name));
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

try {
    print('property accessor this binding test');
    propertyAccessorThisBindingTest();
} catch (e) {
    print(e.stack || e);
}

/*===
property misc test
isSealed: true
isFrozen: true
isExtensible: false
for-in: "inheritTestProperty"
Object.getOwnPropertyNames: "length"
Object.getOwnPropertyNames: "name"
lightFunc.name matches regexp: true
censored lightFunc.name: light_XXX_XXX
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
}

try {
    print('property misc test');
    propertyMiscTest();
} catch (e) {
    print(e.stack || e);
}

/*===
traceback test
URIError: invalid input
	duk_bi_global.c:NNN
	light_PTR_0011 light strict preventsyield
	tracebackTest TESTCASE:NNN
	global TESTCASE:NNN preventsyield
===*/

function tracebackTest() {
    var err;

    try {
        decodeURIComponent('%x');
    } catch (e) {
        err = e;
    }

    // heavy sanitization
    print(sanitizeTraceback(err.stack));
}

try {
    print('traceback test');
    tracebackTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Duktape.act() test
Error: for traceback
	callback TESTCASE:NNN preventsyield
	light_PTR_0212 light strict preventsyield
	duktapeActTest TESTCASE:NNN
	global TESTCASE:NNN preventsyield
-1 ["lineNumber","pc","function"] light_PTR_0011
-2 ["lineNumber","pc","function"] callback
-3 ["lineNumber","pc","function"] light_PTR_0212
-4 ["lineNumber","pc","function"] duktapeActTest
-5 ["lineNumber","pc","function"] global
===*/

function duktapeActTest() {
    // This test assumes that Array.prototype.forEach() has been created
    // as a lightfunc, so that it appears in the middle of the callstack.

    [ 'foo' ].forEach(function callback(x) {
        var e = new Error('for traceback');
        var i;
        var a;

        print(sanitizeTraceback(e.stack));

        for (i = -1; ; i--) {
            a = Duktape.act(i);
            if (!a) { break; }
            print(i, Duktape.enc('jx', Object.getOwnPropertyNames(a)), sanitizeLfunc(a.function.name));
        }
    });
}

try {
    print('Duktape.act() test');
    duktapeActTest();
} catch (e) {
    print(e.stack || e);
}

/*===
exempt built-ins test
Math.max (is lightfunc): function true
eval: function false
yield: function false
resume: function false
require: function false
Object: function false
Function: function false
Array: function false
String: function false
Boolean: function false
Number: function false
Date: function false
RegExp: function false
Error: function false
EvalError: function false
RangeError: function false
ReferenceError: function false
SyntaxError: function false
TypeError: function false
URIError: function false
Proxy: function false
Duktape.Buffer: function false
Duktape.Pointer: function false
Duktape.Thread: function false
Duktape.Logger: function false
Duktape: object false
Math: object false
JSON: object false
===*/

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

try {
    print('exempt built-ins test');
    exemptBuiltinsTest();
} catch (e) {
    print(e.stack || e);
}

/*===
getOwnPropertyNames() test
length,name
length
name
===*/

function getOwnPropertyNamesTest() {
    var lfunc = Math.cos;
    var names;

    named = Object.getOwnPropertyNames(lfunc);
    print(named);
    if (named) {
        named.forEach(function (x) { print(x); });
    }
}

try {
    print('getOwnPropertyNames() test');
    getOwnPropertyNamesTest();
} catch (e) {
    print(e.stack || e);
}

/*===
getOwnPropertyDescriptor() test
key: name
value: string light_PTR_0511
writable: boolean false
enumerable: boolean false
configurable: boolean false
key: length
value: number 1
writable: boolean false
enumerable: boolean false
configurable: boolean false
key: nonExistent
no descriptor
===*/

function getOwnPropertyDescriptorTest() {
    var lfunc = Math.cos;

    function test(key) {
        print('key:', key);
        var pd = Object.getOwnPropertyDescriptor(lfunc, key);
        if (!pd) { print('no descriptor'); return; }
        print('value:', typeof pd.value, typeof pd.value === 'string' ? sanitizeLfunc(pd.value) : pd.value);
        print('writable:', typeof pd.writable, pd.writable);
        print('enumerable:', typeof pd.enumerable, pd.enumerable);
        print('configurable:', typeof pd.configurable, pd.configurable);
    }

    test('name');
    test('length');
    test('nonExistent');
}

try {
    print('getOwnPropertyDescriptor() test');
    getOwnPropertyDescriptorTest();
} catch (e) {
    print(e.stack || e);
}

/*===
hasOwnProperty() test
true
true
false
false
===*/

function hasOwnPropertyTest() {
    var lfunc = Math.cos;

    print(Object.prototype.hasOwnProperty.call(lfunc, 'name'));
    print(Object.prototype.hasOwnProperty.call(lfunc, 'length'));
    print(Object.prototype.hasOwnProperty.call(lfunc, 'nonExistent'));
    print(Object.prototype.hasOwnProperty.call(lfunc, 'call'));  // inherited
}

try {
    print('hasOwnProperty() test');
    hasOwnPropertyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
propertyIsEnumerable() test
false
false
false
false
===*/

function propertyIsEnumerableTest() {
    var lfunc = Math.cos;

    print(Object.prototype.propertyIsEnumerable(lfunc, 'name'));
    print(Object.prototype.propertyIsEnumerable(lfunc, 'length'));
    print(Object.prototype.propertyIsEnumerable(lfunc, 'nonExistent'));
    print(Object.prototype.propertyIsEnumerable(lfunc, 'call'));  // inherited
}

try {
    print('propertyIsEnumerable() test');
    propertyIsEnumerableTest();
} catch (e) {
    print(e.stack || e);
}

/*===
defineProperty() test
nonexistent: success
name: TypeError
length: success
===*/

function definePropertyTest() {
    /*
     *  Object.defineProperty() specification algorithm throws a TypeError if
     *  the argument is not an object.  Because we want lightfuncs to behave
     *  like full objects, Object.defineProperty() should work as if it was
     *  allowed and the lightfunc was an actual object.
     *
     *  The current implementation just coerces a lightfunc to an object with
     *  ToObject().  The resulting normal Function object has a copy of the
     *  lightfunc's virtual properties.  The Function object is also extensible
     *  (which is a bit odd because the lightfunc is not considered extensible).
     *  Because the coerced function is extensible, adding new properties will
     *  appear to succeed.  The temporary object is not exposed to calling code
     *  and is later garbage collected.
     *
     *  Another possible behavior might be an unconditional TypeError.  This
     *  would not match the ordinary defineProperty() behavior where it is
     *  possible to write even to a write-protected value if the new value
     *  SameValue() compares equal to the old value.
     *
     *  The preferred behavior is not clear.
     *
     *  Drawing some analogies from plain strings: plain string values can be
     *  assigned properties with no error, even in strict mode:
     *
     *    > function () { 'use strict'; return ('foo').bar=1; }()
     *    1
     *
     *  There is no analogy for defineProperty() because strings can't be used
     *  as an Object.defineProperty() argument:
     *
     *    > Object.defineProperty('foo', 'bar', { value: 1, writable: true, ... })
     *    TypeError: Object.defineProperty called on non-object
     *
     *  The extensibility of a plain string object also cannot be queried:
     *
     *    > Object.isExtensible('foo')
     *    TypeError: Object.isExtensible called on non-object
     *
     *  If a plain string is coerced to a full String object, it becomes
     *  extensible:
     *
     *    > Object.isExtensible(new String('foo'))
     *    true
     *
     *  So, the current behavior for a defineProperty() call is basically:
     *
     *    Object.defineProperty(Object(lightFunc), ...);
     *
     *  This will succeed in defining new properties (which are not reflected
     *  in the lightFunc value) which are effectively lost.
     */

    var lf = Math.max;

    // Non-existent property: succeeds because the temporary ToObject()
    // coerced object is extensible.  It might be more intuitive for this
    // to fail with "not extensible".
    try {
        Object.defineProperty(lf, 'nonexistent', {
            value: 123, writable: true, enumerable: true, configurable: true
        });
        print('nonexistent:', 'success');
    } catch (e) {
        print('nonexistent:', e.name);
    }

    // Existing non-configurable property with a different value: rejected
    // as part of normal defineProperty() handling.
    try {
        Object.defineProperty(lf, 'name', {
            value: 123, writable: true, enumerable: true, configurable: true
        });
        print('name:', 'success');
    } catch (e) {
        print('name:', e.name);
    }

    // Existing non-configurable property with same value as before: accepted
    // as part of normal defineProperty() handling.
    try {
        Object.defineProperty(lf, 'length', {
            value: 2, writable: false, enumerable: false, configurable: false
        });
        print('length:', 'success');
    } catch (e) {
        print('length:', e.name);
    }
}

try {
    print('defineProperty() test');
    definePropertyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
defineProperties() test
nonexistent: success
name: TypeError
length: success
===*/

function definePropertiesTest() {
    /*
     *  Same as definePropertyTest() but for Object.defineProperties: it has
     *  a slightly different internal code path so should be tested separately.
     */

    var lf = Math.max;

    // Non-existent property.
    try {
        Object.defineProperties(lf, { nonexistent: {
            value: 123, writable: true, enumerable: true, configurable: true
        } });
        print('nonexistent:', 'success');
    } catch (e) {
        print('nonexistent:', e.name);
    }

    // Existing non-configurable property, different value
    try {
        Object.defineProperties(lf, { name: {
            value: 123, writable: true, enumerable: true, configurable: true
        } });
        print('name:', 'success');
    } catch (e) {
        print('name:', e.name);
    }

    // Existing non-configurable property, same value
    try {
        Object.defineProperties(lf, { length: {
            value: 2, writable: false, enumerable: false, configurable: false
        } });
        print('length:', 'success');
    } catch (e) {
        print('length:', e.name);
    }
}

try {
    print('defineProperties() test');
    definePropertiesTest();
} catch (e) {
    print(e.stack || e);
}

/*===
getPrototypeOf() test
true
true
===*/

function getPrototypeOfTest() {
    var lfunc = Math.max;

    print(Object.getPrototypeOf(lfunc) === Function.prototype);
    print(lfunc.__proto__ === Function.prototype);
}

try {
    print('getPrototypeOf() test');
    getPrototypeOfTest();
} catch (e) {
    print(e.stack || e);
}

/*===
setPrototypeOf() test
TypeError
TypeError
TypeError
TypeError
success
success
success
success
===*/

function setPrototypeOfTest() {
    var lfunc = Math.max;
    var nonext;

    function err(cb) {
        try {
            cb();
            print('never here');
        } catch (e) {
            print(e.name);
        }
    }

    function succ(cb) {
        try {
            cb();
            print('success');
        } catch (e) {
            print(e);
        }
    }

    // Trying to change prototype is a TypeError.  The same behavior applies
    // to ordinary non-extensible objects.

    err(function () { var nonext = {}; Object.preventExtensions(nonext); Object.setPrototypeOf(nonext, {}); });
    err(function () { Object.setPrototypeOf(lfunc, {}); });

    err(function () { var nonext = {}; Object.preventExtensions(nonext); nonext.__proto__ = {}; });
    err(function () { lfunc.__proto__ = {}; });

    // Setting existing prototype value is a no-op.

    succ(function () { var nonext = {}; Object.preventExtensions(nonext); Object.setPrototypeOf(nonext, Object.prototype); });
    succ(function () { Object.setPrototypeOf(lfunc, Function.prototype); });

    succ(function () { var nonext = {}; Object.preventExtensions(nonext); nonext.__proto__ = Object.prototype; });
    succ(function () { lfunc.__proto__ = Function.prototype; });
}

try {
    print('setPrototypeOf() test');
    setPrototypeOfTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Array built-in test
Array: object [{_func:true}]
new Array: object [{_func:true}]
isArray: boolean false
toString: string "[object Function]"
valueOf: function {_func:true}
concat: object [{_func:true}]
pop: TypeError
push: TypeError
sort: function {_func:true}
splice: TypeError
reverse: function {_func:true}
shift: TypeError
unshift: TypeError
every: TypeError
some: TypeError
forEach: TypeError
map: TypeError
filter: TypeError
reduce: TypeError
reduceRight: TypeError
===*/

function arrayBuiltinTest() {
    var lfunc = Math.cos;

    function f(meth) {
        testTypedJx(function () {
            return Array.prototype[meth].call(lfunc);
        }, meth);
    }

    testTypedJx(function () { return Array(lfunc) }, 'Array');
    testTypedJx(function () { return new Array(lfunc) }, 'new Array');
    testTypedJx(function () { return Array.isArray(lfunc) }, 'isArray');

    // these tests are not particularly useful, but might reveal some
    // assertion errors

    f('toString');
    f('valueOf');
    f('concat');
    f('pop');          // TypeError, length not writable
    f('push');         // TypeError, length not writable
    f('sort');
    f('splice');       // TypeError, length not writable
    f('reverse');
    f('shift');        // TypeError, length not writable
    f('unshift');      // TypeError, length not writable
    f('every');        // TypeError, callback not a function
    f('some');         // -''-
    f('forEach');      // -''-
    f('map');          // -''-
    f('filter');       // -''-
    f('reduce');       // -''-
    f('reduceRight');  // -''-
}

try {
    print('Array built-in test');
    arrayBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Boolean built-in test
Boolean: boolean true
new Boolean: object true
toString: TypeError
valueOf: TypeError
===*/

function booleanBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Boolean(lfunc) }, 'Boolean');
    testTypedJx(function () { return new Boolean(lfunc) }, 'new Boolean');
    testTypedJx(function () { return Boolean.prototype.toString.call(lfunc, lfunc) }, 'toString');
    testTypedJx(function () { return Boolean.prototype.valueOf.call(lfunc, lfunc) }, 'valueOf');
}

try {
    print('Boolean built-in test');
    booleanBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Duktape.Buffer built-in test
Duktape.Buffer: TypeError
new Duktape.buffer: TypeError
toString: TypeError
valueOf: TypeError
===*/

function duktapeBufferBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Duktape.Buffer(lfunc) }, 'Duktape.Buffer');
    testTypedJx(function () { return new Duktape.Buffer(lfunc) }, 'new Duktape.buffer');
    testTypedJx(function () { return Duktape.Buffer.prototype.toString.call(lfunc, lfunc) }, 'toString');
    testTypedJx(function () { return Duktape.Buffer.prototype.valueOf.call(lfunc, lfunc) }, 'valueOf');
}

try {
    print('Duktape.Buffer built-in test');
    duktapeBufferBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Date built-in test
Date: string "string"
new Date: string "object"
parse: number NaN
UTC: number NaN
now: string "number"
toString: TypeError
valueOf: TypeError
toDateString: TypeError
toTimeString: TypeError
toLocaleString: TypeError
toLocaleDateString: TypeError
toLocaleTimeString: TypeError
getTime: TypeError
getFullYear: TypeError
getUTCFullYear: TypeError
getMonth: TypeError
getUTCFullMonth: TypeError
getDate: TypeError
getUTCDate: TypeError
getDay: TypeError
getUTCDay: TypeError
getHours: TypeError
getUTCHours: TypeError
getMinutes: TypeError
getUTCMinutes: TypeError
getSeconds: TypeError
getUTCSeconds: TypeError
getMilliseconds: TypeError
getUTCMilliseconds: TypeError
getTimezoneOffset: TypeError
setTime: TypeError
setMilliseconds: TypeError
setUTCMilliseconds: TypeError
setSeconds: TypeError
setUTCSeconds: TypeError
setMinutes: TypeError
setUTCMinutes: TypeError
setHours: TypeError
setUTCHours: TypeError
setDate: TypeError
setUTCDate: TypeError
setMonth: TypeError
setUTCMonth: TypeError
setFullYear: TypeError
setUTCFullYear: TypeError
toUTCString: TypeError
toISOString: TypeError
toJSON: TypeError
setYear: TypeError
getYear: TypeError
===*/

function dateBuiltinTest() {
    var lfunc = Math.cos;

    function f(meth) {
        testTypedJx(function () {
            return Date.prototype[meth].call(lfunc, lfunc);
        }, meth);
    }

    testTypedJx(function () { return typeof Date(lfunc) }, 'Date');
    testTypedJx(function () { return typeof new Date(lfunc) }, 'new Date');
    testTypedJx(function () { return Date.parse(lfunc) }, 'parse');
    testTypedJx(function () { return Date.UTC(lfunc) }, 'UTC');
    testTypedJx(function () { return typeof Date.now(lfunc) }, 'now');

    f('toString');
    f('valueOf');
    f('toDateString');
    f('toTimeString');
    f('toLocaleString');
    f('toLocaleDateString');
    f('toLocaleTimeString');
    f('getTime');
    f('getFullYear');
    f('getUTCFullYear');
    f('getMonth');
    f('getUTCFullMonth');
    f('getDate');
    f('getUTCDate');
    f('getDay');
    f('getUTCDay');
    f('getHours');
    f('getUTCHours');
    f('getMinutes');
    f('getUTCMinutes');
    f('getSeconds');
    f('getUTCSeconds');
    f('getMilliseconds');
    f('getUTCMilliseconds');
    f('getTimezoneOffset');
    f('setTime');
    f('setMilliseconds');
    f('setUTCMilliseconds');
    f('setSeconds');
    f('setUTCSeconds');
    f('setMinutes');
    f('setUTCMinutes');
    f('setHours');
    f('setUTCHours');
    f('setDate');
    f('setUTCDate');
    f('setMonth');
    f('setUTCMonth');
    f('setFullYear');
    f('setUTCFullYear');
    f('toUTCString');
    f('toISOString');
    f('toJSON');
    f('setYear');
    f('getYear');
}

try {
    print('Date built-in test');
    dateBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Duktape built-in test
info: object [9]
act: undefined undefined
gc: boolean true
fin-get: TypeError
fin-set: TypeError
encdec-hex: string "function light_PTR_0511() {(* light *)}"
dec-hex: TypeError
compact: function {_func:true}
===*/

function duktapeBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Duktape.info(lfunc); }, 'info');

    // doesn't really make sense
    testTypedJx(function () { return Duktape.act(lfunc); }, 'act');

    // doesn't really make sense (will act like Duktape.gc(0))
    testTypedJx(function () { return Duktape.gc(lfunc); }, 'gc');

    // attempt to get finalizer
    testTypedJx(function () { return Duktape.fin(lfunc); }, 'fin-get');

    // attempt to set finalizer
    testTypedJx(function () { return Duktape.fin(lfunc, function () {}); }, 'fin-set');

    testTypedJx(function () { return sanitizeLfunc(Duktape.dec('hex', Duktape.enc('hex', lfunc))); }, 'encdec-hex');
    testTypedJx(function () { return Duktape.dec('hex', lfunc); }, 'dec-hex');

    // attempt to compact is a no-op
    testTypedJx(function () { return Duktape.compact(lfunc); }, 'compact');
}

try {
    print('Duktape built-in test');
    duktapeBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Error built-in test
Error: object {}
new Error: object {}
toString: string "light_PTR_0511"
valueOf: function {_func:true}
===*/

function errorBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Error(lfunc); }, 'Error');
    testTypedJx(function () { return new Error(lfunc); }, 'new Error');
    testTypedJx(function () { return sanitizeLfunc(Error.prototype.toString.call(lfunc, lfunc)); }, 'toString');
    testTypedJx(function () { return Error.prototype.valueOf.call(lfunc, lfunc); }, 'valueOf');
}

try {
    print('Error built-in test');
    errorBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Function built-in test
Function: function {_func:true}
new Function: function {_func:true}
toString: string "function light_PTR_0511() {(* light *)}"
valueOf: function {_func:true}
===*/

function functionBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Function(lfunc); }, 'Function');
    testTypedJx(function () { return new Function(lfunc); }, 'new Function');
    testTypedJx(function () { return sanitizeLfunc(Function.prototype.toString.call(lfunc, lfunc)) }, 'toString');
    testTypedJx(function () { return Function.prototype.valueOf.call(lfunc, lfunc) }, 'valueOf');

    // Already covered by other tests:
    // - call
    // - apply
    // - bind
}

try {
    print('Function built-in test');
    functionBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
global built-in test
eval: function {_func:true}
parseInt: number NaN
parseFloat: number NaN
isNaN: boolean true
isFinite: boolean false
decodeURI: string "function light_PTR_0511() {(* light *)}"
decodeURIComponent: string "function light_PTR_0511() {(* light *)}"
encodeURI: string "string"
encodeURIComponent: string "string"
escape: string "string"
unescape: string "string"
===*/

function globalBuiltinTest() {
    var lfunc = Math.cos;

    // This interestingly evaluates to a function because ToString(lfunc) parses
    // to a valid function declaration.
    testTypedJx(function () { return eval(lfunc); }, 'eval');

    testTypedJx(function () { return parseInt(lfunc); }, 'parseInt');
    testTypedJx(function () { return parseFloat(lfunc); }, 'parseFloat');
    testTypedJx(function () { return isNaN(lfunc); }, 'isNaN');
    testTypedJx(function () { return isFinite(lfunc); }, 'isFinite');

    // Must sanitize here
    testTypedJx(function () { return sanitizeLfunc(decodeURI(lfunc)); }, 'decodeURI');
    testTypedJx(function () { return sanitizeLfunc(decodeURIComponent(lfunc)); }, 'decodeURIComponent');

    // The encoded output would need to be sanitized; just check it's a string
    testTypedJx(function () { return typeof encodeURI(lfunc); }, 'encodeURI');
    testTypedJx(function () { return typeof encodeURIComponent(lfunc); }, 'encodeURIComponent');
    testTypedJx(function () { return typeof escape(lfunc); }, 'escape');
    testTypedJx(function () { return typeof unescape(lfunc); }, 'unescape');
}

try {
    print('global built-in test');
    globalBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
JSON built-in test
parse: SyntaxError
stringify: undefined undefined
===*/

function jsonBuiltinTest() {
    var lfunc = Math.cos;

    // covered elsewhere too
    testTypedJx(function () { return JSON.parse(lfunc); }, 'parse');
    testTypedJx(function () { return JSON.stringify(lfunc); }, 'stringify');
}

try {
    print('JSON built-in test');
    jsonBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Duktape.Logger built-in test
Duktape.Logger: TypeError
new Duktape.Logger: object {}
fmt: TypeError
raw: TypeError
TIMESTAMP INF test: My light func is: function light_PTR_0511() {(* light *)}
===*/

function duktapeLoggerBuiltinTest() {
    var lfunc = Math.cos;
    var old_raw;
    var logger;

    testTypedJx(function () { return Duktape.Logger(lfunc); }, 'Duktape.Logger');
    testTypedJx(function () { return new Duktape.Logger(lfunc); }, 'new Duktape.Logger');
    testTypedJx(function () { return Duktape.logger.prototype.fmt(lfunc); }, 'fmt');
    testTypedJx(function () { return Duktape.logger.prototype.raw(lfunc); }, 'raw');

    // Test that lightfuncs log in a useful way.  Because the toString()
    // coercion contains a pointer we need to abduct the raw() function.

    old_raw = Duktape.Logger.prototype.old_raw;
    Duktape.Logger.prototype.raw = function (buf) {
        var msg = sanitizeLfunc(String(buf));
        msg = msg.replace(/^\S+/, 'TIMESTAMP');
        print(msg);
    };
    logger = new Duktape.Logger('test');
    logger.info('My light func is:', lfunc);
    Duktape.Logger.prototype.raw = old_raw;
}

try {
    print('Duktape.Logger built-in test');
    duktapeLoggerBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Math built-in test
abs: number NaN
acos: number NaN
asin: number NaN
atan: number NaN
atan2: number NaN
ceil: number NaN
cos: number NaN
exp: number NaN
floor: number NaN
log: number NaN
max: number NaN
min: number NaN
pow: number NaN
random: string "number"
round: number NaN
sin: number NaN
sqrt: number NaN
tan: number NaN
===*/

function mathBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Math.abs(lfunc); }, 'abs');
    testTypedJx(function () { return Math.acos(lfunc); }, 'acos');
    testTypedJx(function () { return Math.asin(lfunc); }, 'asin');
    testTypedJx(function () { return Math.atan(lfunc); }, 'atan');
    testTypedJx(function () { return Math.atan2(lfunc); }, 'atan2');
    testTypedJx(function () { return Math.ceil(lfunc); }, 'ceil');
    testTypedJx(function () { return Math.cos(lfunc); }, 'cos');
    testTypedJx(function () { return Math.exp(lfunc); }, 'exp');
    testTypedJx(function () { return Math.floor(lfunc); }, 'floor');
    testTypedJx(function () { return Math.log(lfunc); }, 'log');
    testTypedJx(function () { return Math.max(lfunc); }, 'max');
    testTypedJx(function () { return Math.min(lfunc); }, 'min');
    testTypedJx(function () { return Math.pow(lfunc); }, 'pow');
    testTypedJx(function () { return typeof Math.random(lfunc); }, 'random');  // avoid outputting result value
    testTypedJx(function () { return Math.round(lfunc); }, 'round');
    testTypedJx(function () { return Math.sin(lfunc); }, 'sin');
    testTypedJx(function () { return Math.sqrt(lfunc); }, 'sqrt');
    testTypedJx(function () { return Math.tan(lfunc); }, 'tan');
}

try {
    print('Math built-in test');
    mathBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Number built-in test
Number: number NaN
new Number: object NaN
toString: TypeError
toLocaleString: TypeError
valueOf: TypeError
toFixed: TypeError
toExponential: TypeError
toPrecision: TypeError
===*/

function numberBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Number(lfunc); }, 'Number');
    testTypedJx(function () { return new Number(lfunc); }, 'new Number');
    testTypedJx(function () { return Number.prototype.toString.call(lfunc, lfunc); }, 'toString');
    testTypedJx(function () { return Number.prototype.toLocaleString.call(lfunc, lfunc); }, 'toLocaleString');
    testTypedJx(function () { return Number.prototype.valueOf.call(lfunc, lfunc); }, 'valueOf');
    testTypedJx(function () { return Number.prototype.toFixed.call(lfunc, lfunc); }, 'toFixed');
    testTypedJx(function () { return Number.prototype.toExponential.call(lfunc, lfunc); }, 'toExponential');
    testTypedJx(function () { return Number.prototype.toPrecision.call(lfunc, lfunc); }, 'toPrecision');
}

try {
    print('Number built-in test');
    numberBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Object built-in test
Object: function {_func:true}
new Object: function {_func:true}
getPrototypeOf: function {_func:true}
setPrototypeOf: TypeError
seal: function {_func:true}
freeze: function {_func:true}
preventExtensions: function {_func:true}
isSealed: boolean true
isFrozen: boolean true
isExtensible: boolean false
toString: string "[object Function]"
toLocaleString: string "function light_PTR_0511() {(* native *)}"
valueOf: function {_func:true}
isPrototypeOf: boolean false
===*/

function objectBuiltinTest() {
    var lfunc = Math.cos;

    // Object coercion: return ToObject, i.e. normal function
    testTypedJx(function () { return Object(lfunc); }, 'Object');

    // new Object(x) is supposed to return 'x' as is, if it is already a
    // function.  This is tricky for lightfuncs: should the input be returned
    // as is (= a lightfunc) or ToObject coerced version of input (not a
    // lightfunc)?  At the moment ToObject(lightfunc) is returned.

    testTypedJx(function () { return new Object(lfunc); }, 'new Object');
    testTypedJx(function () { return Object.getPrototypeOf(lfunc); }, 'getPrototypeOf');
    testTypedJx(function () { return Object.setPrototypeOf(lfunc, {}); }, 'setPrototypeOf');
    testTypedJx(function () { return Object.seal(lfunc); }, 'seal');
    testTypedJx(function () { return Object.freeze(lfunc); }, 'freeze');
    testTypedJx(function () { return Object.preventExtensions(lfunc); }, 'preventExtensions');
    testTypedJx(function () { return Object.isSealed(lfunc); }, 'isSealed');
    testTypedJx(function () { return Object.isFrozen(lfunc); }, 'isFrozen');
    testTypedJx(function () { return Object.isExtensible(lfunc); }, 'isExtensible');

    // Covered elsewhere:
    // - getOwnPropertyDescriptor()
    // - defineProperty()
    // - defineProperties()
    // - keys()

    testTypedJx(function () { return Object.prototype.toString.call(lfunc, lfunc); }, 'toString');
    testTypedJx(function () { return sanitizeLfunc(Object.prototype.toLocaleString.call(lfunc, lfunc)); }, 'toLocaleString');
    testTypedJx(function () { return Object.prototype.valueOf.call(lfunc, lfunc); }, 'valueOf');
    testTypedJx(function () { return Object.prototype.isPrototypeOf.call(lfunc, lfunc); }, 'isPrototypeOf');
}

try {
    print('Object built-in test');
    objectBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Duktape.Pointer built-in test
Duktape.Pointer: pointer (null)
new Duktape.Pointer: object (null)
toString: TypeError
valueOf: TypeError
===*/

function duktapePointerBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return Duktape.Pointer(lfunc); }, 'Duktape.Pointer');
    testTypedJx(function () { return new Duktape.Pointer(lfunc); }, 'new Duktape.Pointer');
    testTypedJx(function () { return Duktape.Pointer.prototype.toString.call(lfunc); }, 'toString');
    testTypedJx(function () { return Duktape.Pointer.prototype.toString.call(lfunc); }, 'valueOf');
}

try {
    print('Duktape.Pointer built-in test');
    duktapePointerBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Proxy built-in test
get
this: object false [object Object]
target: function false function light_PTR_0511() {(* native *)}
key: string name
proxy.name: light_PTR_0511
get
this: object false [object Object]
target: function false function light_PTR_0511() {(* native *)}
key: string length
proxy.length: 1
get
this: object false [object Object]
target: function false function light_PTR_0511() {(* native *)}
key: string nonExistent
proxy.nonExistent: dummy
get
this: function false function light_PTR_0511() {(* native *)}
target: object false [object Object]
key: string foo
proxy.foo: bar
get
this: function false function light_PTR_0511() {(* native *)}
target: object false [object Object]
key: string nonExistent
proxy.nonExistent: dummy
===*/

function proxyBuiltinTest() {
    var lfunc = Math.cos;

    // Proxy as a target object; ES6 requires that must be an Object and a
    // lightfunc pretends to be an object.  So, it must be possible to use
    // lightfunc as a target.  Currently Proxy will just coerce the lightfunc
    // to a full Function silently.

    var handler = {}
    var proxy = new Proxy(lfunc, handler);
    handler.get = function (target, key) {
        print('get');
        print('this:', typeof this, isLightFunc(this), sanitizeLfunc(this));
        print('target:', typeof target, isLightFunc(target), sanitizeLfunc(target));
        print('key:', typeof key, key);
        return target[key] || 'dummy';  // passthrough
    }
    print('proxy.name:', sanitizeLfunc(proxy.name));
    print('proxy.length:', proxy.length);
    print('proxy.nonExistent:', proxy.nonExistent);

    // Proxy as a handler value; ES6 requires it must be an Object and a
    // lightfunc pretends to be an object.  The traps must be placed in
    // Function.prototype for it to actually work - so this is not a very
    // useful thing.  Currently Proxy will just coerce the lightfunc to a
    // full Function silently.

    var proxy = new Proxy({ foo: 'bar' }, lfunc);
    Function.prototype.get = function (target, key) {
        print('get');
        print('this:', typeof this, isLightFunc(this), sanitizeLfunc(this));
        print('target:', typeof target, isLightFunc(target), target);
        print('key:', typeof key, key);
        return target[key] || 'dummy';  // passthrough
    };
    print('proxy.foo:', proxy.foo);
    print('proxy.nonExistent:', proxy.nonExistent);
    delete Function.prototype.get;
}

try {
    print('Proxy built-in test');
    proxyBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
RegExp built-in test
RegExp: SyntaxError
new RegExp: SyntaxError
exec: TypeError
test: TypeError
toString: TypeError
valueOf: function {_func:true}
===*/

function regexpBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return RegExp(lfunc); }, 'RegExp');
    testTypedJx(function () { return new RegExp(lfunc); }, 'new RegExp');

    testTypedJx(function () { return RegExp.prototype.exec.call(lfunc, lfunc); }, 'exec');
    testTypedJx(function () { return RegExp.prototype.test.call(lfunc, lfunc); }, 'test');
    testTypedJx(function () { return RegExp.prototype.toString.call(lfunc, lfunc); }, 'toString');
    testTypedJx(function () { return RegExp.prototype.valueOf.call(lfunc, lfunc); }, 'valueOf');
}

try {
    print('RegExp built-in test');
    regexpBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
String built-in test
String: string "function light_PTR_0511() {(* light *)}"
new String: string "function light_PTR_0511() {(* light *)}"
new String: string "object"
fromCharCode: string "\x00"
toString: TypeError
valueOf: TypeError
charAt: string "f"
charCodeAt: number 102
concat: string "function light_PTR_0511() {(* light *)}function light_PTR_0511() {(* light *)}"
indexOf: number 0
lastIndexOf: number 0
localeCompare: number 0
match: SyntaxError
replace: string "undefined"
search: SyntaxError
slice: string "function light_PTR_0511() {(* light *)}"
split: object ["",""]
substring: string "function light_PTR_0511() {(* light *)}"
toLowerCase: string "function light_PTR_0511() {(* light *)}"
toLocaleLowerCase: string "function light_PTR_0511() {(* light *)}"
toUpperCase: string "FUNCTION LIGHT_PTR_0511() {(* LIGHT *)}"
toLocaleUpperCase: string "FUNCTION LIGHT_PTR_0511() {(* LIGHT *)}"
trim: string "function light_PTR_0511() {(* light *)}"
substr: string "function light_PTR_0511() {(* light *)}"
===*/

function stringBuiltinTest() {
    var lfunc = Math.cos;

    testTypedJx(function () { return sanitizeLfunc(String(lfunc)); }, 'String');

    // new String() returns an object, but sanitizeLfunc() coerces it to a
    // plain string; check return type separately
    testTypedJx(function () { return sanitizeLfunc(new String(lfunc)); }, 'new String');
    testTypedJx(function () { return typeof new String(lfunc); }, 'new String');

    testTypedJx(function () { return String.fromCharCode(lfunc); }, 'fromCharCode');

    testTypedJx(function () { return String.prototype.toString.call(lfunc, lfunc); }, 'toString');
    testTypedJx(function () { return String.prototype.valueOf.call(lfunc, lfunc); }, 'valueOf');
    testTypedJx(function () { return String.prototype.charAt.call(lfunc, lfunc); }, 'charAt');
    testTypedJx(function () { return String.prototype.charCodeAt.call(lfunc, lfunc); }, 'charCodeAt');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.concat.call(lfunc, lfunc)); }, 'concat');
    testTypedJx(function () { return String.prototype.indexOf.call(lfunc, lfunc); }, 'indexOf');
    testTypedJx(function () { return String.prototype.lastIndexOf.call(lfunc, lfunc); }, 'lastIndexOf');
    testTypedJx(function () { return String.prototype.localeCompare.call(lfunc, lfunc); }, 'localeCompare');
    testTypedJx(function () { return String.prototype.match.call(lfunc, lfunc); }, 'match');
    testTypedJx(function () { return String.prototype.replace.call(lfunc, lfunc); }, 'replace');
    testTypedJx(function () { return String.prototype.search.call(lfunc, lfunc); }, 'search');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.slice.call(lfunc, lfunc)); }, 'slice');
    testTypedJx(function () { return String.prototype.split.call(lfunc, lfunc); }, 'split');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.substring.call(lfunc, lfunc)); }, 'substring');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.toLowerCase.call(lfunc, lfunc)); }, 'toLowerCase');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.toLocaleLowerCase.call(lfunc, lfunc)); }, 'toLocaleLowerCase');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.toUpperCase.call(lfunc, lfunc)); }, 'toUpperCase');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.toLocaleUpperCase.call(lfunc, lfunc)); }, 'toLocaleUpperCase');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.trim.call(lfunc, lfunc)); }, 'trim');
    testTypedJx(function () { return sanitizeLfunc(String.prototype.substr.call(lfunc, lfunc)); }, 'substr');
}

try {
    print('String built-in test');
    stringBuiltinTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Duktape.Thread built-in test
TypeError
TypeError
===*/

function duktapeThreadBuiltinTest() {
    var lfunc = Math.cos;
    var thr;

    // Lightfunc should be accepted as an initial function for a thread, but
    // as of Duktape 1.0 only non-bound Ecmascript functions are allowed.
    try {
        thr = new Duktape.Thread(lfunc);
        print(Duktape.Thread.resume(thr, 1.23));
    } catch (e) {
        print(e.name);
    }

    try {
        print(Duktape.Thread.yield(lfunc, 1.23));
    } catch (e) {
        print(e.name);
    }
}

try {
    print('Duktape.Thread built-in test');
    duktapeThreadBuiltinTest();
} catch (e) {
    print(e.stack || e);
}
