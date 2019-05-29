/*
 *  Node.js Buffer toJSON()
 */

/*@include util-buffer.js@*/

/*===
node.js Buffer toJSON() test
object string Buffer object [65,66,67]
true
object string Buffer object []
true
{"foo":"bar","my_buffer":{"type":"Buffer","data":[241,242,243]}}
{"foo":"bar","my_buffer":{"type":"Buffer","data":[]}}
still here
===*/

function nodejsBufferToJsonTest() {
    var b, obj, v;
    var i, j;

    // buf.toJSON()

    // Explicit call

    b = new Buffer(3);
    b[0] = 0x41; b[1] = 0x42; b[2] = 0x43;
    v = b.toJSON();
    print(typeof v, typeof v.type, v.type, typeof v.data, JSON.stringify(v.data));
    print(Object.getPrototypeOf(v.data) === Array.prototype);

    b = new Buffer(0);
    v = b.toJSON();
    print(typeof v, typeof v.type, v.type, typeof v.data, JSON.stringify(v.data));
    print(Object.getPrototypeOf(v.data) === Array.prototype);

    // As part of JSON.stringify()

    b = new Buffer(3);
    b[0] = 0xf1; b[1] = 0xf2; b[2] = 0xf3;
    obj = { foo: 'bar', my_buffer: b };
    print(JSON.stringify(obj));

    b = new Buffer(0);
    obj = { foo: 'bar', my_buffer: b };
    print(JSON.stringify(obj));

    // Test up to 1MB for memory safety (no output), up to 4096 all byte sizes,
    // then skipping.
    var rnd = Math.random;
    for (i = 0; i < 1024 * 1024; i += (i < 4096 ? 1 : i >> 5)) {
        //print(i);
        b = new Buffer(i);
        for (j = 0; j < i; j++) {
            b[j] = rnd() * 256;
        }
        void b.toJSON();
    }
    print('still here');
}

try {
    print('node.js Buffer toJSON() test');
    nodejsBufferToJsonTest();
} catch (e) {
    print(e.stack || e);
}
