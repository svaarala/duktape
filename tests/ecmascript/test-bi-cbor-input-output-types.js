/*===
foo
foo
===*/

// CBOR.decode() accepts any buffer type now; cbor-js only accepts an
// ArrayBuffer.

var t;

try {
    t = new Uint8Array([ 0x63, 0x66, 0x6f, 0x6f ]).buffer;
    print(CBOR.decode(t));
} catch (e) {
    print(e.name);
}

try {
    t = new Uint8Array([ 0x63, 0x66, 0x6f, 0x6f ]);
    print(CBOR.decode(t));
} catch (e) {
    print(e.name);
}

/*===
object
true
false
===*/

// CBOR.encode() results in an ArrayBuffer.
try {
    t = CBOR.encode('foo');
    print(typeof t);
    print(t instanceof ArrayBuffer);
    print(t instanceof Uint8Array);
} catch (e) {
    print(e.name);
}

/*===
object
false
true
===*/

// While ArrayBuffer is the preferred input/output type, buffer values still
// decode as Uint8Array in cbor-js and Duktape.

try {
    t = CBOR.decode(new Uint8Array([ 0x43, 0x11, 0x22, 0x33 ]).buffer);
    print(typeof t);
    print(t instanceof ArrayBuffer);
    print(t instanceof Uint8Array);
} catch (e) {
    print(e.name);
}

/*===
done
===*/

print('done');
