/*
 *  Duktape Buffer objects
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
false
object
0,1,2,length,byteLength,byteOffset,BYTES_PER_ELEMENT
string 0 number 111
string 1 number 102
string 2 number 102
string length number 3
string byteLength number 3
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
true
object
0,1,2,length,byteLength,byteOffset,BYTES_PER_ELEMENT
string 0 number 111
string 1 number 102
string 2 number 102
string length number 3
string byteLength number 3
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
0,1,2,length,byteLength,byteOffset,BYTES_PER_ELEMENT
string 0 number 111
string 1 number 255
string 2 number 102
string length number 3
string byteLength number 3
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
0,1,2,length,byteLength,byteOffset,BYTES_PER_ELEMENT
string 0 number 111
string 1 number 255
string 2 number 102
string length number 3
string byteLength number 3
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
object
0,1,2,length,byteLength,byteOffset,BYTES_PER_ELEMENT
string 0 number 98
string 1 number 97
string 2 number 114
string length number 3
string byteLength number 3
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
false
false
0,1,2,3,length,byteLength,byteOffset,BYTES_PER_ELEMENT
string 0 number 113
string 1 number 117
string 2 number 255
string 3 number 120
string length number 4
string byteLength number 4
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
0,1,2,3,length,byteLength,byteOffset,BYTES_PER_ELEMENT
string 0 number 113
string 1 number 117
string 2 number 117
string 3 number 120
string length number 4
string byteLength number 4
string byteOffset number 0
string BYTES_PER_ELEMENT number 1
===*/

function dump(x) {
    // Works for both plain buffers and ArrayBuffers.
    print(Object.getOwnPropertyNames(x));
    Object.getOwnPropertyNames(x).forEach(function (k) {
        print(typeof k, k, typeof x[k], x[k]);
    });
}

function test() {
    var a, b, c;

    // Plain buffers can be created in many ways; from Ecmascript code one
    // easy way is to decode hex data.

    a = Duktape.dec('hex', '6f6666');
    print(a.valueOf() === a);  // .valueOf() is Object() coerced now, so false
    print(typeof a);
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

    // One way of making a buffer copy:

    a = Duktape.Buffer('quux');
    b = Duktape.Buffer(bufferToString(a));
    print(a === b);  // strict equals: compares pointer
    print(a == b);   // non-strict equals: compares pointer (Since Duktape 2.x)
    a[2] = 0xff;  // demonstrate independence
    dump(a);
    dump(b);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
