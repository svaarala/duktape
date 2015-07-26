/*
 *  Node.js Buffer toJSON()
 */

/*@include util-nodejs-buffer.js@*/

/*===
node.js Buffer toJSON() test
object string Buffer object [65,66,67]
object string Buffer object []
{"foo":"bar","my_buffer":{"type":"Buffer","data":[241,242,243]}}
{"foo":"bar","my_buffer":{"type":"Buffer","data":[]}}
===*/

function nodejsBufferToJsonTest() {
    var b, obj, v;

    // buf.toJSON()

    // Explicit call

    b = new Buffer(3);
    b[0] = 0x41; b[1] = 0x42; b[2] = 0x43;
    v = b.toJSON();
    print(typeof v, typeof v.type, v.type, typeof v.data, JSON.stringify(v.data));

    b = new Buffer(0);
    v = b.toJSON();
    print(typeof v, typeof v.type, v.type, typeof v.data, JSON.stringify(v.data));

    // As part of JSON.stringify()

    b = new Buffer(3);
    b[0] = 0xf1; b[1] = 0xf2; b[2] = 0xf3;
    obj = { foo: 'bar', my_buffer: b };
    print(JSON.stringify(obj));

    b = new Buffer(0);
    obj = { foo: 'bar', my_buffer: b };
    print(JSON.stringify(obj));
}

try {
    print('node.js Buffer toJSON() test');
    nodejsBufferToJsonTest();
} catch (e) {
    print(e.stack || e);
}
