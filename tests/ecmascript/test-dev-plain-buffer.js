/*
 *  Plain buffer values behavior.  In Duktape 2.x plain buffers mostly behave
 *  like ArrayBuffers for Ecmascript code.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

function createPlain() {
    var pb = createPlainBuffer(16);
    for (var i = 0; i < 16; i++) {
        pb[i] = 0x61 + i;
    }
    return pb;
}

function createArrayBuffer() {
    var ab = new ArrayBuffer(16);
    for (var i = 0; i < 16; i++) {
        ab[i] = 0x61 + i;
    }
    return ab;
}

function createNodejsBuffer() {
    var nb = new Buffer(16);
    for (var i = 0; i < 16; i++) {
        nb[i] = 0x61 + i;
    }
    return nb;
}

/*===
basic test
object
object
[object ArrayBuffer]
[object ArrayBuffer]
true
true
===*/

function basicTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();

    // typeof
    print(typeof pb);  // 'buffer' in Duktape 1.x, 'object' in Duktape 2.x
    print(typeof ab);  // 'object'

    // class name in Object.prototype.toString()
    print(Object.prototype.toString.call(pb));  // '[object Buffer]' in Duktape 1.x, '[object ArrayBuffer]' in Duktape 2.x
    print(Object.prototype.toString.call(ab));  // '[object ArrayBuffer]'

    // instanceof
    print(pb instanceof ArrayBuffer);
    print(ab instanceof ArrayBuffer);
}

/*===
property test
16
16
16
16
0
0
1
1
undefined
undefined
97
97
===*/

function propertyTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();

    // ArrayBuffer virtual properties
    print(pb.length);
    print(ab.length);
    print(pb.byteLength);
    print(ab.byteLength);
    print(pb.byteOffset);
    print(ab.byteOffset);
    print(pb.BYTES_PER_ELEMENT);
    print(ab.BYTES_PER_ELEMENT);
    print(pb.buffer);  // not present
    print(ab.buffer);  // not present
    print(pb[0]);
    print(ab[0]);
}

/*===
enumeration test
for-in plain
for-in object
for-in plain
myEnumerable
for-in object
myEnumerable
Object.keys plain
Object.keys object
Object.keys plain
Object.keys object
Object.getOwnPropertyNames plain
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
length
byteLength
byteOffset
BYTES_PER_ELEMENT
Object.getOwnPropertyNames object
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
length
byteLength
byteOffset
BYTES_PER_ELEMENT
===*/

function enumerationTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();
    var k;

    // No enumerable properties by default.

    print('for-in plain');
    for (k in pb) {
        print(k);
    }
    print('for-in object');
    for (k in ab) {
        print(k);
    }

    // Add enumerable inherited property.

    ArrayBuffer.prototype.myEnumerable = 1;
    print('for-in plain');
    for (k in pb) {
        print(k);
    }
    print('for-in object');
    for (k in ab) {
        print(k);
    }
    delete ArrayBuffer.prototype.myEnumerable;

    // Object.keys() will include only own enumerable keys.

    print('Object.keys plain');
    Object.keys(pb).forEach(function (k) {
        print(k);
    });
    print('Object.keys object');
    Object.keys(ab).forEach(function (k) {
        print(k);
    });

    // Inherited properties not included.

    ArrayBuffer.prototype.myEnumerable = 1;
    print('Object.keys plain');
    Object.keys(pb).forEach(function (k) {
        print(k);
    });
    print('Object.keys object');
    Object.keys(ab).forEach(function (k) {
        print(k);
    });
    delete ArrayBuffer.prototype.myEnumerable;

    // Object.getOwnPropertyNames() will include all own properties,
    // including non-enumerable ones.

    print('Object.getOwnPropertyNames plain');
    Object.getOwnPropertyNames(pb).forEach(function (k) {
        print(k);
    });
    print('Object.getOwnPropertyNames object');
    Object.getOwnPropertyNames(ab).forEach(function (k) {
        print(k);
    });
}

/*===
read/write coercion test
-1234 46 46
-256 0 0
-255 1 1
-1.6 255 255
-1.4 255 255
-1 255 255
-0.6 0 0
-0.4 0 0
-0 0 0
0 0 0
0.4 0 0
0.6 0 0
1 1 1
1.4 1 1
1.6 1 1
255 255 255
256 0 0
1234 210 210
NaN 0 0
Infinity 0 0
-Infinity 0 0
"123" 123 123
"130" 130 130
"-123" 133 133
"-130" 126 126
===*/

function readWriteCoercionTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();

    [
      -1234, -256, -255, -1.6, -1.4, -1, -0.6, -0.4, -0,
      +0, 0.4, 0.6, 1, 1.4, 1.6, 255, 256, 1234,
      0/0, 1/0, -1/0, '123', '130', '-123', '-130'
    ].forEach(function (v) {
        pb[0] = v;
        ab[0] = v;
        print(Duktape.enc('jx', v), Duktape.enc('jx', pb[0]), Duktape.enc('jx', ab[0]));
    });
}

/*===
operator test
[object ArrayBuffer][object ArrayBuffer]
[object ArrayBuffer][object ArrayBuffer]
false
false
true
true
false
false
true
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
string "length" true true
string "byteLength" true true
string "byteOffset" true true
string "BYTES_PER_ELEMENT" true true
number -1 false false
number 0 true true
number 15 true true
number 16 false false
string "15" true true
string "16" false false
string "15.0" false false
true
true
true
true
===*/

function operatorTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();

    // '+' operator
    print(pb + pb);  // 'abcdefghijklmnopabcdefghijklmnop' in Duktape 1.x, '[object ArrayBuffer][object ArrayBuffer]' in Duktape 2.x
    print(ab + ab);  // '[object ArrayBuffer][object ArrayBuffer]'

    // equality comparison: no content comparison in Duktape 2.x when
    // comparing plain buffers using '==' (or '==='), all comparisons
    // are now reference based
    print(createPlain() == createPlain());
    print(createPlain() === createPlain());
    print(pb == pb);
    print(pb === pb);
    print(createArrayBuffer() == createArrayBuffer());
    print(createArrayBuffer() === createArrayBuffer());
    print(ab == ab);
    print(ab === ab);
    print(pb == ab);
    print(pb === ab);

    // number and string comparisons are always false
    pb = createPlainBuffer(4);
    pb[0] = '1'.charCodeAt(0);
    pb[1] = '2'.charCodeAt(0);
    pb[2] = '3'.charCodeAt(0);
    pb[3] = '4'.charCodeAt(0);
    ab = new ArrayBuffer(4);
    ab[0] = '1'.charCodeAt(0);
    ab[1] = '2'.charCodeAt(0);
    ab[2] = '3'.charCodeAt(0);
    ab[3] = '4'.charCodeAt(0);
    print(pb == 1234);
    print(pb === 1234);
    print(ab == 1234);
    print(ab === 1234);
    print(pb == '1234');
    print(pb === '1234');
    print(ab == '1234');
    print(ab === '1234');
    pb = createPlain();  // reset after change
    ab = createArrayBuffer();

    // object comparison is always false
    print(pb == {});
    print(pb === {});
    print(ab == {});
    print(ab === {});

    [ 'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT', -1, 0, 15, 16, '15', '16', '15.0' ].forEach(function (v) {
        print(typeof v, Duktape.enc('jx', v), v in pb, v in ab);
    });

    // plain buffers ToBoolean() coerce to true, even if a zero length
    // buffer (this differs from Duktape 1.x)
    print(!!pb);
    print(!!ab);
    print(!!createPlainBuffer(0));
    print(!!new ArrayBuffer(0));
}

/*===
coercion test
false
true
[object ArrayBuffer]
[object ArrayBuffer]
[Overridden]
[Overridden]
TypeError: coercion to primitive failed
TypeError: coercion to primitive failed
TypeError: coercion to primitive failed
TypeError: coercion to primitive failed
NaN
NaN
123
123
===*/

function coercionTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();

    // ES5 coercions

    // ToObject() coercion returns a full ArrayBuffer object, so that
    // Object(plain) !== plain.  This matches current lightfunc behavior
    // but is not necessarily very intuitive.
    print(Object(pb) === pb);
    print(Object(ab) === ab);

    // ToString() coercion
    print(String(pb));
    print(String(ab));

    // ToString goes through ArrayBuffer.prototype
    ArrayBuffer.prototype.toString = function () { return '[Overridden]'; };
    print(String(pb));
    print(String(ab));
    delete ArrayBuffer.prototype.toString;

    // ToString() when overridden .toString() and .valueOf() also return a
    // plain buffer; causes a TypeError (matches V8 behavior for ArrayBuffer)
    ArrayBuffer.prototype.toString = function () { return createPlain(); };
    ArrayBuffer.prototype.valueOf = function () { return createPlain(); };
    try {
        print(String(pb));
    } catch (e) {
        print(e);
    }
    try {
        print(String(ab));
    } catch (e) {
        print(e);
    }
    delete ArrayBuffer.prototype.toString;
    delete ArrayBuffer.prototype.valueOf;

    // Same behavior if .toString() returns an ArrayBuffer object
    ArrayBuffer.prototype.toString = function () { return createArrayBuffer(); };
    ArrayBuffer.prototype.valueOf = function () { return createArrayBuffer(); };
    try {
        print(String(pb));
    } catch (e) {
        print(e);
    }
    try {
        print(String(ab));
    } catch (e) {
        print(e);
    }
    delete ArrayBuffer.prototype.toString;
    delete ArrayBuffer.prototype.valueOf;

    // ToNumber() coerces via ToString(); usually results in NaN but by
    // overriding .toString() one can get a number result
    print(Number(pb));
    print(Number(ab));
    try {
        ArrayBuffer.prototype.toString = function () { return '123'; };
        print(Number(pb));
        print(Number(ab));
    } catch (e) {
        print(e);
    }
    delete ArrayBuffer.prototype.toString;

}

/*===
json test
{}
{}
|6162636465666768696a6b6c6d6e6f70|
|6162636465666768696a6b6c6d6e6f70|
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
{"bufferLength":16,"plain":false,"data":"abcdefghijklmnop"}
{bufferLength:16,plain:true,data:"abcdefghijklmnop"}
{bufferLength:16,plain:false,data:"abcdefghijklmnop"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
{"bufferLength":16,"plain":false,"data":"abcdefghijklmnop"}
{}
{}
|6162636465666768696a6b6c6d6e6f70|
|6162636465666768696a6b6c6d6e6f70|
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
{"bufferLength":16,"plain":false,"data":"abcdefghijklmnop"}
{bufferLength:16,plain:true,data:"abcdefghijklmnop"}
{bufferLength:16,plain:false,data:"abcdefghijklmnop"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
{"bufferLength":16,"plain":false,"data":"abcdefghijklmnop"}
===*/

function jsonTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();
    function id(k,v) { return v; }

    function doTest(repl) {
        // JSON, JX, and JC
        print(JSON.stringify(pb, repl));
        print(JSON.stringify(ab, repl));
        print(Duktape.enc('jx', pb, repl));
        print(Duktape.enc('jx', ab, repl));
        print(Duktape.enc('jc', pb, repl));
        print(Duktape.enc('jc', ab, repl));

        // .toJSON() works; with plain buffers treated like ArrayBuffer (which
        // are objects) a .toJSON() check is also necessary, unlike for other
        // primitives types.
        ArrayBuffer.prototype.toJSON = function (k) {
            'use strict';  // must be strict to avoid object coercion for 'this'
            return {
                bufferLength: this.length,
                plain: isPlainBuffer(this),
                data: bufferToString(this)
            };
        };
        print(JSON.stringify(pb, repl));
        print(JSON.stringify(ab, repl));
        print(Duktape.enc('jx', pb, repl));
        print(Duktape.enc('jx', ab, repl));
        print(Duktape.enc('jc', pb, repl));
        print(Duktape.enc('jc', ab, repl));

        delete ArrayBuffer.prototype.toJSON;
    }

    doTest();
    doTest(id);  // force slow path
}

/*===
view test
[object Uint32Array]
4 16 0 4
[object ArrayBuffer] false
1616928864
|60606060616161616262626263636363|
[object Uint32Array]
4 16 0 4
[object ArrayBuffer] true
1616928864
|60606060616161616262626263636363|
===*/

function viewTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();
    var view;
    var i;

    for (i = 0; i < pb.length; i++) {
        pb[i] = 0x60 + (i >> 2);  // endian neutral value
    }
    for (i = 0; i < ab.length; i++) {
        ab[i] = 0x60 + (i >> 2);  // endian neutral value
    }

    // create typedarray on top of plain buffer / ArrayBuffer
    view = new Uint32Array(pb);
    print(Object.prototype.toString.call(view));
    print(view.length, view.byteLength, view.byteOffset, view.BYTES_PER_ELEMENT);
    print(view.buffer, view.buffer === pb);
    print(view[0]);
    print(Duktape.enc('jx', view));
    view = new Uint32Array(ab);
    print(Object.prototype.toString.call(view));
    print(view.length, view.byteLength, view.byteOffset, view.BYTES_PER_ELEMENT);
    print(view.buffer, view.buffer === ab);
    print(view[0]);
    print(Duktape.enc('jx', view));
}

/*===
Object methods
- getPrototypeOf
[object Object] true
[object Object] true
[object Object] true
[object Object] true
- setPrototypeOf
TypeError
undefined
[object ArrayBuffer]
undefined
[object ArrayBuffer]
123
- getOwnPropertyDescriptor
0 {value:97,writable:true,enumerable:false,configurable:false}
0 {value:97,writable:true,enumerable:false,configurable:false}
1 {value:98,writable:true,enumerable:false,configurable:false}
1 {value:98,writable:true,enumerable:false,configurable:false}
2 {value:99,writable:true,enumerable:false,configurable:false}
2 {value:99,writable:true,enumerable:false,configurable:false}
3 {value:100,writable:true,enumerable:false,configurable:false}
3 {value:100,writable:true,enumerable:false,configurable:false}
4 {value:101,writable:true,enumerable:false,configurable:false}
4 {value:101,writable:true,enumerable:false,configurable:false}
5 {value:102,writable:true,enumerable:false,configurable:false}
5 {value:102,writable:true,enumerable:false,configurable:false}
6 {value:103,writable:true,enumerable:false,configurable:false}
6 {value:103,writable:true,enumerable:false,configurable:false}
7 {value:104,writable:true,enumerable:false,configurable:false}
7 {value:104,writable:true,enumerable:false,configurable:false}
8 {value:105,writable:true,enumerable:false,configurable:false}
8 {value:105,writable:true,enumerable:false,configurable:false}
9 {value:106,writable:true,enumerable:false,configurable:false}
9 {value:106,writable:true,enumerable:false,configurable:false}
10 {value:107,writable:true,enumerable:false,configurable:false}
10 {value:107,writable:true,enumerable:false,configurable:false}
11 {value:108,writable:true,enumerable:false,configurable:false}
11 {value:108,writable:true,enumerable:false,configurable:false}
12 {value:109,writable:true,enumerable:false,configurable:false}
12 {value:109,writable:true,enumerable:false,configurable:false}
13 {value:110,writable:true,enumerable:false,configurable:false}
13 {value:110,writable:true,enumerable:false,configurable:false}
14 {value:111,writable:true,enumerable:false,configurable:false}
14 {value:111,writable:true,enumerable:false,configurable:false}
15 {value:112,writable:true,enumerable:false,configurable:false}
15 {value:112,writable:true,enumerable:false,configurable:false}
16 undefined
16 undefined
length {value:16,writable:false,enumerable:false,configurable:false}
length {value:16,writable:false,enumerable:false,configurable:false}
byteLength {value:16,writable:false,enumerable:false,configurable:false}
byteLength {value:16,writable:false,enumerable:false,configurable:false}
byteOffset {value:0,writable:false,enumerable:false,configurable:false}
byteOffset {value:0,writable:false,enumerable:false,configurable:false}
BYTES_PER_ELEMENT {value:1,writable:false,enumerable:false,configurable:false}
BYTES_PER_ELEMENT {value:1,writable:false,enumerable:false,configurable:false}
noSuch undefined
noSuch undefined
- getOwnPropertyNames
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,length,byteLength,byteOffset,BYTES_PER_ELEMENT
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,length,byteLength,byteOffset,BYTES_PER_ELEMENT
- create
1 true 100
255
1 true 100
255
- defineProperty
[object ArrayBuffer]
99
undefined
[object ArrayBuffer]
99
1234
- defineProperties
[object ArrayBuffer]
99
undefined
[object ArrayBuffer]
99
1234
- defineProperty index
TypeError
99
TypeError
99
- defineProperties index
TypeError
99
TypeError
99
- seal
false
[object ArrayBuffer]
false
true
[object ArrayBuffer]
false
- freeze
false
TypeError
false
true
TypeError
true
- preventExtensions
false
[object ArrayBuffer]
false
true
[object ArrayBuffer]
false
- isSealed
true
false
- isFrozen
false
false
- isExtensible
false
true
- keys


===*/

function objectMethodTest() {
    var pb, ab, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    resetValues();
    print('- getPrototypeOf');
    print(Object.getPrototypeOf(pb), Object.getPrototypeOf(pb) === ArrayBuffer.prototype);
    print(Object.getPrototypeOf(ab), Object.getPrototypeOf(ab) === ArrayBuffer.prototype);
    print(pb.__proto__, pb.__proto__ == ArrayBuffer.prototype);
    print(ab.__proto__, ab.__proto__ == ArrayBuffer.prototype);

    // Plain buffer prototype cannot be set; if the new prototype differs
    // from the existing one a TypeError is thrown because the plain buffer
    // is considered non-extensible.
    resetValues();
    print('- setPrototypeOf');
    try {
        print(String(Object.setPrototypeOf(pb, { foo: 123 })));
    } catch (e) {
        print(e.name);
    }
    print(pb.foo);
    try {
        print(String(Object.setPrototypeOf(pb, ArrayBuffer.prototype)));
    } catch (e) {
        print(e);
    }
    print(pb.foo);

    // ArrayBuffer prototype can be set
    try {
        print(Object.setPrototypeOf(ab, { foo: 123 }));
    } catch (e) {
        print(e);
    }
    print(ab.foo);

    resetValues();
    print('- getOwnPropertyDescriptor');
    [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
      'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT', 'noSuch' ].forEach(function (k) {
        print(k, Duktape.enc('jx', Object.getOwnPropertyDescriptor(pb, k)));
        print(k, Duktape.enc('jx', Object.getOwnPropertyDescriptor(ab, k)));
    });

    resetValues();
    print('- getOwnPropertyNames');
    print(Object.getOwnPropertyNames(pb));
    print(Object.getOwnPropertyNames(ab));

    // Object.create takes a prototype argument; if a plain buffer is used as
    // a prototype, it gets object coerced.
    resetValues();
    print('- create');
    t = Object.create(pb);
    print(t.BYTES_PER_ELEMENT, t instanceof ArrayBuffer, t[3]);
    pb[3] = 0xff;  // underlying buffer the same
    print(t[3]);
    t = Object.create(ab);
    print(t.BYTES_PER_ELEMENT, t instanceof ArrayBuffer, t[3]);
    ab[3] = 0xff;
    print(t[3]);

    // Covered by other tests more exhaustively, so just check minimally here.
    // Object.defineProperty() and Object.defineProperties() automatically
    // promote the argument value to an object; if any new properties are
    // established that happens on the temporary object and is then forgotten.
    // This matches how lightfuncs behave right now.

    resetValues();
    print('- defineProperty');
    print(Object.defineProperty(pb, 'newProp', { value: 1234 }));
    print(pb[2]);
    print(pb.newProp);
    print(Object.defineProperty(ab, 'newProp', { value: 1234 }));
    print(ab[2]);
    print(ab.newProp);

    resetValues();
    print('- defineProperties');
    print(Object.defineProperties(pb, { newProp: { value: 1234 } }));
    print(pb[2]);
    print(pb.newProp);
    print(Object.defineProperties(ab, { newProp: { value: 1234 } }));
    print(ab[2]);
    print(ab.newProp);

    // XXX: current limitation, attempt to write indices using
    // Object.defineProperty() or Object.defineProperties() rejected
    // even for ArrayBuffers (and other typed arrays).

    resetValues();
    print('- defineProperty index');
    try {
        print(Object.defineProperty(pb, '2', { value: 1234 }));
    } catch (e) {
        print(e.name);
    }
    print(pb[2]);
    try {
        print(Object.defineProperty(ab, '2', { value: 1234 }));
    } catch (e) {
        print(e.name);
    }
    print(ab[2]);

    resetValues();
    print('- defineProperties index');
    try {
        print(Object.defineProperties(pb, {
            2: { value: 1234 }
        }));
    } catch (e) {
        print(e.name);
    }
    print(pb[2]);
    try {
        print(Object.defineProperties(ab, {
            2: { value: 1234 }
        }));
    } catch (e) {
        print(e.name);
    }
    print(ab[2]);

    resetValues();
    print('- seal');
    print(Object.isExtensible(pb));
    print(String(Object.seal(pb)));  // already sealed, nop
    print(Object.isExtensible(pb));
    print(Object.isExtensible(ab));
    print(String(Object.seal(ab)));
    print(Object.isExtensible(ab));

    // Plain buffer cannot be frozen because the virtual array index properties
    // cannot be made non-writable.  Because the operation is rejected, the object's
    // extensibility flag is not touched.
    resetValues();
    print('- freeze');
    print(Object.isExtensible(pb));
    try {
        print(String(Object.freeze(pb)));
    } catch (e) {
        print(e.name);
    }
    print(Object.isExtensible(pb));
    print(Object.isExtensible(ab));
    try {
        print(String(Object.freeze(ab)));
    } catch (e) {
        print(e.name);
    }
    print(Object.isExtensible(ab));

    resetValues();
    print('- preventExtensions');
    print(Object.isExtensible(pb));
    print(String(Object.preventExtensions(pb)));
    print(Object.isExtensible(pb));
    print(Object.isExtensible(ab));
    print(String(Object.preventExtensions(ab)));
    print(Object.isExtensible(ab));

    resetValues();
    print('- isSealed');
    print(Object.isSealed(pb));
    print(Object.isSealed(ab));
    resetValues();
    print('- isFrozen');
    print(Object.isFrozen(pb));
    print(Object.isFrozen(ab));
    resetValues();
    print('- isExtensible');
    print(Object.isExtensible(pb));
    print(Object.isExtensible(ab));
    resetValues();
    print('- keys');
    print(Object.keys(pb));
    print(Object.keys(ab));
}

/*===
Object.prototype methods
- toString
[object ArrayBuffer]
[object ArrayBuffer]
- toLocaleString
[object ArrayBuffer]
[object ArrayBuffer]
- valueOf
[object ArrayBuffer] false
[object ArrayBuffer] true
- hasOwnProperty
0 true
0 true
1 true
1 true
2 true
2 true
3 true
3 true
4 true
4 true
5 true
5 true
6 true
6 true
7 true
7 true
8 true
8 true
9 true
9 true
10 true
10 true
11 true
11 true
12 true
12 true
13 true
13 true
14 true
14 true
15 true
15 true
16 false
16 false
length true
length true
byteLength true
byteLength true
byteOffset true
byteOffset true
BYTES_PER_ELEMENT true
BYTES_PER_ELEMENT true
noSuch false
noSuch false
- isPrototypeOf
false
false
false
true
- propertyIsEnumerable
false
false
false
false
false
false
false
false
===*/

function objectPrototypeMethodTest() {
    var pb, ab, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    resetValues();
    print('- toString');
    print(pb.toString());
    print(ab.toString());

    resetValues();
    print('- toLocaleString');
    print(pb.toLocaleString());
    print(ab.toLocaleString());

    resetValues();
    print('- valueOf');
    print(pb.valueOf(), pb.valueOf() === pb);  // .valueOf() is Object() coerced now
    print(ab.valueOf(), ab.valueOf() === ab);

    resetValues();
    print('- hasOwnProperty');
    [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
      'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT', 'noSuch' ].forEach(function (k) {
        print(k, pb.hasOwnProperty(k));
        print(k, ab.hasOwnProperty(k));
    });

    resetValues();
    print('- isPrototypeOf');
    print(pb.isPrototypeOf(pb));  // false, no object is its own prototype (unless there's a prototype loop)
    t = Object.create(pb);
    print(pb.isPrototypeOf(t));   // false; Object.create() object coerces plain buffer when using it as prototype
    print(ab.isPrototypeOf(ab));  // false
    t = Object.create(ab);
    print(ab.isPrototypeOf(t));   // true, 'ab' is prototype of 't'

    resetValues();
    print('- propertyIsEnumerable');
    print(pb.propertyIsEnumerable('byteLength'));
    print(pb.propertyIsEnumerable(3));
    print(pb.propertyIsEnumerable('3'));
    print(pb.propertyIsEnumerable('noSuch'));
    print(ab.propertyIsEnumerable('byteLength'));
    print(ab.propertyIsEnumerable(3));
    print(ab.propertyIsEnumerable('3'));
    print(ab.propertyIsEnumerable('noSuch'));
}

/*===
ArrayBuffer methods
- isView
false
false
===*/

function arrayBufferMethodTest() {
    var pb, ab, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    // ArrayBuffer only has .isView()
    print('- isView');
    resetValues();

    print(ArrayBuffer.isView(pb));
    print(ArrayBuffer.isView(ab));
}

/*===
ArrayBuffer.prototype methods
- slice
97
object [object ArrayBuffer]
true
4 0 4 1
99 100 101 102 undefined
255 99
97
object [object ArrayBuffer]
false
4 0 4 1
99 100 101 102 undefined
255 99
- slice stress
done
===*/

function arrayBufferPrototypeMethodTest() {
    var pb, ab, t;
    var i, j;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    // ArrayBuffer.prototype only has .slice()
    print('- slice');
    resetValues();

    t = pb.slice(2, 6);
    print(pb[0]);
    print(typeof t, t);
    print(isPlainBuffer(t));
    print(t.byteLength, t.byteOffset, t.length, t.BYTES_PER_ELEMENT);
    print(t[0], t[1], t[2], t[3], t[4]);
    t[0] = 0xff;
    print(t[0], pb[2]);
    t = ab.slice(2, 6);
    print(ab[0]);
    print(typeof t, t);
    print(isPlainBuffer(t));
    print(t.byteLength, t.byteOffset, t.length, t.BYTES_PER_ELEMENT);
    print(t[0], t[1], t[2], t[3], t[4]);
    t[0] = 0xff;
    print(t[0], ab[2]);

    // Stress test for ArrayBuffer.prototype.slice()
    print('- slice stress');
    resetValues();
    for (i = -3; i < 19; i++) {
        for (j = -3; j < 19; j++) {
            var ps = pb.slice(i, j);
            var as = ab.slice(i, j);
            var px = Duktape.enc('jx', ps);
            var ax = Duktape.enc('jx', as);

            //print(i, j, px, ax);
            if (px !== ax) {
                print('slices differ:', i, j, px, ax);
            }
        }
    }
    print('done');
}

/*===
Duktape methods
- fin
TypeError
TypeError
undefined
undefined
function dummy() {"ecmascript"}
- enc
|6162636465666768696a6b6c6d6e6f70|
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
6162636465666768696a6b6c6d6e6f70
YWJjZGVmZ2hpamtsbW5vcA==
|6162636465666768696a6b6c6d6e6f70|
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
6162636465666768696a6b6c6d6e6f70
YWJjZGVmZ2hpamtsbW5vcA==
- dec
foo
foo
true
string 61626364
abcd
true
false
string 61626364
abcd
true
- compact
[object ArrayBuffer]
true
[object ArrayBuffer]
false
===*/

function duktapeMethodTest() {
    var pb, ab, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    // info(): not relevant
    // act(): not relevant
    // gc(): not relevant

    // fin() fails with TypeError, cannot get/set a finalizer on a plain buffer
    resetValues();
    print('- fin');
    try {
        print(Duktape.fin(pb));
    } catch (e) {
        print(e.name);
    }
    try {
        print(Duktape.fin(pb, function dummy() {}));
        print(Duktape.fin(ab));
    } catch (e) {
        print(e.name);
    }
    try {
        print(Duktape.fin(ab));
    } catch (e) {
        print(e.name);
    }
    try {
        print(Duktape.fin(ab, function dummy() {}));
        print(Duktape.fin(ab));
    } catch (e) {
        print(e.name);
    }

    resetValues();
    print('- enc');
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jc', pb));
    print(Duktape.enc('hex', pb));
    print(Duktape.enc('base64', pb));
    print(Duktape.enc('jx', ab));
    print(Duktape.enc('jc', ab));
    print(Duktape.enc('hex', ab));
    print(Duktape.enc('base64', ab));

    // Duktape.dec() interprets a plain buffer like an input string.
    resetValues();
    print('- dec');
    t = '666f6f';
    pb = createPlainBuffer(6);
    pb[0] = t.charCodeAt(0);
    pb[1] = t.charCodeAt(1);
    pb[2] = t.charCodeAt(2);
    pb[3] = t.charCodeAt(3);
    pb[4] = t.charCodeAt(4);
    pb[5] = t.charCodeAt(5);
    print(bufferToString(Duktape.dec('hex', pb)));  // hex decode '666f6f' to buffer containing 'foo'
    ab = new ArrayBuffer(6);
    ab[0] = t.charCodeAt(0);
    ab[1] = t.charCodeAt(1);
    ab[2] = t.charCodeAt(2);
    ab[3] = t.charCodeAt(3);
    ab[4] = t.charCodeAt(4);
    ab[5] = t.charCodeAt(5);
    print(bufferToString(Duktape.dec('hex', ab)));

    // Duktape.dec() outputs a plain buffer.
    pb = createPlainBuffer(4);
    pb[0] = 0x61;
    pb[1] = 0x62;
    pb[2] = 0x63;
    pb[3] = 0x64;
    print(isPlainBuffer(pb));
    t = Duktape.enc('hex', pb);
    print(typeof t, t);
    t = Duktape.dec('hex', t);
    print(bufferToString(t));
    print(isPlainBuffer(t));
    ab = new ArrayBuffer(4);
    ab[0] = 0x61;
    ab[1] = 0x62;
    ab[2] = 0x63;
    ab[3] = 0x64;
    print(isPlainBuffer(ab));
    t = Duktape.enc('hex', ab);
    print(typeof t, t);
    t = Duktape.dec('hex', t);
    print(bufferToString(t));
    print(isPlainBuffer(t));

    // compact() is a no-op and returns input value
    resetValues();
    print('- compact');
    print(String(Duktape.compact(pb)));
    print(isPlainBuffer(Duktape.compact(pb)));
    print(String(Duktape.compact(ab)));
    print(isPlainBuffer(Duktape.compact(ab)));
}

/*===
Node.js Buffer methods
- compare
0
0
0
0
0
0
-1
1
- isBuffer
false
false
true
- byteLength
20
20
16
- concat
{type:"Buffer",data:[255,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,254,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,253,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
===*/

function nodejsBufferMethodTest() {
    var pb, ab, nb, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
        nb = createNodejsBuffer();
    }

    /* While Node.js Buffer methods are intended for Buffer instances only,
     * they also work for ArrayBuffers and plain buffers in Duktape.
     */

    resetValues();
    print('- compare');
    print(Buffer.compare(pb, pb));
    print(Buffer.compare(pb, ab));
    print(Buffer.compare(pb, nb));
    print(Buffer.compare(ab, pb));
    print(Buffer.compare(ab, ab));
    print(Buffer.compare(ab, nb));
    pb[0] = 1;
    print(Buffer.compare(pb, ab));
    pb[0] = 255;
    print(Buffer.compare(pb, ab));

    resetValues();
    print('- isBuffer');
    print(Buffer.isBuffer(pb));
    print(Buffer.isBuffer(ab));
    print(Buffer.isBuffer(nb));

    // XXX: for now follows old Node.js behavior where Buffer.byteLength()
    // would string coerce its argument; ArrayBuffer (usually) coerces to
    // '[object ArrayBuffer]' whose length is 20, Node.js Buffer coerces
    // (currently) to a string with the same bytes as the buffer so it comes
    // out more usefully as 16 here.
    resetValues();
    print('- byteLength');
    print(Buffer.byteLength(pb));
    print(Buffer.byteLength(ab));
    print(Buffer.byteLength(nb));

    resetValues();
    print('- concat');
    pb[0] = 255;
    ab[0] = 254;
    nb[0] = 253;
    t = Buffer.concat([ pb, ab, nb ]);
    print(Duktape.enc('jx', t));
}

/*===
Node.js Buffer.prototype methods
- toJSON
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
- compare
equal
0
0
0
0
0
0
0
0
0
not equal
0
1
-1
-1
0
-1
1
1
0
- equals
equal
true
true
true
true
true
true
true
true
true
not equal
true
false
false
false
true
false
false
false
true
- fill
[object ArrayBuffer]
|6162111111111111116a6b6c6d6e6f70|
[object ArrayBuffer]
|6162111111111111116a6b6c6d6e6f70|
- copy, source plain buffer, target Node.js Buffer
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
16
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
2
{type:"Buffer",data:[97,98,99,111,112,102,103,104,105,106,107,108,109,110,111,112]}
- copy, source ArrayBuffer, target Node.js Buffer
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
16
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
2
{type:"Buffer",data:[97,98,99,111,112,102,103,104,105,106,107,108,109,110,111,112]}
- copy, source plain buffer, target plain buffer
|0000000000000000000000000000000000000000000000000000000000000000|
16
|6162636465666768696a6b6c6d6e6f7000000000000000000000000000000000|
2
|6162636465666768696a6b6c6d6e6f70000000000000006f7000000000000000|
- copy, source ArrayBuffer, target plain buffer
|0000000000000000000000000000000000000000000000000000000000000000|
16
|6162636465666768696a6b6c6d6e6f7000000000000000000000000000000000|
2
|6162636465666768696a6b6c6d6e6f70000000000000006f7000000000000000|
- slice
object cdefg false
|6162636465666768696a6b6c6d6e6f70|
{type:"Buffer",data:[99,100,101,102,103]}
|6162636465ffee68696a6b6c6d6e6f70|
{type:"Buffer",data:[99,100,101,255,238]}
- write
|6162636465666768696a6b6c6d6e6f70| abcdefghijklmnop
3
|616263464f4f6768696a6b6c6d6e6f70| abcFOOghijklmnop
2
|616263464f4f6742416a6b6c6d6e6f70| abcFOOgBAjklmnop
|6162636465666768696a6b6c6d6e6f70| abcdefghijklmnop
3
|616263464f4f6768696a6b6c6d6e6f70| abcFOOghijklmnop
2
|616263464f4f6742416a6b6c6d6e6f70| abcFOOgBAjklmnop
- readUInt16LE
25442
25442
- writeUInt32LE
|6162636465666768696a6b6c6d6e6f70|
11
|61626364656667efbeadde6c6d6e6f70|
|6162636465666768696a6b6c6d6e6f70|
11
|61626364656667efbeadde6c6d6e6f70|
- toString
abcdefghijklmnop
abcdefghijklmnop
===*/

function nodejsBufferPrototypeMethodTest() {
    var pb, ab, nb, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
        nb = createNodejsBuffer();
    }

    /* While Node.js Buffer.prototype methods are intended for Buffer
     * instances only, they also work for ArrayBuffers and plain buffers
     * in Duktape.
     */

    resetValues();
    print('- toJSON');
    print(Duktape.enc('jx', Buffer.prototype.toJSON.call(pb)));
    print(Duktape.enc('jx', Buffer.prototype.toJSON.call(ab)));
    print(Duktape.enc('jx', Buffer.prototype.toJSON.call(nb)));

    resetValues();
    print('- compare');
    print('equal');
    print(Buffer.prototype.compare.call(pb, pb));
    print(Buffer.prototype.compare.call(pb, ab));
    print(Buffer.prototype.compare.call(pb, nb));
    print(Buffer.prototype.compare.call(ab, pb));
    print(Buffer.prototype.compare.call(ab, ab));
    print(Buffer.prototype.compare.call(ab, nb));
    print(Buffer.prototype.compare.call(nb, pb));
    print(Buffer.prototype.compare.call(nb, ab));
    print(Buffer.prototype.compare.call(nb, nb));
    print('not equal');
    pb[0] = 100; pb[1] = 100;
    ab[0] = 100; ab[1] = 99;
    nb[0] = 100; nb[1] = 101;
    print(Buffer.prototype.compare.call(pb, pb));
    print(Buffer.prototype.compare.call(pb, ab));
    print(Buffer.prototype.compare.call(pb, nb));
    print(Buffer.prototype.compare.call(ab, pb));
    print(Buffer.prototype.compare.call(ab, ab));
    print(Buffer.prototype.compare.call(ab, nb));
    print(Buffer.prototype.compare.call(nb, pb));
    print(Buffer.prototype.compare.call(nb, ab));
    print(Buffer.prototype.compare.call(nb, nb));

    resetValues();
    print('- equals');
    print('equal');
    print(Buffer.prototype.equals.call(pb, pb));
    print(Buffer.prototype.equals.call(pb, ab));
    print(Buffer.prototype.equals.call(pb, nb));
    print(Buffer.prototype.equals.call(ab, pb));
    print(Buffer.prototype.equals.call(ab, ab));
    print(Buffer.prototype.equals.call(ab, nb));
    print(Buffer.prototype.equals.call(nb, pb));
    print(Buffer.prototype.equals.call(nb, ab));
    print(Buffer.prototype.equals.call(nb, nb));
    print('not equal');
    pb[0] = 100; pb[1] = 100;
    ab[0] = 100; ab[1] = 99;
    nb[0] = 100; nb[1] = 101;
    print(Buffer.prototype.equals.call(pb, pb));
    print(Buffer.prototype.equals.call(pb, ab));
    print(Buffer.prototype.equals.call(pb, nb));
    print(Buffer.prototype.equals.call(ab, pb));
    print(Buffer.prototype.equals.call(ab, ab));
    print(Buffer.prototype.equals.call(ab, nb));
    print(Buffer.prototype.equals.call(nb, pb));
    print(Buffer.prototype.equals.call(nb, ab));
    print(Buffer.prototype.equals.call(nb, nb));

    resetValues();
    print('- fill');
    print(Buffer.prototype.fill.call(pb, 0x11, 2, 9));
    print(Duktape.enc('jx', pb));
    print(Buffer.prototype.fill.call(ab, 0x11, 2, 9));
    print(Duktape.enc('jx', ab));

    resetValues();
    print('- copy, source plain buffer, target Node.js Buffer');
    print(Duktape.enc('jx', nb));
    print(Buffer.prototype.copy.call(pb, nb));
    print(Duktape.enc('jx', nb));
    print(Buffer.prototype.copy.call(pb, nb, 3 /*targetStart*/, 14 /*sourceStart*/, 16 /*sourceEnd*/));
    print(Duktape.enc('jx', nb));

    resetValues();
    print('- copy, source ArrayBuffer, target Node.js Buffer');
    print(Duktape.enc('jx', nb));
    print(Buffer.prototype.copy.call(ab, nb));
    print(Duktape.enc('jx', nb));
    print(Buffer.prototype.copy.call(ab, nb, 3 /*targetStart*/, 14 /*sourceStart*/, 16 /*sourceEnd*/));
    print(Duktape.enc('jx', nb));

    resetValues();
    print('- copy, source plain buffer, target plain buffer');
    t = createPlainBuffer(32);
    print(Duktape.enc('jx', t));
    print(Buffer.prototype.copy.call(pb, t));
    print(Duktape.enc('jx', t));
    print(Buffer.prototype.copy.call(pb, t, 23 /*targetStart*/, 14 /*sourceStart*/, 16 /*sourceEnd*/));
    print(Duktape.enc('jx', t));

    resetValues();
    print('- copy, source ArrayBuffer, target plain buffer');
    t = createPlainBuffer(32);
    print(Duktape.enc('jx', t));
    print(Buffer.prototype.copy.call(ab, t));
    print(Duktape.enc('jx', t));
    print(Buffer.prototype.copy.call(ab, t, 23 /*targetStart*/, 14 /*sourceStart*/, 16 /*sourceEnd*/));
    print(Duktape.enc('jx', t));

    // .slice() for a plain buffer returns a Node.js Buffer (an Uint8Array
    // inheriting from Buffer.prototype) because a plain buffer cannot
    // represent an offset/length slice (view) into the argument
    resetValues();
    print('- slice');
    t = Buffer.prototype.slice.call(pb, 2, 7);
    print(typeof t, t, isPlainBuffer(t));
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', t));
    t[3] = 0xff;  // demonstrate same underlying buffer
    pb[6] = 0xee;
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', t));

    resetValues();
    print('- write');
    print(Duktape.enc('jx', pb), bufferToString(pb));
    print(Buffer.prototype.write.call(pb, 'FOO', 3));
    print(Duktape.enc('jx', pb), bufferToString(pb));
    print(Buffer.prototype.write.call(pb, 'BARQUUX', 7, 2));
    print(Duktape.enc('jx', pb), bufferToString(pb));
    print(Duktape.enc('jx', ab), bufferToString(ab));
    print(Buffer.prototype.write.call(ab, 'FOO', 3));
    print(Duktape.enc('jx', ab), bufferToString(ab));
    print(Buffer.prototype.write.call(ab, 'BARQUUX', 7, 2));
    print(Duktape.enc('jx', ab), bufferToString(ab));

    // Spot check one read method
    resetValues();
    print('- readUInt16LE');
    print(Buffer.prototype.readUInt16LE.call(pb, 1));  // 62 63 in memory -> 0x6362
    print(Buffer.prototype.readUInt16LE.call(ab, 1));

    // Spot check one write method
    resetValues();
    print('- writeUInt32LE');
    print(Duktape.enc('jx', pb));
    print(Buffer.prototype.writeUInt32LE.call(pb, 0xdeadbeef, 7));
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', ab));
    print(Buffer.prototype.writeUInt32LE.call(ab, 0xdeadbeef, 7));
    print(Duktape.enc('jx', ab));

    resetValues();
    print('- toString');
    print(Buffer.prototype.toString.call(pb));
    print(Buffer.prototype.toString.call(ab));
}

/*===
TypedArray.prototype methods
- set
|6162636465666768696a6b6c6d6e6f70|
|6162636465660102036a6b6c6d6e6f70|
|6162636465660102036a6b6cfffefd70|
|61fefd6465660102036a6b6cfffefd70|
|6162636465666768696a6b6c6d6e6f70|
|6162636465660102036a6b6c6d6e6f70|
|6162636465660102036a6b6cfffefd70|
|61fefd6465660102036a6b6cfffefd70|
- subarray
object [object ArrayBuffer] false
|65666768696a6b6c6d|
4 9 1
|65ff6768696a6b6c6d|
object [object ArrayBuffer] false
|65666768696a6b6c6d|
4 9 1
|65ff6768696a6b6c6d|
object [object ArrayBuffer] false
|6162636465ff6768696a6b6c6d6e6f70|
0 16 1
|6162636465ff6768696a6b6c6d6e6f70|
===*/

function typedArrayPrototypeMethodTest() {
    var pb, ab, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    // typedarray.prototype.set
    resetValues();
    print('- set');
    print(Duktape.enc('jx', pb));
    Uint16Array.prototype.set.call(pb, [ 1, 2, 3 ], 6);  // as 'this'
    print(Duktape.enc('jx', pb));
    t = createPlainBuffer(3);
    t[0] = 0xff;
    t[1] = 0xfe;
    t[2] = 0xfd;
    Uint16Array.prototype.set.call(pb, t, 12);  // as 'this' and source
    print(Duktape.enc('jx', pb));
    Uint16Array.prototype.set.call(pb, t.slice(1), 1);  // as 'this' and source, sliced
    print(Duktape.enc('jx', pb));

    print(Duktape.enc('jx', ab));
    Uint16Array.prototype.set.call(ab, [ 1, 2, 3 ], 6);
    print(Duktape.enc('jx', ab));
    t = createPlainBuffer(3);
    t[0] = 0xff;
    t[1] = 0xfe;
    t[2] = 0xfd;
    Uint16Array.prototype.set.call(ab, t, 12);
    print(Duktape.enc('jx', ab));
    Uint16Array.prototype.set.call(ab, t.slice(1), 1);
    print(Duktape.enc('jx', ab));

    // typedarray.prototype.subarray() creates a new view into an existing
    // backing buffer.  When the typedarray method is forcibly called with
    // a plain buffer as the 'this' binding (which is non-standard behavior
    // anyway) we can't return a plain buffer result because slice information
    // cannot be recorded by a plain buffer.  Instead, the call behaves as if
    // the input buffer was coerced into a full ArrayBuffer object, and an
    // ArrayBuffer result is resulted (backed by the input buffer).  This is
    // the case for consistency even when the indices span [0,length[.

    resetValues();
    print('- subarray');

    t = Uint8Array.prototype.subarray.call(pb, 4, -3);
    print(typeof t, t, isPlainBuffer(t));
    print(Duktape.enc('jx', t));
    print(t.byteOffset, t.byteLength, t.BYTES_PER_ELEMENT);
    pb[5] = 0xff;  // demonstrate pb and t share same storage
    print(Duktape.enc('jx', t));

    t = Uint8Array.prototype.subarray.call(ab, 4, -3);
    print(typeof t, t, isPlainBuffer(t));
    print(Duktape.enc('jx', t));
    print(t.byteOffset, t.byteLength, t.BYTES_PER_ELEMENT);
    ab[5] = 0xff;
    print(Duktape.enc('jx', t));

    t = Uint8Array.prototype.subarray.call(pb);  // full index span, i.e. no actual view offset/limit
    print(typeof t, t, isPlainBuffer(t));
    print(Duktape.enc('jx', t));
    print(t.byteOffset, t.byteLength, t.BYTES_PER_ELEMENT);
    pb[5] = 0xff;  // demonstrate pb and t share same storage
    print(Duktape.enc('jx', t));
}

/*===
DataView.prototype methods
- getUint16
|6162636465666768696a6b6c6d6e6f70|
26215
26470
|6162636465666768696a6b6c6d6e6f70|
26215
26470
- setInt32
|6162636465666768696a6b6c6d6e6f70|
undefined
|616263deadbeef68696a6b6c6d6e6f70|
undefined
|616263deadbeef68efbeadde6d6e6f70|
|6162636465666768696a6b6c6d6e6f70|
undefined
|616263deadbeef68696a6b6c6d6e6f70|
undefined
|616263deadbeef68efbeadde6d6e6f70|
===*/

function dataViewPrototypeMethodTest() {
    var pb, ab, t;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    // Spot check one get method
    resetValues();
    print('- getUint16');
    print(Duktape.enc('jx', pb));
    print(DataView.prototype.getUint16.call(pb, 5, false));  // 66 67 -> 0x6667 big endian
    print(DataView.prototype.getUint16.call(pb, 5, true));   // 66 67 -> 0x6766 little endian
    print(Duktape.enc('jx', ab));
    print(DataView.prototype.getUint16.call(ab, 5, false));
    print(DataView.prototype.getUint16.call(ab, 5, true));

    // Spot check one set method
    resetValues();
    print('- setInt32');
    print(Duktape.enc('jx', pb));
    print(DataView.prototype.setInt32.call(pb, 3, 0xdeadbeef, false));
    print(Duktape.enc('jx', pb));
    print(DataView.prototype.setInt32.call(pb, 8, 0xdeadbeef, true));
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', ab));
    print(DataView.prototype.setInt32.call(ab, 3, 0xdeadbeef, false));
    print(Duktape.enc('jx', ab));
    print(DataView.prototype.setInt32.call(ab, 8, 0xdeadbeef, true));
    print(Duktape.enc('jx', ab));
}

/*===
this coercion test
object true
object false
object false
object false
===*/

function thisCoercionTest() {
    var pb = createPlain();
    var ab = createArrayBuffer();

    function myStrict() {
        'use strict';
        print(typeof this, isPlainBuffer(this));
    }
    myStrict.call(pb);
    myStrict.call(ab);

    // The 'this' binding of a non-strict function is not further coerced if
    // it is an object.  While plain buffers mimic ArrayBuffer objects, they
    // are ToObject() coerced when the call target is a non-strict function.
    // This matches lightfunc 'this' binding behavior in Duktape 2.x.

    function myNonStrict() {
        print(typeof this, isPlainBuffer(this));
    }
    myNonStrict.call(pb);
    myNonStrict.call(ab);
}

/*===
proxy test
Proxy get for key: BYTES_PER_ELEMENT
1
Proxy get for key: 0
97
Proxy get for key: slice
function
123
Proxy get for key: BYTES_PER_ELEMENT
1
Proxy get for key: 0
97
Proxy get for key: slice
function
123
get
this: object false [object Object]
target: object false [object Object]
key: string foo
proxy.foo: bar
get
this: object false [object Object]
target: object false [object Object]
key: string nonExistent
proxy.nonExistent: dummy
get
this: object false [object Object]
target: object false [object Object]
key: string foo
proxy.foo: bar
get
this: object false [object Object]
target: object false [object Object]
key: string nonExistent
proxy.nonExistent: dummy
===*/

function proxyTest() {
    var pb;
    var ab;
    var proxy;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    // Plain buffer is accepted as a proxy target, but object coerced.
    resetValues();
    proxy = new Proxy(pb, {
        get: function get(targ, key, receiver) {
            print('Proxy get for key:', key);
            return targ[key];
        }
    });
    print(proxy.BYTES_PER_ELEMENT);
    print(proxy[0]);
    print(typeof proxy.slice);
    proxy[0] = 123;
    print(pb[0]);

    proxy = new Proxy(ab, {
        get: function get(targ, key, receiver) {
            print('Proxy get for key:', key);
            return targ[key];
        }
    });
    print(proxy.BYTES_PER_ELEMENT);
    print(proxy[0]);
    print(typeof proxy.slice);
    proxy[0] = 123;
    print(ab[0]);

    // Proxy as a handler value; ES6 requires it must be an Object and a
    // plain buffer pretends to be an object.  The traps must be placed in
    // ArrayBuffer.prototype for it to actually work - so this is not a very
    // useful thing in practice.  Currently Proxy will just coerce the plain
    // buffer to a full ArrayBuffer silently.

    ArrayBuffer.prototype.get = function (target, key) {
        print('get');
        print('this:', typeof this, isPlainBuffer(target), target);
        print('target:', typeof target, isPlainBuffer(target), target);
        print('key:', typeof key, key);
        return target[key] || 'dummy';  // passthrough
    };

    proxy = new Proxy({ foo: 'bar' }, pb);
    print('proxy.foo:', proxy.foo);
    print('proxy.nonExistent:', proxy.nonExistent);

    proxy = new Proxy({ foo: 'bar' }, ab);
    print('proxy.foo:', proxy.foo);
    print('proxy.nonExistent:', proxy.nonExistent);

    delete ArrayBuffer.prototype.get;

}

/*===
misc test
0
0
4 abcd
4 abcd
4 abcd
4 abcd
4 [object Uint32Array]
102 1717986918
object false
119 2004318071
4 [object Uint32Array]
102 1717986918
object true
119 2004318071
16 [object DataView]
97 24930
object false
119 30562
16 [object DataView]
97 24930
object true
119 30562
read using [object ArrayBuffer] key
read using [object ArrayBuffer] key
read using [Overridden] key
read using [Overridden] key
/[object ArrayBuffer]/
true
true
false
/[object ArrayBuffer]/
true
true
false
/foox+/
false
false
false
false
true
/foox+/
false
false
false
false
true
true
true
false
false
Error
Error
Error: fake message
Error: fake message
===*/

function miscTest() {
    var pb;
    var ab;
    var t;
    var i;

    function resetValues() {
        pb = createPlain();
        ab = createArrayBuffer();
    }

    // ArrayBuffer constructor no longer handles a plain buffer specially
    // as in Duktape 1.x (which created a Node.js Buffer with the same
    // underlying plain buffer)  Instead, the argument is ToNumber() coerced
    // ultimately resulting in zero, and the result is a zero length ArrayBuffer.
    t = new ArrayBuffer(createPlainBuffer(4));
    print(t.length);
    t = new ArrayBuffer(new ArrayBuffer(4));
    print(t.length);

    // Similarly, Node.js Buffer constructor no longer special cases plain
    // buffer.  Instead, a plain buffer is treated like ArrayBuffer or any
    // other object: its .length is read, and index properties are coerced
    // to form a fresh buffer with matching .length.
    pb = createPlainBuffer(4);
    pb[0] = 0x61; pb[1] = 0x62; pb[2] = 0x63; pb[3] = 0x64;
    ab = new ArrayBuffer(4);
    ab[0] = 0x61; ab[1] = 0x62; ab[2] = 0x63; ab[3] = 0x64;
    t = new Buffer(pb);
    print(t.length, t);
    pb[0] = 0x69;  // demonstrate independent backing buffer
    print(t.length, t);
    t = new Buffer(ab);
    print(t.length, t);
    ab[0] = 0x69;
    print(t.length, t);

    // Typed array constructor coerces a plain buffer into an actual ArrayBuffer
    // and uses it for typedArray.buffer.  The underlying buffer is the same.
    pb = createPlainBuffer(16);
    for (i = 0; i < pb.length; i++) { pb[i] = 0x66; }  // endian neutral
    ab = new ArrayBuffer(16);
    for (i = 0; i < ab.length; i++) { ab[i] = 0x66; }
    t = new Uint32Array(pb);
    print(t.length, t);
    print(pb[0], t[0]);
    print(typeof t.buffer, t.buffer === pb);
    t[0] = 0x77777777;  // endian neutral; demonstrate same underlying buffer
    print(pb[0], t[0]);
    t = new Uint32Array(ab);
    print(t.length, t);
    print(ab[0], t[0]);
    print(typeof t.buffer, t.buffer === ab);
    t[0] = 0x77777777;  // endian neutral; demonstrate same underlying buffer
    print(ab[0], t[0]);

    // DataView now accepts a plain buffer like an ArrayBuffer.  Duktape 1.x
    // DataView would reject a plain buffer.
    resetValues();
    t = new DataView(pb);
    print(t.length, t);  // .length is a custom DataView property in Duktape
    print(pb[0], t.getUint16(0));
    print(typeof t.buffer, t.buffer === pb);
    pb[0] = 0x77;  // demonstrate same underlying buffer
    print(pb[0], t.getUint16(0));
    t = new DataView(ab);
    print(t.length, t);
    print(ab[0], t.getUint16(0));
    print(typeof t.buffer, t.buffer === ab);
    ab[0] = 0x77;  // demonstrate same underlying buffer
    print(ab[0], t.getUint16(0));

    // When a plain buffer (or an ArrayBuffer) is used as a property key,
    // they get coerced to a string first, usually '[object ArrayBuffer]'.
    resetValues();
    var obj = {
        '[object ArrayBuffer]': 'read using [object ArrayBuffer] key',
        '[Overridden]': 'read using [Overridden] key'
    };
    print(obj[pb]);
    print(obj[ab]);
    ArrayBuffer.prototype.toString = function () { return '[Overridden]'; };
    print(obj[pb]);
    print(obj[ab]);
    delete ArrayBuffer.prototype.toString;

    // Create a RegExp whose pattern is a plain buffer.  The buffer argument
    // gets string coerced, usually to '[object ArrayBuffer]' which is a
    // character class match.
    resetValues();
    var re = new RegExp(pb);
    print(re);
    print(re.test('ABBA'));   // 'A' matches
    print(re.test('xyzzy'));  // 'y' matches
    print(re.test('q'));  // no match
    var re = new RegExp(ab);
    print(re);
    print(re.test('ABBA'));
    print(re.test('xyzzy'));
    print(re.test('q'));
    ArrayBuffer.prototype.toString = function () { return 'foox+'; };
    var re = new RegExp(pb);
    print(re);
    print(re.test('ABBA'));   // no match with /foox+/ pattern
    print(re.test('xyzzy'));
    print(re.test('q'));
    print(re.test('foo'));
    print(re.test('fooxxx'));
    var re = new RegExp(ab);
    print(re);
    print(re.test('ABBA'));
    print(re.test('xyzzy'));
    print(re.test('q'));
    print(re.test('foo'));
    print(re.test('fooxxx'));
    delete ArrayBuffer.prototype.toString;

    // Plain buffer as a RegExp is string coerced, usually to '[object ArrayBuffer]'.
    resetValues();
    var re = /ArrayBuffer/;
    print(re.test(pb));
    print(re.test(ab));
    ArrayBuffer.prototype.toString = function () { return 'something else'; };
    print(re.test(pb));
    print(re.test(ab));
    delete ArrayBuffer.prototype.toString;

    // Error.prototype.toString(), has lightfunc handling and also plain buffer
    // handling.  Not much of a practical difference.
    resetValues();
    print(Error.prototype.toString.call(pb));
    print(Error.prototype.toString.call(ab));
    ArrayBuffer.prototype.message = 'fake message';
    print(Error.prototype.toString.call(pb));
    print(Error.prototype.toString.call(ab));
    delete ArrayBuffer.prototype.message;
}

try {
    print('basic test');
    basicTest();

    print('property test');
    propertyTest();

    print('enumeration test');
    enumerationTest();

    print('read/write coercion test');
    readWriteCoercionTest();

    print('operator test');
    operatorTest();

    print('coercion test');
    coercionTest();

    print('json test');
    jsonTest();

    print('view test');
    viewTest();

    print('Object methods');
    objectMethodTest();

    print('Object.prototype methods');
    objectPrototypeMethodTest();

    print('ArrayBuffer methods');
    arrayBufferMethodTest();

    print('ArrayBuffer.prototype methods');
    arrayBufferPrototypeMethodTest();

    print('Duktape methods');
    duktapeMethodTest();

    print('Node.js Buffer methods');
    nodejsBufferMethodTest();

    print('Node.js Buffer.prototype methods');
    nodejsBufferPrototypeMethodTest();

    print('TypedArray.prototype methods');
    typedArrayPrototypeMethodTest();

    print('DataView.prototype methods');
    dataViewPrototypeMethodTest();

    print('this coercion test');
    thisCoercionTest();

    print('proxy test');
    proxyTest();

    print('misc test');
    miscTest();
} catch (e) {
    print(e.stack || e);
}
