/*
 *  Bufferobjects have a shallow fastpath for getprop/putprop.  Test that
 *  fastpath and also the slow path which is needed for inheritance.
 */

/*===
object
100
101
object
101
101 200
101 201
===*/

function bufferObjectFastPathTest() {
    var b1 = new Uint16Array(8);

    var b2 = {};
    Object.setPrototypeOf(b2, b1);

    print(typeof b1);
    b1[3] = 100;     // fast path write
    print(b1[3]);    // fast path read
    b1['3'] = 101;   // slow path write
    print(b1['3']);  // slow path read

    print(typeof b2);
    print(b2[3]);             // slow path read, inherited from b1
    b2[3] = 200;              // slow path write; creates shadowing own property '3'
    print(b1[3], b2[3]);      // fast path read + slow path read
    b2['3'] = 201;            // slow path write; updates shadowing own property '3'
    print(b1['3'], b2['3']);  // slow path read + slow path read
}

try {
    bufferObjectFastPathTest();
} catch (e) {
    print(e.stack || e);
}
