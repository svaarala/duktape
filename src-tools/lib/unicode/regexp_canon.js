/*
 *  Generate tables for case insensitive RegExp normalization.
 */

'use strict';

const { assert } = require('../util/assert');
const { jsonDeepClone } = require('../util/clone');

// Generate direct canonicalization lookup: codepoint to normalized
// codepoint.
function generateReCanonDirectLookup(convmap) {
    var res = [];
    var highestNonId = -1;
    var countNonId = 0;

    for (let cp = 0; cp < 65536; cp++) {
        let resCp = cp;  // default to as is
        if (convmap[cp]) {
            // If multiple codepoints in result, ignore.
            if (convmap[cp].length === 1) {
                resCp = convmap[cp][0];
            }
        }
        if (cp >= 0x80 && resCp < 0x80) {
            // If non-ASCII mapped to ASCII, ignore.
            resCp = cp;
        }
        if (cp !== resCp) {
            countNonId++;
            highestNonId = cp;
        }
        res.push(resCp);
    }

    // At the moment highestNonId is 65370, which means there's very little
    // gain in assuming 1:1 mapping above a certain BMP codepoint (though we
    // do assume 1:1 mapping for above BMP codepoints).
    console.debug('- ' + countNonId + ' regexp canon non-identity mappings, highest: ' + highestNonId);

    return res;
}
exports.generateReCanonDirectLookup = generateReCanonDirectLookup;

// Figure out which BMP values are never the result of canonicalization.
// Such codepoints are "don't care" in the sense that they are never
// matched against at runtime: ranges are canonicalized at compile time,
// and codepoint being matched is also canonicalized at run time.
function generateReCanonDontCare(canontab) {
    var res = [];
    var dontCareCount = 0;

    while (res.length < 65536) {
        res.push(true);
    }
    canontab.forEach((v, k) => {
        void k;
        res[v] = false;
    });
    res.forEach((v) => {
        if (v) {
            dontCareCount++;
        }
    });

    console.debug('- ' + dontCareCount + ' regexp canon dontcare codepoints');
    return res;
}
exports.generateReCanonDontCare = generateReCanonDontCare;

// Generate maximal continuous ranges for canonicalization.  A continuous
// range is a sequence with N codepoints where IN+i canonicalizes to OUT+i
// for fixed IN, OUT, and i in 0...N-1.  There are unfortunately >1000
// of these ranges, mostly because there are a lot of individual exceptions.
function generateReCanonRanges(canontab) {
    var res = [];

    // Merge adjacent ranges if continuity allows.
    function mergeCompatibleNogap(rng1, rng2) {
        if (rng1[0] + rng1[2] === rng2[0] &&
            rng1[1] + rng1[2] === rng2[1]) {
            return [ rng1[0], rng1[1], rng1[2] + rng2[2] ];
        }
    }

    // Merge ranges in input, returning a possibly smaller set of ranges.
    function mergeRangesPass(input) {
        var ranges = jsonDeepClone(input);

        for (let i = 0; i < ranges.length - 1; i++) {
            let j = i + 1;
            let rng1 = ranges[i];
            let rng2 = ranges[j];
            if (!(rng1 && rng2)) {
                continue;
            }
            let merged = mergeCompatibleNogap(rng1, rng2);
            if (merged) {
                ranges[j] = null;
                ranges[i] = merged;
            }

        }

        return ranges.filter((v) => v !== null);
    }

    // Start with 1 codepoint ranges at first.
    for (let cp = 0; cp < 65536; cp++) {
        res.push([ cp, canontab[cp], 1 ]);  // [ in, out, len ]
    }

    // Merge adjacent ranges until no more ranges can be merged.
    for (;;) {
        let currLen = res.length;
        res = mergeRangesPass(res);
        if (res.length === currLen) {
            break;
        }
    }

    console.debug('- ' + res.length + ' regexp canon ranges');
    return res;
}
exports.generateReCanonRanges = generateReCanonRanges;

// Generate true/false ranges for BMP codepoints where:
// - A codepoint is flagged true if continuity is broken at that point, so
//   an explicit codepoint canonicalization is needed at runtime.
// - A codepoint is flagged false if case conversion is continuous from the
//   previous codepoint, i.e. out_curr = out_prev + 1.
//
// The result is a lot of small ranges due to a lot of small 'false' ranges.
// Reduce the range set by checking if adjacent 'true' ranges have at most
// false_limit 'false' entries between them.  If so, force the 'false'
// entries to 'true' (safe but results in an unnecessary runtime codepoint
// lookup) and merge the three ranges into a larger 'true' range.
function generateReCanonNeedCheck(canontab) {
    // Generate plain map indicating whether canonicalization is continuous
    // at a certain codepoint.
    function generateStraight() {
        var res = [];
        assert(canontab[0] === 0);  // can start from in == out == 0
        var prevIn = -1;
        var prevOut = -1;
        for (let i = 0; i < 65536; i++) {
            let currIn = i;
            let currOut = canontab[i];
            if (prevIn + 1 === currIn && prevOut + 1 === currOut) {
                res[i] = false;
            } else {
                res[i] = true;
            }
            prevIn = currIn;
            prevOut = currOut;
        }
        return res;
    }

    // Generate maximal accurate ranges.
    function generateRanges(data) {
        var prev;
        var count = 0;
        var res = [];
        data.forEach((needcheck) => {
            if (prev === void 0 || prev !== needcheck) {
                if (prev !== void 0) {
                    res.push([ prev, count ]);
                }
                prev = needcheck;
                count = 1;
            } else {
                count++;
            }
        });
        if (prev !== void 0) {
            res.push([ prev, count ]);
        }
        return res;

    }

    // Fill in TRUE-FALSE*N-TRUE gaps into TRUE-TRUE*N-TRUE which is
    // safe (leads to an unnecessary runtime check) but reduces
    // range data size considerably.
    function fillInRanges(data, falseLimit) {
        var res = jsonDeepClone(data);
        for (;;) {
            let found = false;
            for (let i = 0; i < res.length - 2; i++) {
                let r1 = res[i];
                let r2 = res[i + 1];
                let r3 = res[i + 2];
                if (r1[0] === true && r2[0] === false && r3[0] === true &&
                    r2[1] <= falseLimit) {
                    //console.debug('fillin ' + r2[1] + ' falses');
                    void res.splice(i + 1, 2);  // remove r2 and r3
                    res[i] = [ true, r1[1] + r2[1] + r3[1] ];
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
        return res;
    }

    var straight = generateStraight();
    var rangesAccurate = generateRanges(straight);
    var rangesFilledIn = fillInRanges(rangesAccurate, 11);
    console.debug('- regexp canon needcheck, ' + rangesAccurate.length + ' accurate ranges, ' + rangesFilledIn.length + ' filled in ranges');

    return { straight, rangesAccurate, rangesFilledIn };
}
exports.generateReCanonNeedCheck = generateReCanonNeedCheck;

// Generate a bitmap for BMP, divided into N-codepoint blocks, with each
// bit indicating: "entire codepoint block canonicalizes continuously, and
// the block is continuous with the previous and next block".  A 'true'
// entry allows runtime code to just skip the block, advancing 'in' and
// 'out' by the block size, with no codepoint conversion.  The block size
// should be large enough to produce a relatively small lookup table, but
// small enough to reduce codepoint conversions to a manageable number
// because the conversions are (currently) quite slow.  This matters
// especially for case-insensitive RegExps; without any optimization,
// /[\u0000-\uffff]/i requires 65536 case conversions for runtime
// normalization.
function generateReCanonBitmap(canontab) {
    var blockShift = 5;
    var blockSize = 1 << blockShift;
    var blockMask = blockSize - 1;
    var numBlocks = 65536 / blockSize;

    function generateBlockBits(checkContinuity) {
        var res = [];
        while (res.length < numBlocks) {
            res.push(true);
        }
        for (let i = 0; i < numBlocks; i++) {
            let baseIn = i * blockSize;
            let baseOut = canontab[baseIn];
            let lower, upper;
            if (checkContinuity) {
                lower = -1;  // [-1, blockSize]
                upper = blockSize + 1;
            } else {
                lower = 0;   // [0, blockSize-1]
                upper = blockSize;
            }
            for (let j = lower; j < upper; j++) {
                let cpIn = baseIn + j;
                let cpOut = canontab[cpIn];
                let expectOut = baseOut + j;
                if (cpIn >= 0x0000 && cpIn <= 0xffff && cpOut != expectOut) {
                    res[i] = false;  // block is not continuous
                }
            }
        }
        return res;
    }

    function dumpBlockBitmap(bits) {
        var trueCount = 0;
        var falseCount = 0;
        bits.forEach((v) => {
            if (v) {
                trueCount++;
            } else {
                falseCount++;
            }
        });
        var dump = bits.map((v) => v ? 'x' : '.').join('').replace(/.{64}/g, (x) => (x) + '\n');
        console.debug('- regexp canon block bitmap, ' + trueCount + ' continuous blocks, ' + falseCount + ' non-continuous blocks');
        console.debug(dump);
    }

    function convertToBitmap(bits) {
        // C code looks up bits as:
        //   index = codepoint >> blockShift
        //   bitnum = codepoint & blockMask
        //   bitmask = 1 << bitnum
        // So block 0 is mask 0x01 of first byte, block 1 is mask 0x02 of
        // first byte, etc.

        var res = [];
        var curr = 0;
        var mask = 0x01;
        bits.forEach((b) => {
            if (b) {
                curr += mask;
            }
            mask <<= 1;
            if (mask === 0x100) {
                res.push(curr);
                curr = 0;
                mask = 0x01;
            }
        });
        assert(mask === 0x01);  // no leftover
        return res;
    }

    // This is useful to figure out corner case test cases.
    function compareBits(bits1, bits2) {
        console.debug('- regexp canon blocks which are different with and without continuity check');
        assert(bits1.length === bits2.length);
        for (let i = 0; i < bits1.length; i++) {
            if (bits1[i] !== bits2[i]) {
                console.debug('  + block ' + i + ' [' + (i * blockSize) + ',' + (i * blockSize + blockSize - 1) + '] differs');
            }
        }
    }

    var bitsNoContinuity = generateBlockBits(false);
    var bitmapNoContinuity = convertToBitmap(bitsNoContinuity);
    dumpBlockBitmap(bitsNoContinuity);

    var bitsContinuity = generateBlockBits(true);
    var bitmapContinuity = convertToBitmap(bitsContinuity);
    dumpBlockBitmap(bitsContinuity);

    compareBits(bitsNoContinuity, bitsContinuity);

    return { blockShift, blockSize, blockMask, numBlocks,
             bitsNoContinuity, bitsContinuity,
             bitmapNoContinuity, bitmapContinuity };
}
exports.generateReCanonBitmap = generateReCanonBitmap;
