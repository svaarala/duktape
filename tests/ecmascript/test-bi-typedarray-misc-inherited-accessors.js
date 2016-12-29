/*
 *  In ES2015 all ArrayBuffer and typed array properties like .length, .byteLength
 *  are inherited accessors.  At least up to Duktape 2.0 they are virtual "own"
 *  properties.
 */

/*===
[object ArrayBuffer]
length false false undefined undefined
byteLength true false number 10
byteOffset false false undefined undefined
BYTES_PER_ELEMENT false false undefined undefined
0 false false undefined undefined
9 false false undefined undefined
10 false false undefined undefined
[object Uint8Array]
length true false number 10
byteLength true false number 10
byteOffset true false number 0
BYTES_PER_ELEMENT true false number 1
0 true true number 0
9 true true number 0
10 false false undefined undefined
[object Float64Array]
length true false number 10
byteLength true false number 80
byteOffset true false number 0
BYTES_PER_ELEMENT true false number 8
0 true true number 0
9 true true number 0
10 false false undefined undefined
===*/

function test() {
    [ new ArrayBuffer(10), new Uint8Array(10), new Float64Array(10) ].forEach(function (buf) {
        print(Object.prototype.toString.call(buf));
        [ 'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT',
          '0', '9', '10' ].forEach(function (v) {
            print(v, v in buf, buf.hasOwnProperty(v), typeof buf[v], buf[v]);
        });
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
