/*
 *  Buffer.isBuffer()
 */

/*@include util-nodejs-buffer.js@*/

/*===
isBuffer test
true
true
false
false
false
false
false
false
false
===*/

function isBufferTest() {
    var b1 = new Buffer(16);
    var b2;
    var b2 = b1.slice(4, 8);

    [ b1, b2, Buffer, Buffer.prototype, Object, Object.prototype, {}, 123, null ].forEach(function (v) {
        try {
            print(Buffer.isBuffer(v));
        } catch (e) {
            print(e);
        }
    });
}

try {
    print('isBuffer test');
    isBufferTest();
} catch (e) {
    print(e.stack || e);
}

/*===
inheritance test
true
4
65 66 67 68
===*/

function inheritanceTest() {
    var obj = {};
    var buf = new Buffer('ABCD');
    Object.setPrototypeOf(obj, buf);
    print(Buffer.isBuffer(obj));
    print(obj.length);
    print(obj[0], obj[1], obj[2], obj[3]);
}

try {
    print('inheritance test');
    inheritanceTest();
} catch (e) {
    print(e.stack || e);
}
