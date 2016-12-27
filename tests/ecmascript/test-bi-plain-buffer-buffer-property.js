/*@include util-buffer.js@*/

/*===
[object Uint8Array]
true
10
[object ArrayBuffer]
undefined
10
[object ArrayBuffer]
undefined
10
false
|00001111ff0000000000|
|00001111ff0000000000|
|00001111ff0000000000|
|00001111ff0000000000|
===*/

function test() {
    var plain = Uint8Array.plainOf(new Uint8Array(10));
    print(Object.prototype.toString.call(plain));
    print(isPlainBuffer(plain));
    print(plain.length);

    // Plain buffers inherit from Uint8Array.prototype and their .buffer
    // is an inherited getter which creates an ArrayBuffer instance with
    // the same backing as the plain buffer.

    var a1 = plain.buffer;
    print(Object.prototype.toString.call(a1));
    print(a1.length);
    print(a1.byteLength);

    // However, because there's no place to store/cache the ArrayBuffer,
    // each access creates a new ArrayBuffer instance.  They all share the
    // same backing however.

    var a2 = plain.buffer;
    print(Object.prototype.toString.call(a1));
    print(a1.length);
    print(a1.byteLength);
    print(a1 === a2);

    // Views sharing the plain buffer can be constructed over the ArrayBuffers.

    var u16 = new Uint16Array(a2);
    plain[4] = 0xff;
    u16[1] = 0x1111;

    print(Duktape.enc('jx', plain));
    print(Duktape.enc('jx', a1));
    print(Duktape.enc('jx', a2));
    print(Duktape.enc('jx', u16));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
