/* Print out a few IEEE double representations related to the Duktape fastint
 * number model.
 */

'use strict';

const { hexEncodeLower } = require('../util/hex');

function isFastint(x) {
    return Math.floor(x) === x &&
        x >= -(2 ** 47) &&
        x < (2 ** 47) &&
        (x !== 0 || 1 / x > 0 /* pos zero */ );
}

function stringRep(x) {
    var buf = new ArrayBuffer(8);
    var dv = new DataView(buf);
    dv.setFloat64(0, x); // big endian
    var sgnexp = dv.getUint16(0);
    var sgn = sgnexp >> 15;
    var exp = (sgnexp & 0x7ff0) >> 4;
    return hexEncodeLower(buf) + ' sgn=' + sgn + ' exp=' + exp + ' sgnexp=' + hexEncodeLower(sgnexp, 4);
}

function fastintReps() {
    [-(2 ** 47) - 1,
        -(2 ** 47),
        -(2 ** 47) + 1,
        -(2 ** 32) - 1,
        -(2 ** 32),
        -(2 ** 32) + 1,
        -0xdeadbeef,
        -9,
        -8,
        -8,
        -7,
        -6,
        -5,
        -4,
        -3,
        -2,
        -1,
        -0,
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        0xdeadbeef,
        (2 ** 32) - 1,
        (2 ** 32),
        (2 ** 32) + 1,
        (2 ** 47) - 1,
        (2 ** 47)
    ].forEach(function (v) {
        console.log(v, isFastint(v), stringRep(v));
    });
}
exports.fastintReps = fastintReps;
