/*
 *  Select a set of Unicode characters (based on included/excluded categories)
 *  and write out a compact bitstream for matching a character against
 *  the set at runtime.  This is for the slow path, where we're especially
 *  concerned with compactness.
 *
 *  Unicode categories (such as 'Z') and codepoints (such as 0x200d) can be
 *  used.  Two pseudo-categories are also available for exclusion only: 'ASCII'
 *  and 'NONBMP'.  'ASCII' category excludes ASCII codepoints which is useful
 *  because C code typically contains an ASCII fast path so ASCII characters
 *  don't need to be considered in the Unicode tables.  'NONBMP' excludes
 *  codepoints above U+FFFF which is useful because such codepoints don't need
 *  to be supported in standard ECMAScript.
 */

'use strict';

const { createBareObject } = require('../util/bare');
const { BitEncoder } = require('../util/bitencoder');
const { assert } = require('../util/assert');

// Filter cpMap by first including all categories listed in 'includeList' and
// then excluding anything in 'excludeList'.
function filterCpMap(cpMap, includeList, excludeList) {
    var filterAscii = excludeList.indexOf('ASCII') >= 0;
    var filterNonBmp = excludeList.indexOf('NONBMP') >= 0;
    var includeCatMap = createBareObject({});
    var excludeCatMap = createBareObject({});
    var includeCpMap = createBareObject({});
    var excludeCpMap = createBareObject({});
    var filteredCpMap;

    // Helper lookups to speed up processing.
    includeList.forEach((cat) => {
        if (typeof cat === 'number') {
            includeCpMap[cat] = true;
        } else if (typeof cat === 'string') {
            includeCatMap[cat] = true;
        } else {
            throw new TypeError('invalid includeList entry: ' + cat);
        }
    });
    excludeList.forEach((cat) => {
        if (typeof cat === 'number') {
            excludeCpMap[cat] = true;
        } else if (typeof cat === 'string') {
            excludeCatMap[cat] = true;  // includes ASCII and NONBMP, does not matter
        } else {
            throw new TypeError('invalid excludeList entry: ' + cat);
        }
    });

    // Filter codepoint map according to our criteria.
    filteredCpMap = cpMap.filter((ent) => {
        if (!ent) {
            return false;
        }
        let cp = ent.cp;
        let gc = ent.gc;
        if ((filterAscii && cp <= 0x7f) || (filterNonBmp && cp >= 0x10000)) {
            return false;
        }
        if ((includeCatMap[gc] || includeCpMap[cp]) &&
            !(excludeCatMap[gc] || excludeCpMap[cp])) {
            // Included in one or more categories/codepoints, and not
            // excluded by any categories/codepoints.
            return true;
        }
        return false;
    });

    return filteredCpMap;
}
exports.filterCpMap = filterCpMap;

// Pack match ranges into a varint encoding.  For previous unused encoding
// variants, see old Python tooling.
function generateMatchTable3(ranges) {
    var be = new BitEncoder();
    var freq = [];  // informative
    while (freq.length < 0x110000) {
        freq.push(0);
    }

    function encCustom(x) {
        freq[x]++;

        if (x <= 0x0e) {
            // 4-bit encoding
            be.bits(x, 4);
            return;
        }
        x -= 0x0e + 1;

        if (x <= 0xfd) {
            // 12-bit encoding
            be.bits(0x0f, 4);
            be.bits(x, 8);
            return;
        }
        x -= 0xfd + 1;

        if (x <= 0xfff) {
            // 24-bit encoding
            be.bits(0x0f, 4);
            be.bits(0xfe, 8);
            be.bits(x, 12);
            return;
        }
        x -= 0xfff + 1;

        // 36-bit encoding
        be.bits(0x0f, 4);
        be.bits(0xff, 8);
        be.bits(x, 24);
    }

    function encVaruint(x) {
        be.varuint(x);
    }
    void encVaruint;

    var enc = encCustom;

    var prevRangeEnd = 0;
    for (let i = 0; i < ranges.length; i++) {
        let rangeStart = ranges[i][0];
        let rangeEnd = ranges[i][1];
        let r1 = rangeStart - prevRangeEnd;  // 1 or above (no unjoined ranges)
        assert(r1 >= 1);
        let r2 = rangeEnd - rangeStart;  // 0 or above
        assert(r2 >= 0);

        // r1 is >= 1, so r1 == 0 is used as an end marker.  Encoding an
        // explicit count and (r1 - 1) here improves total output size
        // by about 30 bytes.
        //
        // Encoding using BitEncoder varuint is more efficient and shares
        // code so maybe switch to that.

        enc(r1);
        enc(r2);
        prevRangeEnd = rangeEnd;
    }

    // End marker (r1 can never be 0).
    enc(0);

    return {
        data: be.getBytes(),
        nbits: be.getNumBits(),
        freq: freq
    };
}
exports.generateMatchTable3 = generateMatchTable3;
