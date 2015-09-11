/*
 *  NaN handling
 *
 *  https://www.khronos.org/registry/typedarray/specs/latest/#4
 */

/*@include util-typedarray.js@*/

/*===
NaN handling test
through dataview
sign: 1
expt: 2047
mantissa is non-zero
through float64array
read back value: NaN
read back value is NaN: true
===*/

function nanHandlingTest() {
    var b = new ArrayBuffer(8);
    var v1 = new DataView(b);
    var v2 = new Uint8Array(b);
    var v3 = new Float64Array(b);
    var sign, expt;
    var i, t;

    /* IEEE double NaN pattern is (http://en.wikipedia.org/wiki/Double-precision_floating-point_format):
     *
     *   7ffm mmmm mmmm mmmm
     *   fffm mmmm mmmm mmmm
     *
     * where mmm.. is mantissa, which must be non-zero (any non-zero value is OK).
     */

    /*
     *  Test first through DataView
     */

    print('through dataview');

    v1.setFloat64(0, 0.0 / 0.0, false /*littleEndian*/);

    for (i = 0; i < 8; i++) {
        // Implementation specific, don't print
        //print(i, v1.getUint8(i).toString(16));
    }

    expt = v1.getUint16(0, false /*littleEndian*/);
    sign = expt >>> 15;
    expt = (expt >> 4) & 0x7ff;

    // Note: NaN normalization is allowed, and NaN sign is also platform
    // dependent.

    print('sign: ' + sign);
    print('expt: ' + expt);

    // Implementation specific, don't print
    //print('mant hi: ' + (v1.getUint32(0, false /*littleEndian*/) & 0x000fffff).toString(16));
    //print('mant lo: ' + (v1.getUint32(4, false /*littleEndian*/) & 0x000fffff).toString(16));

    if ((v1.getUint32(0, false /*littleEndian*/) & 0x000fffff) == 0 &&
        v1.getUint32(4, false /*littleEndian*/) == 0) {
        print('mantissa is zero');
    } else {
        print('mantissa is non-zero');
    }

    /*
     *  Then writing NaN explicitly, reading it into an Ecmascript value,
     *  writing it back, and checking through DataView.
     */

    print('through float64array');

    b = new ArrayBuffer(8);  // use a new ArrayBuffer just in case
    v1 = new DataView(b);
    v2 = new Uint8Array(b);
    v3 = new Float64Array(b);

    if (doubleEndianness == 'big') {
        // FF F0 00 00 00 00 00 01, NaN with sign set, mantissa lowest bit set
        v2[0] = 0xff;
        v2[1] = 0xf0;
        v2[2] = 0x00;
        v2[3] = 0x00;
        v2[4] = 0x00;
        v2[5] = 0x00;
        v2[6] = 0x00;
        v2[7] = 0x01;
    } else if (doubleEndianness == 'little') {
        v2[0] = 0x01;
        v2[1] = 0x00;
        v2[2] = 0x00;
        v2[3] = 0x00;
        v2[4] = 0x00;
        v2[5] = 0x00;
        v2[6] = 0xf0;
        v2[7] = 0xff;
    } else if (doubleEndianness == 'mixed') {
        v2[0] = 0x00;
        v2[1] = 0x00;
        v2[2] = 0xf0;
        v2[3] = 0xff;
        v2[4] = 0x01;
        v2[5] = 0x00;
        v2[6] = 0x00;
        v2[7] = 0x00;
    } else {
        throw new Error('internal error');
    }

    t = v3[0];
    print('read back value: ' + t);
    print('read back value is NaN: ' + isNaN(t));

    v3[0] = t;

    for (i = 0; i < 8; i++) {
        // Implementation specific, don't print
        //print(i, v2[i].toString(16));
    }

    if (doubleEndianness == 'big') {
        if ((v2[0] & 0x7f != 0x7f) ||
            (v2[1] & 0xf0 != 0xf0)) {
            throw new Error('expt is not 0x7ff');  // sign can be either
        }
        if ((v2[1] & 0x0f == 0x00) &&
            (v2[2] == 0x00) &&
            (v2[3] == 0x00) &&
            (v2[4] == 0x00) &&
            (v2[5] == 0x00) &&
            (v2[6] == 0x00) &&
            (v2[7] == 0x00)) {
            throw new Error('mantissa is zero');
        }
    } else if (doubleEndianness == 'little') {
        if ((v2[7] & 0x7f != 0x7f) ||
            (v2[6] & 0xf0 != 0xf0)) {
            throw new Error('expt is not 0x7ff');  // sign can be either
        }
        if ((v2[6] & 0x0f == 0x00) &&
            (v2[5] == 0x00) &&
            (v2[4] == 0x00) &&
            (v2[3] == 0x00) &&
            (v2[2] == 0x00) &&
            (v2[1] == 0x00) &&
            (v2[0] == 0x00)) {
            throw new Error('mantissa is zero');
        }
    } else if (doubleEndianness == 'mixed') {
        if ((v2[3] & 0x7f != 0x7f) ||
            (v2[2] & 0xf0 != 0xf0)) {
            throw new Error('expt is not 0x7ff');  // sign can be either
        }
        if ((v2[2] & 0x0f == 0x00) &&
            (v2[1] == 0x00) &&
            (v2[0] == 0x00) &&
            (v2[7] == 0x00) &&
            (v2[6] == 0x00) &&
            (v2[5] == 0x00) &&
            (v2[4] == 0x00)) {
            throw new Error('mantissa is zero');
        }
    } else {
        throw new Error('internal error');
    }
}

try {
    print('NaN handling test');
    nanHandlingTest();
} catch (e) {
    print(e.stack || e);
}
