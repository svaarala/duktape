/*
 *  In Duktape 1.x getting a slice/subarray of a buffer object would copy the
 *  internal prototype reference to the result object.  This differs from the
 *  required standard behavior and behavior of other engines: the result object
 *  should have the "default prototype" matching the class of the result.  The
 *  behavior was fixed in 2.x.
 */

/*===
Uint8Array
123 undefined
undefined function
false true
Uint8ClampedArray
123 undefined
undefined function
false true
Int8Array
123 undefined
undefined function
false true
Uint16Array
123 undefined
undefined function
false true
Int16Array
123 undefined
undefined function
false true
Uint32Array
123 undefined
undefined function
false true
Int32Array
123 undefined
undefined function
false true
Float32Array
123 undefined
undefined function
false true
Float64Array
123 undefined
undefined function
false true
===*/

function test() {
    var a, b;

    // original prototype objects
    var u8p = Uint8Array.prototype;
    var u8cp = Uint8ClampedArray.prototype;
    var i8p = Int8Array.prototype;
    var u16p = Uint16Array.prototype;
    var i16p = Int16Array.prototype;
    var u32p = Uint32Array.prototype;
    var i32p = Int32Array.prototype;
    var f32p = Float32Array.prototype;
    var f64p = Float64Array.prototype;

    Int16Array.prototype = {};  // overwrite; not used for new objects (original prototype is used)

    var dummy = { foo: 123 };

    print('Uint8Array');
    a = new Uint8Array(16);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = u8p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === u8p, Object.getPrototypeOf(b) === u8p);

    print('Uint8ClampedArray');
    a = new Uint8ClampedArray(16);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = u8cp.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === u8cp, Object.getPrototypeOf(b) === u8cp);

    print('Int8Array');
    a = new Int8Array(16);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = i8p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === i8p, Object.getPrototypeOf(b) === i8p);

    print('Uint16Array');
    a = new Uint16Array(16);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = u16p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === u16p, Object.getPrototypeOf(b) === u16p);

    print('Int16Array');
    a = new Int16Array(16);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = i16p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === i16p, Object.getPrototypeOf(b) === i16p);

    print('Uint32Array');
    a = new Uint32Array(32);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = u32p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === u32p, Object.getPrototypeOf(b) === u32p);

    print('Int32Array');
    a = new Int32Array(32);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = i32p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === i32p, Object.getPrototypeOf(b) === i32p);

    print('Float32Array');
    a = new Float32Array(32);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = f32p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === f32p, Object.getPrototypeOf(b) === f32p);

    print('Float64Array');
    a = new Float64Array(32);
    Object.setPrototypeOf(a, dummy);
    print(a.foo, typeof a.subarray);
    b = f64p.subarray.call(a, 1, 3);
    print(b.foo, typeof b.subarray);
    print(Object.getPrototypeOf(a) === f64p, Object.getPrototypeOf(b) === f64p);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
