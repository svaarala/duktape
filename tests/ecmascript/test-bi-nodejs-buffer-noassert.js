/*
 *  noAssert specifics
 */

/*@include util-nodejs-buffer.js@*/

/*===
noAssert write return value test
4
8 bytes: deadbeef45464748
5
8 bytes: 41deadbeef464748
3
8 bytes: 4142434445464748
8
8 bytes: 41424344deadbeef
9
8 bytes: 4142434445464748
===*/

function noAssertWriteRetvalTest() {
    /*
     *  Node.js return value for a write that fails with noAssert=true
     *  is technically the same as for a succeeded call:
     *
     *      offset + #bytes_written
     *
     *  Because the input offset is coerced this causes weird return
     *  values when the input offset is a negative value (which is
     *  invalid but causes no error throw with noAssert==true).
     */

    var b;
    var ret;

    b = new Buffer('ABCDEFGH');
    ret = b.writeUInt32BE(0xdeadbeef, 0, true);
    print(ret);  // 0 + 4 -> 4
    printNodejsBuffer(b);

    b = new Buffer('ABCDEFGH');
    ret = b.writeUInt32BE(0xdeadbeef, 1, true);
    print(ret);  // 1 + 4 -> 5
    printNodejsBuffer(b);

    // This case is interesting.  Internally this is what happens:
    //   - -1 coerces to 0xffffffff
    //   - 0xffffffff + 4 = 4294967299, but when duk_uint_t is 32-bit,
    //     the result wraps to 3.
    //
    // Node.js returns 4294967299 here (no wrapping in the addition but
    // unsigned coercion); Duktape returns 3.
    b = new Buffer('ABCDEFGH');
    ret = b.writeUInt32BE(0xdeadbeef, -1, true);
    print(ret);
    printNodejsBuffer(b);

    b = new Buffer('ABCDEFGH');
    ret = b.writeUInt32BE(0xdeadbeef, 4, true);
    print(ret);
    printNodejsBuffer(b);

    b = new Buffer('ABCDEFGH');
    ret = b.writeUInt32BE(0xdeadbeef, 5, true);
    print(ret);
    printNodejsBuffer(b);
}

try {
    print('noAssert write return value test');
    noAssertWriteRetvalTest();
} catch (e) {
    print(e.stack || e);
}

/*===
noAssert cast test
undefined RangeError 4142434445464748
null RangeError 4142434445464748
true 3 4142434445464748
false RangeError 4142434445464748
0 RangeError 4142434445464748
123 3 4142434445464748
 RangeError 4142434445464748
foo 3 4142434445464748
 3 4142434445464748
[object Object] 3 4142434445464748
[object Object] 3 4142434445464748
[object Object] 3 4142434445464748
===*/

function noAssertCastTest() {
    function test(val) {
        var b = new Buffer('ABCDEFGH');
        var ret;
        try {
            ret = b.writeUInt32BE(0xdeadbeef, -1, val);
            print(val, ret, printableNodejsBuffer(b));
        } catch (e) {
            print(val, e.name, printableNodejsBuffer(b));
        }
    }

    // Node.js seems to use ToBoolean() cast, ToBoolean() doesn't invoke
    // valueOf() but all objects are considered truthy.

    [ undefined, null, true, false, 0, 123, '', 'foo', [], {},
      { valueOf: function () { return true; } },
      { valueOf: function () { return false; } } ].forEach(test);
}

try {
    print('noAssert cast test');
    noAssertCastTest();
} catch (e) {
    print(e.stack || e);
}
