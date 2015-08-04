/*
 *  Test wrap handling of indexed bufferobject properties.  This is easy
 *  to get wrong in the implementation, e.g. the following can wrap:
 *
 *    byte_offset = array_index << bufobj->shift
 *
 *  On a 32-bit target array_index 0x40000000 would wrap to 0 if shift
 *  was 2 (Uint32Array for example).
 */

/*===
1094795585
1094795585
undefined
undefined
undefined
undefined
undefined
undefined
===*/

function bufferObjectWrapTest() {
    var b = new Uint32Array(0x100000);
    var i;

    for (var i = 0; i < b.length; i++) {
        b[i] = 0x41414141;
    }

    print(b[0x00000000]);
    print(b[0x000fffff]);
    print(b[0x40000000]);
    print(b[0x400fffff]);
    print(b[0x80000000]);
    print(b[0x800fffff]);
    print(b[0xc0000000]);
    print(b[0xc00fffff]);
}

try {
    bufferObjectWrapTest();
} catch (e) {
    print(e.stack || e);
}
