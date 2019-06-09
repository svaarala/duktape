/*@include util-buffer.js@*/

/*===
Object methods
- getPrototypeOf
[object Object] true
[object Object] true
- setPrototypeOf
TypeError
undefined
[object Uint8Array]
undefined
- getOwnPropertyDescriptor
0 {value:97,writable:true,enumerable:true,configurable:false}
1 {value:98,writable:true,enumerable:true,configurable:false}
2 {value:99,writable:true,enumerable:true,configurable:false}
3 {value:100,writable:true,enumerable:true,configurable:false}
4 {value:101,writable:true,enumerable:true,configurable:false}
5 {value:102,writable:true,enumerable:true,configurable:false}
6 {value:103,writable:true,enumerable:true,configurable:false}
7 {value:104,writable:true,enumerable:true,configurable:false}
8 {value:105,writable:true,enumerable:true,configurable:false}
9 {value:106,writable:true,enumerable:true,configurable:false}
10 {value:107,writable:true,enumerable:true,configurable:false}
11 {value:108,writable:true,enumerable:true,configurable:false}
12 {value:109,writable:true,enumerable:true,configurable:false}
13 {value:110,writable:true,enumerable:true,configurable:false}
14 {value:111,writable:true,enumerable:true,configurable:false}
15 {value:112,writable:true,enumerable:true,configurable:false}
16 undefined
length {value:16,writable:false,enumerable:false,configurable:false}
byteLength undefined
byteOffset undefined
BYTES_PER_ELEMENT undefined
buffer undefined
noSuch undefined
- getOwnPropertyNames
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,length
- create
1 true 100
255
- defineProperty
false
[object Uint8Array]
99
undefined
- defineProperties
false
[object Uint8Array]
99
undefined
- defineProperty index
TypeError
99
- defineProperties index
TypeError
99
- seal
false
[object Uint8Array]
false
- freeze
false
TypeError
false
- preventExtensions
false
[object Uint8Array]
false
- isSealed
true
- isFrozen
false
- isExtensible
false
- keys
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
===*/

function objectMethodTest() {
    var pb, ab, t;

    function resetValues() {
        pb = createPlainBuffer('abcdefghijklmnop');
    }

    resetValues();
    print('- getPrototypeOf');
    print(Object.getPrototypeOf(pb), Object.getPrototypeOf(pb) === Uint8Array.prototype);
    print(pb.__proto__, pb.__proto__ == Uint8Array.prototype);

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
        print(String(Object.setPrototypeOf(pb, Uint8Array.prototype)));
    } catch (e) {
        print(e);
    }
    print(pb.foo);

    resetValues();
    print('- getOwnPropertyDescriptor');
    [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
      'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT', 'buffer', 'noSuch' ].forEach(function (k) {
        print(k, Duktape.enc('jx', Object.getOwnPropertyDescriptor(pb, k)));
    });

    resetValues();
    print('- getOwnPropertyNames');
    print(Object.getOwnPropertyNames(pb));

    // Object.create takes a prototype argument; if a plain buffer is used as
    // a prototype, it gets object coerced.
    resetValues();
    print('- create');
    t = Object.create(pb);
    print(t.BYTES_PER_ELEMENT, t instanceof Uint8Array, t[3]);
    pb[3] = 0xff;  // underlying buffer the same
    print(t[3]);

    // Covered by other tests more exhaustively, so just check minimally here.
    // Object.defineProperty() and Object.defineProperties() automatically
    // promote the argument value to an object; if any new properties are
    // established that happens on the temporary object which is then forgotten.
    // This matches how lightfuncs behave right now.

    resetValues();
    print('- defineProperty');
    var ret = Object.defineProperty(pb, 'newProp', { value: 1234 });
    print(pb === ret);  // no match because of upgrade
    print(Object.prototype.toString.call(ret));
    print(pb[2]);
    print(pb.newProp);

    resetValues();
    print('- defineProperties');
    var ret = Object.defineProperties(pb, { newProp: { value: 1234 } });
    print(pb === ret);  // no match because of upgrade
    print(Object.prototype.toString.call(ret));
    print(pb[2]);
    print(pb.newProp);

    // XXX: current limitation, attempt to write indices using
    // Object.defineProperty() or Object.defineProperties() rejected
    // even for typed array objects.

    resetValues();
    print('- defineProperty index');
    try {
        print(Object.defineProperty(pb, '2', { value: 1234 }));
    } catch (e) {
        print(e.name);
    }
    print(pb[2]);

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

    resetValues();
    print('- seal');
    print(Object.isExtensible(pb));
    print(String(Object.seal(pb)));  // already sealed, nop
    print(Object.isExtensible(pb));

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

    resetValues();
    print('- preventExtensions');
    print(Object.isExtensible(pb));
    print(String(Object.preventExtensions(pb)));
    print(Object.isExtensible(pb));

    resetValues();
    print('- isSealed');
    print(Object.isSealed(pb));
    resetValues();
    print('- isFrozen');
    print(Object.isFrozen(pb));
    resetValues();
    print('- isExtensible');
    print(Object.isExtensible(pb));
    resetValues();
    print('- keys');
    print(Object.keys(pb));
}

try {
    print('Object methods');
    objectMethodTest();
} catch (e) {
    print(e.stack || e);
}
