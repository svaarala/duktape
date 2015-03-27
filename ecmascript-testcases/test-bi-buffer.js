/*
 *  Duktape Buffer objects
 */

/*---
{
    "custom": true
}
---*/

/*===
buffer
TypeError
length number 3
byteLength number 3
byteOffset number 0
BYTES_PER_ELEMENT number 1
0 number 111
1 number 102
2 number 102
true
object
string 0 number 111
string 1 number 102
string 2 number 102
string length number 3
string byteLength number 3
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
length number 3
byteLength number 3
byteOffset number 0
BYTES_PER_ELEMENT number 1
0 number 111
1 number 255
2 number 102
string 0 number 111
string 1 number 255
string 2 number 102
string length number 3
string byteLength number 3
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
buffer
length number 3
byteLength number 3
byteOffset number 0
BYTES_PER_ELEMENT number 1
0 number 98
1 number 97
2 number 114
false
true
length number 4
byteLength number 4
byteOffset number 0
BYTES_PER_ELEMENT number 1
0 number 113
1 number 117
2 number 255
3 number 120
length number 4
byteLength number 4
byteOffset number 0
BYTES_PER_ELEMENT number 1
0 number 113
1 number 117
2 number 117
3 number 120
===*/

function dump(x) {
    if (typeof x === 'buffer') {
        // Object.getOwnPropertyNames() throws for a plain buffer
        print('length', typeof x.length, x.length);
        print('byteLength', typeof x.byteLength, x.byteLength);
        print('byteOffset', typeof x.byteOffset, x.byteOffset);
        print('BYTES_PER_ELEMENT', typeof x.BYTES_PER_ELEMENT, x.BYTES_PER_ELEMENT);
        for (i = 0; i < x.length; i++) {
            print(i, typeof x[i], x[i]);
        }
    } else {
        Object.getOwnPropertyNames(x).forEach(function (k) {
            print(typeof k, k, typeof x[k], x[k]);
        });
    }
}

function test() {
    var a, b, c;

    // Plain buffers can be created in many ways; from Ecmascript code one
    // easy way is to decode hex data.

    a = Duktape.dec('hex', '6f6666');
    print(typeof a);
    try {
        // For strings: Object.getOwnPropertyNames("foo") throws an error
        // because the argument is not an object.  Buffers behave the same.
        print(Object.getOwnPropertyNames(a));
    } catch (e) {
        print(e.name);
    }
    dump(a);

    // The Duktape.Buffer() constructor works similarly to the String()
    // constructor.  When called through 'new' (as a constructor) it will
    // create a Buffer value with the underlying primitive value pointing
    // to the input buffer.

    b = new Duktape.Buffer(a);
    print(b.valueOf() === a);  // strict comparison: does not compare contents
    print(typeof b);
    dump(b);

    // The buffer value will be shared.  Here modifications to 'a' show
    // up in 'b' and vice versa.  There is no analogous behavior in the
    // standard Ecmascript built-ins because the primitive values (such
    // as booleans and strings) are not mutable.

    a[1] = 0xff;
    dump(a);
    dump(b);

    // When Duktape.Buffer() is called as a function, it will coerce its
    // argument to a *plain* buffer - again, this follows the behavior
    // of String.

    c = Duktape.Buffer('bar');
    print(typeof c);
    dump(c);

    // In Duktape 1.0 the Ecmascript buffer bindings are very minimal:
    // buffers are intended to be manipulated mainly from C code.  As an
    // example, there's no convenient way to make an actual copy of a buffer;
    // the closest workaround (which is memory inefficient) is to go through
    // a string temporary:

    a = Duktape.Buffer('quux');
    b = Duktape.Buffer(String(a));
    print(a === b);  // strict equals: compares pointer
    print(a == b);   // non-strict equals: compares contents
    a[2] = 0xff;
    dump(a);
    dump(b);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
