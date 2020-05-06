// Bitpack input bytes (a string in internal representation) into a
// bit-packed format shared by heap and thread RAM init data.  Output
// is into a user-supplied BitEncoder, and stats are accumulated to an
// optional stats object.
//
// Strings are encoded as follows: a string begins in lowercase mode
// and recognizes the following 5-bit symbols:
//
//     0-25    'a' ... 'z' or 'A' ... 'Z' depending on mode
//     26-31   special controls, see code below
//
// Input string data is already in internal format (i.e. CESU-8,
// Symbol representations, etc).

'use strict';

const { assert } = require('../util/assert');
const { bstrToUint8Array } = require('../util/bstr');
const { BitEncoder } = require('../util/bitencoder');

const LOOKUP1 = 26;
const LOOKUP2 = 27;
const SWITCH1 = 28;
const SWITCH = 29;
const UNUSED1 = 30; void UNUSED1;
const EIGHTBIT = 31;

const charLookup = '0123456789_ \x82\x80"{';  // special characters

function isCodepointLower(cp) {
    return (cp >= 0x61 && cp <= 0x7a);
}

function isCodepointUpper(cp) {
    return (cp >= 0x41 && cp <= 0x5a);
}

function bitpack5BitUint8Array(be, data, stats) {
    assert(charLookup.length === 16);

    // Support up to 255 byte strings now, cases above ~30 bytes are very
    // rare, so favor short strings in encoding.
    if (data.length <= 30) {
        be.bits(data.length, 5);
    } else if (data.length <= 255) {
        be.bits(31, 5);
        be.bits(data.length, 8);
    } else {
        throw new TypeError('input too long: ' + data.length);
    }

    // 5-bit character, mode specific interpretation, start from lower
    // case state.
    var mode = 'lowercase';

    if (stats) { stats.numInputBytes += data.length; }

    for (let i = 0; i < data.length; i++) {
        let o = data[i];
        let c = String.fromCharCode(o);

        let isLower = isCodepointLower(o);
        let isUpper = isCodepointUpper(o);
        let isLast = (i == data.length - 1);

        let isNextLower = false;
        let isNextUpper = false;
        if (!isLast) {
            let o2 = data[i + 1];
            isNextLower = isCodepointLower(o2);
            isNextUpper = isCodepointUpper(o2);
        }

        //console.debug(o, c, isLower, isUpper, isLast, isNextLower, isNextUpper);

        if (isLower && mode === 'lowercase') {
            be.bits(o - 0x61, 5);
            if (stats) { stats.numOptimal++; }
        } else if (isUpper && mode === 'uppercase') {
            be.bits(o - 0x41, 5);
            if (stats) { stats.numOptimal++; }
        } else if (isLower && mode === 'uppercase') {
            if (isNextLower) {
                be.bits(SWITCH, 5);
                be.bits(o - 0x61, 5);
                mode = 'lowercase';
                if (stats) { stats.numSwitch++; }
            } else {
                be.bits(SWITCH1, 5);
                be.bits(o - 0x61, 5);
                if (stats) { stats.numSwitch1++; }
            }
        } else if (isUpper && mode === 'lowercase') {
            if (isNextUpper) {
                be.bits(SWITCH, 5);
                be.bits(o - 0x41, 5);
                mode = 'uppercase';
                if (stats) { stats.numSwitch++; }
            } else {
                be.bits(SWITCH1, 5);
                be.bits(o - 0x41, 5);
                if (stats) { stats.numSwitch1++; }
            }
        } else if (charLookup.indexOf(c) >= 0) {
            let idx = charLookup.indexOf(c);
            if (idx >= 8) {
                be.bits(LOOKUP2, 5);
                be.bits(idx - 8, 3);
                if (stats) { stats.numLookup2++; }
            } else {
                be.bits(LOOKUP1, 5);
                be.bits(idx, 3);
                if (stats) { stats.numLookup1++; }
            }
        } else if (o >= 0 && o <= 255) {
            //console.debug('eightbit encoding for codepoint ' + o);
            be.bits(EIGHTBIT, 5);
            be.bits(o, 8)
            if (stats) { stats.numEightBit++; }
        } else {
            throw new TypeError('internal error in bitpacking a string, codepoint ' + o);
        }
    }
}
exports.bitpack5BitUint8Array = bitpack5BitUint8Array;

// Variant which accepts a byte string (bstr).
function bitpack5BitBstr(be, s, stats) {
    return bitpack5BitUint8Array(be, bstrToUint8Array(s), stats);
}
exports.bitpack5BitBstr = bitpack5BitBstr;

function test() {
    var be = new BitEncoder();
    bitpack5BitBstr(be, 'testString1234567890!');
    var u8 = be.getBytes();
    // From Python genbuiltins.py:
    // acc929f2538a1a6d1d2d3d4d5d6d7d8d9d0f9080
    var expectU8 = new Uint8Array([ 0xac, 0xc9, 0x29, 0xf2, 0x53, 0x8a, 0x1a, 0x6d, 0x1d,
                                    0x2d, 0x3d, 0x4d, 0x5d, 0x6d, 0x7d, 0x8d, 0x9d, 0x0f,
                                    0x90, 0x80 ]);
    assert(u8.length === expectU8.length);
    for (let i = 0; i < u8.length; i++) {
        assert(u8[i] === expectU8[i]);
    }
}
exports.test = test;
