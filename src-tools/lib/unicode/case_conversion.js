/*
 *  Extract rules for Unicode case conversion, specifically the behavior
 *  required by ECMAScript E5 in Sections 15.5.4.16 to 15.5.4.19.  The
 *  bitstream encoded rules are used for the slow path at run time, so
 *  compactness is favored over speed.
 *
 *  There is no support for context or locale sensitive rules, as they
 *  are handled directly in C code before consulting tables generated
 *  here.  ECMAScript requires case conversion both with and without
 *  locale/language specific rules (e.g. String.prototype.toLowerCase()
 *  and String.prototype.toLocaleLowerCase()), so they are best handled
 *  in C anyway.
 *
 *  Case conversion rules for ASCII are also excluded as they are handled
 *  by the C fast path.  Rules for non-BMP characters (codepoints above
 *  U+FFFF) are omitted as they're not required for standard ECMAScript.
 */

'use strict';

const { BitEncoder } = require('../util/bitencoder');
const { jsonDeepClone } = require('../util/clone');
const { numericSort } = require('../util/sort');
const { assert } = require('../util/assert');

const SKIP_MAX = 6;  // Skips 1...6 are useful at least.

// Remove ASCII case conversion parts, as they are handled by C fast path.
function removeConversionMapAscii(convmap) {
    for (let i = 0; i < 128; i++) {
        delete convmap[i];
    }
}
exports.removeConversionMapAscii = removeConversionMapAscii;

// Create lowercase, uppercase, and titlecase conversion maps from
// parsed Unicode data.
function createConversionMaps(cpMap) {
    var uc = [];  // uppercase, codepoint number -> codepoint array
    var lc = [];  // lowercase
    var tc = [];  // titlecase

    cpMap.forEach(function (v) {
        if (!v) {
            return;
        }
        if (v.cp >= 0x10000) {
            // Only BMP support needed.
            return;
        }
        if (v.uc) {
            uc[v.cp] = v.uc;
        }
        if (v.lc) {
            lc[v.cp] = v.lc;
        }
        if (v.tc) {
            tc[v.cp] = v.tc;
        }
    });

    return { uc, lc, tc };
}
exports.createConversionMaps = createConversionMaps;

// Scan for a range of continuous case conversions, for example:
// A -> B, A + SKIP -> B + SKIP, A + 2*SKIP -> B + 2*SKIP, etc.
// Delete the range if found.  These patterns occur quite a lot.
//
// XXX: Mark SpecialCasing and other fast paths (matched before
// these ranges) as "don't care" to minimize table sizes.
function scanRangeWithSkip(convmap, start_idx, skip) {
    var conv_i, conv_o, start_i, start_o;

    conv_i = start_idx;
    if (!(conv_i in convmap)) {
        // Case conversion rule not present, converts to itself.
        return null;
    }
    if (convmap[conv_i].length > 1) {
        // Complex case conversion, to multiple characters, ignored.
        return null;
    }
    conv_o = convmap[conv_i][0];

    start_i = conv_i;
    start_o = conv_o;
    for (;;) {
        let new_i = conv_i + skip;
        let new_o = conv_o + skip;

        if (!(new_i in convmap)) {
            break;
        }
        if (convmap[new_i].length > 1) {
            break;
        }
        if (convmap[new_i][0] != new_o) {
            break;
        }

        conv_i = new_i;
        conv_o = new_o;
    }

    // [start_i,conv_i] maps to [start_o,conv_o], ignore ranges of 1 char.
    var count = (conv_i - start_i) / skip + 1;
    if (count <= 1) {
        return null;
    }

    // We have an acceptable range, remove them from the convmap here.
    for (let cp = start_i; cp <= conv_i; cp += skip) {
        delete convmap[cp];
    }

    return { start_i, start_o, count };
}

// Find the first range with a certain skip value.  Delete the range
// if found.
function findFirstRangeWithSkip(convmap, skip) {
    for (let cp = 0; cp < 65536; cp++) {
        let res = scanRangeWithSkip(convmap, cp, skip);
        if (res) {
            return res;
        }
    }
    return null;
}

// Generate bit-packed case conversion table for a given conversion map.
//
// The bitstream encoding is based on manual inspection for whatever
// regularity the Unicode case conversion rules have.
//
// Start with a full description of (BMP) case conversions which does not
// cover all codepoints; unmapped codepoints convert to themselves.
// Scan for range-to-range mappings with a range of skips starting from 1.
// Whenever a valid range is found, remove it from the map.  Finally,
// output the remaining case conversions (1:1 and 1:n) on a per codepoint
// basis.  This is very slow because we always scan from scratch, but it's
// the most reliable and simple way to scan.
function scanCaseconvTables(convmap) {
    console.debug('scan caseconv tables');

    var ranges = [];   // range mappings (2 or more consecutive mappings with a certain skip)
    var singles = [];  // 1:1 character mappings
    var multis = [];   // 1:n character mappings

    // Ranges with skips.

    for (let skip = 1; skip <= SKIP_MAX; skip++) {
        for (;;) {
            let res = findFirstRangeWithSkip(convmap, skip);
            if (!res) {
                break;
            }
            console.debug('- range: skip ' + skip + ': ' + res.start_i + ' ' + res.start_o + ' ' + res.count);
            ranges.push({
                start_i: res.start_i,
                start_o: res.start_o,
                count: res.count,
                skip: skip
            });
        }
    }

    // 1:1 conversions.

    numericSort(Object.keys(convmap)).forEach((cp) => {
        if (convmap[cp].length > 1) {
            return;
        }
        console.debug('- 1-to-1: ' + cp + ' ' + convmap[cp][0]);
        singles.push({ cp_i: cp, cp_o: convmap[cp][0] });
        delete convmap[cp];
    });

    /* There are many mappings to 2-char sequences with latter char being U+0399.
     * These could be handled as a special case, but we don't do that right now.
     * For example:
     *
     * [8064L, u'\u1f08\u0399']
     * [8065L, u'\u1f09\u0399']
     * [8066L, u'\u1f0a\u0399']
     * [8067L, u'\u1f0b\u0399']
     * [8068L, u'\u1f0c\u0399']
     * [8069L, u'\u1f0d\u0399']
     * [8070L, u'\u1f0e\u0399']
     * [8071L, u'\u1f0f\u0399']
     * ...
     */

    // 1:n conversions.

    numericSort(Object.keys(convmap)).forEach((cp) => {
        if (convmap[cp].length <= 1) {
            throw new TypeError('internal error, convmap still has a <= 1 mapping');
        }
        console.debug('- 1-to-n: ' + cp + ' ' + convmap[cp]);
        multis.push({ cp_i: cp, cpseq_o: convmap[cp] });
        delete convmap[cp];
    });

    console.debug('- range mappings: ' + ranges.length);
    console.debug('- 1-to-1 mappings: ' + singles.length);
    console.debug('- 1-to-n mappings: ' + multis.length);
    if (Object.keys(convmap).length > 0) {
        throw new TypeError('internal error, convmap not empty after caseconv processing');
    }

    return { ranges, singles, multis };
}

function bitpackCaseconvTables(ranges, singles, multis) {
    console.debug('bitpack caseconv tables');

    var be = new BitEncoder();
    var count;

    for (let curr_skip = 1; curr_skip <= SKIP_MAX; curr_skip++) {
        count = 0;
        ranges.forEach((r) => {
            if (r.skip !== curr_skip) {
                return;
            }
            count++;
        });
        assert(count < 0x3f);
        be.bits(count, 6);
        console.debug('- encode: skip=' + curr_skip + ', count=' + count);

        ranges.forEach((r) => {
            if (r.skip !== curr_skip) {
                return;
            }
            be.bits(r.start_i, 16);
            be.bits(r.start_o, 16);
            be.bits(r.count, 7);
        });
    }
    be.bits(0x3f, 6);  // end of skip ranges

    count = singles.length;
    be.bits(count, 7);
    singles.forEach((s) => {
        be.bits(s.cp_i, 16);
        be.bits(s.cp_o, 16);
    });

    count = multis.length;
    be.bits(count, 7);
    multis.forEach((m) => {
        let cp_i = m.cp_i;
        let cpseq_o = m.cpseq_o;
        be.bits(cp_i, 16);
        be.bits(cpseq_o.length, 2);  // At most 3 codepoints.
        for (let i = 0; i < cpseq_o.length; i++) {
            be.bits(cpseq_o[i], 16);
        }
    });

    console.debug('caseconv table: ' + be.getStatsString());
    return be;
}

function removeArrayNulls(arr) {
    for (let i = 0; i < arr.length; i++) {
        if (arr[i] === null) {
            delete arr[i];
        }
    }
    return arr;
}

function generateCaseconvTables(convmap) {
    console.debug('generate caseconv tables');
    var t = scanCaseconvTables(removeArrayNulls(jsonDeepClone(convmap)));
    var be = bitpackCaseconvTables(t.ranges, t.singles, t.multis);
    return { data: be.getBytes() };
}
exports.generateCaseconvTables = generateCaseconvTables;
