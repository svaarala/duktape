// Encode variable bit-width fields into a bitstream.  Supports encoding of
// integers, 8-bit strings, and 'varuint'.  Maintains statistics of encoded
// fields, especially varuint performance.
//
// The 'varuint' encoding is shared by multiple internal formats and has been
// manually tuned to work reasonably well.  If changed, check the effect on
// the full Duktape build.  Allowed integer range is [0,1048575], i.e. 20 bits.

'use strict';

const { uint8ArrayToBstr } = require('./bstr');
const { assert } = require('./assert');

function zeroArray(length) {
    var res = [];
    while (res.length < length) {
        res.push(0);
    }
    return res;
}

function BitEncoder() {
    Object.assign(this, {
        currBits: [],
        bitsCount: 0,
        varuintDist: zeroArray(65536),
        varuintCats: zeroArray(5),
        varuintCount: 0,
        varuintBits: 0
    });
}

// Append 'nbits' bits with value 'x'.  Fails if 'x' has more bits than 'nbits'.
BitEncoder.prototype.bits = function bits(x, nbits) {
    if ((x >>> nbits) !== 0) {
        throw new TypeError('input value has too many bits; value: ' + x + ', nbits: ' + nbits);
    }
    for (let shift = nbits - 1; shift >= 0; shift--) {
        this.currBits.push((x >>> shift) & 0x01);
    }
};

// Append a bstr as 8-bit codepoints.  Fails if string contains codepoints
// higher than U+00FF.
BitEncoder.prototype.string = function string(x) {
    for (let i = 0; i < x.length; i++) {
        let cp = x.charCodeAt(i);
        if (cp >= 0x100) {
            throw new TypeError('input string has a codepoint above U+00FF: ' + cp);
        }
        for (let shift = 7; shift >= 0; shift--) {
            this.currBits.push((cp >>> shift) & 0x01);
        }
    }
};

// Append an Uint8Array.
BitEncoder.prototype.uint8array = function uint8array(x) {
    assert(x instanceof Uint8Array);
    for (let i = 0; i < x.length; i++) {
        let val = x[i];
        for (let shift = 7; shift >= 0; shift--) {
            this.currBits.push((val >>> shift) & 0x01);
        }
    }
};

// Encode an unsigned integer as a 'varuint'.
BitEncoder.prototype.varuint = function varuint(x) {
    if (typeof x !== 'number' || !(x >= 0)) {
        throw new TypeError('invalid argument');
    }
    if (x < this.varuintDist.length) {
        this.varuintDist[x]++;
    }
    this.varuintCount++;

    if (x === 0) {
        this.bits(0, 2);   // 0b00
        this.varuintBits += 2;
        this.varuintCats[0]++;
    } else if (x <= 4) {
        this.bits(1, 2);   // 0b01
        this.bits(x - 1, 2);
        this.varuintBits += 2 + 2;
        this.varuintCats[1]++;
    } else if (x <= 36) {
        this.bits(2, 2);   // 0b10
        this.bits(x - 5, 5);
        this.varuintBits += 2 + 5;
        this.varuintCats[2]++;
    } else if (x <= 163) {
        this.bits(3, 2);   // 0b11
        this.bits(x - 37 + 1, 7);  // 7-bit: [1,127]
        this.varuintBits += 2 + 7;
        this.varuintCats[3]++;
    } else if (x <= 1048575) {
        this.bits(3, 2);   // 0b11
        this.bits(0, 7);   // 7-bit: 0, marker
        this.bits(x, 20);  // 20-bit value
        this.varuintBits += 2 + 7 + 20;
        this.varuintCats[4]++;
    } else {
        throw new TypeError('varuint too large: ' + x);
    }
};

// Get current number of bits.
BitEncoder.prototype.getNumBits = function getNumBits() {
    return this.currBits.length;
};

// Get current number of bytes, rounded up.
BitEncoder.prototype.getNumBytes = function getNumBytes() {
    var nbits = this.currBits.length;
    while (nbits % 8) {
        nbits++;
    }
    return nbits / 8;
};

// Get current bitstream as a byte sequence (Uint8Array), padded with
// zero bits to make full bytes.
BitEncoder.prototype.getBytes = function getBytes() {
    var outLength = this.getNumBytes();
    var u8 = new Uint8Array(outLength);

    for (let i = 0; i < outLength; i++) {
        let t = 0;
        for (let j = 0; j < 8; j++) {
            let off = i * 8 + j;
            if (off >= this.currBits.length) {
                t = (t << 1);
            } else {
                t = (t << 1) + this.currBits[off];
            }
        }
        u8[i] = t;
    }

    return u8;
};

// Get current bitstream, padded with zeroes, as a byte string (bstr).
BitEncoder.prototype.getByteString = function getByteString() {
    return uint8ArrayToBstr(this.getBytes());
};

// Get bitstream stats.
BitEncoder.prototype.getStats = function getStats() {
    return {
        numBits: this.getNumBits(),
        numBytes: this.getNumBytes(),
        encVaruintCount: this.varuintCount,
        encVaruintBits: this.varuintBits,
        encVaruintBitsPerField: this.varuintBits / this.varuintCount,
        encVaruintCats: this.varuintCats,
        encVaruintDist: this.varuintDist
    };
};

// Get bitstream one-liner stats string.
BitEncoder.prototype.getStatsString = function getStatsString() {
    var st = this.getStats();
    var msg = 'BitEncoder: ' + st.numBits + ' bits, ' + st.numBytes + ' bytes, ' +
              st.encVaruintCount + ' varuints, ' + st.encVaruintBitsPerField +
              ' bits/varuint, varuint categories: ' + JSON.stringify(st.encVaruintCats);
    return msg;
};

exports.BitEncoder = BitEncoder;

function test() {
    var be = new BitEncoder();
    be.bits(123, 7);
    be.string('foo\u00febar');
    be.varuint(3);
    be.varuint(123);
    be.varuint(64000);
    //console.log('getNumBits: ' + be.getNumBits());
    //console.log('getNumBytes: ' + be.getNumBytes());
    //console.log('getStatsString: ' + be.getStatsString());
    var u8 = be.getBytes();
    // From dukutil.BitEncoder: f6ccdedffcc4c2e4dd7c007d0000
    var expectU8 = new Uint8Array([ 0xf6, 0xcc, 0xde, 0xdf, 0xfc, 0xc4, 0xc2, 0xe4,
                                    0xdd, 0x7c, 0x00, 0x7d, 0x00, 0x00 ]);
    assert(u8.length === expectU8.length);
    for (let i = 0; i < u8.length; i++) {
        assert(u8[i] === expectU8[i]);
    }
}
exports.test = test;
