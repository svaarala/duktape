/*@include util-buffer.js@*/

/*===
ArrayBuffer.prototype methods
- slice
97
object [object Uint8Array]
true
4 0 4 1
99 100 101 102 undefined
255 99
- slice stress
done
===*/

function arrayBufferPrototypeMethodTest() {
    var pb, ab, t;
    var i, j;

    function resetValues() {
        pb = createPlainBuffer('abcdefghijklmnop');
        ab = new Uint8Array([
            0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
            0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70
        ]).buffer;
    }

    // ArrayBuffer.prototype only has .slice()
    print('- slice');
    resetValues();

    t = ArrayBuffer.prototype.slice.call(pb, 2, 6);
    print(pb[0]);
    print(typeof t, t);
    print(isPlainBuffer(t));
    print(t.byteLength, t.byteOffset, t.length, t.BYTES_PER_ELEMENT);
    print(t[0], t[1], t[2], t[3], t[4]);
    t[0] = 0xff;
    print(t[0], pb[2]);

    // Stress test for ArrayBuffer.prototype.slice()
    print('- slice stress');
    resetValues();
    for (i = -3; i < 19; i++) {
        for (j = -3; j < 19; j++) {
            var ps = ArrayBuffer.prototype.slice.call(pb, i, j);
            var as = ab.slice(i, j);
            var px = Duktape.enc('jx', ps);
            var ax = Duktape.enc('jx', as);

            //print(i, j, px, ax);
            if (px !== ax) {
                print('slices differ:', i, j, px, ax);
            }
        }
    }
    print('done');
}

try {
    print('ArrayBuffer.prototype methods');
    arrayBufferPrototypeMethodTest();
} catch (e) {
    print(e.stack || e);
}
