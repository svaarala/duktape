// WTF-8 sanitization algorithm used for string interning in Duktape 3.x.
// Must match the C code behavior exactly for all input byte sequences.
//
// https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Surrogates
// https://en.wikipedia.org/wiki/UTF-8#WTF-8
// https://simonsapin.github.io/wtf-8/
// https://en.wikipedia.org/wiki/CESU-8

'use strict';

function encodeCodepoint(result, cp, useCesu8) {
    if (cp <= 0x7f) {
        result.push(cp);
    } else if (cp <= 0x7ff) {
        result.push(0b11000000 + (cp >>> 6));
        result.push(0b10000000 + (cp & 0x3f));
    } else if (cp <= 0xffff) {
        result.push(0b11100000 + (cp >>> 12));
        result.push(0b10000000 + ((cp >>> 6) & 0x3f));
        result.push(0b10000000 + (cp & 0x3f));
    } else if (cp <= 0x10ffff) {
        if (useCesu8) {
            // Actually cold code now, caller handles CESU-8 surrogate pair split.
            let cp2 = cp - 0x10000;
            let hi = 0xd800 + (cp2 >>> 10);
            let lo = 0xdc00 + (cp2 & 0x3ff);
            encodeCodepoint(result, hi, false);
            encodeCodepoint(result, lo, false);
        } else {
            result.push(0b11110000 + (cp >>> 18));
            result.push(0b10000000 + ((cp >>> 12) & 0x3f));
            result.push(0b10000000 + ((cp >>> 6) & 0x3f));
            result.push(0b10000000 + (cp & 0x3f));
        }
    } else {
        throw new TypeError('internal error, invalid codepoint: ' + cp);
    }
}

function encodeCodepoints(arr, useCesu8) {
    let result = [];
    for (let i = 0; i < arr.length; i++) {
        encodeCodepoint(result, arr[i], useCesu8);
    }
    return result;
}

function getDebugStringForU8(u8) {
    let arr = [];
    for (let i = 0; i < u8.length; i++) {
        let ch = u8[i];
        let c = String.fromCharCode(ch);
        if (ch >= 0x20 && ch <= 0x7e && c !== '<' && c !== '>' && c !== '/' && c !== '\\' && c !== '*') {
            arr.push(c);
        } else {
            arr.push('<' + ch.toString(16) + '>');
        }
    }
    return arr.join('');
}

function getDebugStringForArray(arr) {
    let debugMap = (cp) => {
        let c = String.fromCharCode(cp);
        if (cp >= 0x20 && cp <= 0x7e && c !== '<' && c !== '>' && c !== '/' && c !== '\\' && c !== '*') {
            return c;
        } else {
            return '<' + cp.toString(16) + '>';
        }
    };
    return arr.map(debugMap).join('');
}

function wtf8SanitizeSymbol(u8) {
    // Symbol encoding is not WTF-8.  Even so, runtime charLength must have a
    // consistent value.  For now, charLength for symbols will be the number
    // of non-continuation bytes in the string data (same as for strings before
    // the switch to WTF-8).

    let resultSym = [];
    let charLengthSym = 0;
    for (let i = 0; i < u8.length; i++) {
        let ch = u8[i];
        if (!(ch >= 0b10000000 && ch <= 0b10111111)) {
            charLengthSym++;
        }
        resultSym.push(ch);
    }

    let outputUint8ArraySym = new Uint8Array(resultSym);
    let outputDebugStringSym = '<Symbol ' + resultSym.map((ch) => {
        return ('00' + ch.toString(16)).substr(-2);
    }).join(' ') + '>';

    return {
        isSymbol: true,
        inputUint8Array: new Uint8Array(u8),
        inputDebugString: getDebugStringForU8(u8),
        outputUint8ArrayWtf8: outputUint8ArraySym,
        outputUint8ArrayCesu8: outputUint8ArraySym,
        outputDebugStringWtf8: outputDebugStringSym,
        outputDebugStringCesu8: outputDebugStringSym,
        byteLengthWtf8: outputUint8ArraySym.length,
        byteLengthCesu8: outputUint8ArraySym.length,
        charLengthWtf8: charLengthSym,
        charLengthCesu8: charLengthSym,

        // Stats are dummy for symbols.
        asciiCount: 0,
        bmpCount: 0,
        nonBmpCount: 0,
        combinedSurrogateCount: 0,
        unpairedSurrogateCount: 0,
        replacementCharCount: 0
    };
}

function wtf8SanitizeString(u8) {
    // Non-symbol, apply normal sanitization algorithm.

    let asciiCount = 0; // ASCII codepoints [U+0000,U+007F].
    let bmpCount = 0; // Non-ASCII BMP codepoints [U+0080,U+FFFF].
    let nonBmpCount = 0; // Non-BMP (supplemental) codepoints [U+10000,U+10FFFF].
    let combinedSurrogateCount = 0; // Number of valid surrogate pairs combined.
    let unpairedSurrogateCount = 0; // Number of unpaired surrogate chars encountered.
    let replacementCharCount = 0; // Number of replacement chars emitted (included in bmpCount too).
    let codepointResultWtf8 = []; // WTF-8 result as a codepoint array.
    let codepointResultCesu8 = []; // (Loose) CESU-8 result as a codepoint array.

    function pushReplacement() {
        //console.log('pushReplacement');

        let cp = 0xfffd;
        codepointResultWtf8.push(cp);
        codepointResultCesu8.push(cp);
        replacementCharCount++;
    }

    function pushWtf8(cp) {
        //console.log('pushWtf8:', cp);

        // Categorize into ASCII, BMP, non-BMP.
        if (cp <= 0x7f) {
            asciiCount++;
        } else if (cp <= 0xffff) {
            bmpCount++;
        } else if (cp <= 0x10ffff) {
            nonBmpCount++;
        } else {
            throw new TypeError('internal error, invalid codepoint: ' + cp);
        }

        if (cp < 0x10000) {
            codepointResultWtf8.push(cp);
            codepointResultCesu8.push(cp);
        } else if (cp <= 0x10ffff) {
            let cp2 = cp - 0x10000;
            let hi = 0xd800 + (cp2 >>> 10);
            let lo = 0xdc00 + (cp2 & 0x3ff);

            codepointResultWtf8.push(cp);
            codepointResultCesu8.push(hi);
            codepointResultCesu8.push(lo);
        } else {
            throw new TypeError('internal error, invalid codepoint: ' + cp);
        }
    }

    for (let i = 0; i < u8.length;) {
        //console.log('wtf8 loop:', i);

        //let iStart = i;
        let ch = u8[i++];

        /* Decoder based on https://encoding.spec.whatwg.org/#utf-8-decoder
         * in "replacement" mode, except that:
         *   - U+D800 to U+DFFF (encoded ED A0 80 to ED BF BF) are allowed
         *     to reach WTF-8 surrogate pair processing.
         *
         * Note in particular that non-canonical encodings (codepoint value
         * too low or too high) must be terminated on the first invalid byte,
         * not after decoding the whole N-byte sequence.
         *
         * Maximum input expansion is 3x from an invalid initial byte expanding
         * to 3 bytes of UTF-8 encoded U+FFFD replacement character.
         */

        if (ch <= 0x7f) {
            pushWtf8(ch);
            continue;
        }

        let lower = 0x80;
        let upper = 0xbf;
        let numCont = 0;
        let cp = 0;
        let cpMin = 0;
        let cpMax = 0;

        if (ch >= 0x80 && ch <= 0xbf) {
            // Invalid leading continuation byte.
            pushReplacement();
            continue;
        } else if (ch >= 0xc0 && ch <= 0xc1) {
            // Invalid 2-byte sequence, initial byte too low.
            pushReplacement();
            continue;
        } else if (ch >= 0xc2 && ch <= 0xdf) {
            // 2-byte sequence, valid initial byte.
            numCont = 1;
            cp = ch & 0x1f;
            cpMin = 0x80;
            cpMax = 0x7ff;
            lower = 0x80;
            upper = 0xbf;
        } else if (ch >= 0xe0 && ch <= 0xef) {
            // 3-byte sequence, valid initial byte.
            numCont = 2;
            cp = ch & 0x0f;
            cpMin = 0x800;
            cpMax = 0xffff;
            lower = (ch === 0xe0 ? 0xa0 : 0x80);
            upper = 0xbf;
            // This would be the case for TextDecoder to reject U+D800 to U+DFFF,
            // but we must allow them.
            //upper = (ch === 0xed ? 0x9f : 0xbf);
        } else if (ch >= 0xf0 && ch <= 0xf4) {
            // 4-byte sequence, valid initial byte.
            numCont = 3;
            cp = ch & 0x07;
            cpMin = 0x10000;
            cpMax = 0x10ffff;
            lower = (ch === 0xf0 ? 0x90 : 0x80);
            upper = (ch === 0xf4 ? 0x8f : 0xbf);
        } else if (ch >= 0xf5 && ch <= 0xf7) {
            // Invalid 4-byte sequence, initial byte too high.
            pushReplacement();
            continue;
        } else {
            // Invalid UTF-8 sequence, invalid initial byte.
            pushReplacement();
            continue;
        }

        let brokenEncoding = false;
        while (numCont-- > 0) {
            if (i >= u8.length) {
                // Out of bounds, truncated encoding.  Replace everything
                // so far with one replacement char.  (Can also get 3x
                // expansion for the last byte here.)
                brokenEncoding = true;
                break;
            }

            let cb = u8[i++];
            if (cb >= lower && cb <= upper) {
                cp = (cp << 6) + (cb & 0x3f);
                lower = 0x80;
                upper = 0xbf;
            } else {
                // Encoding broken at current index, replace everything so
                // far (excluding the broken byte) with one replacement
                // character and backtrack, reinterpreting the broken byte.
                i--;
                brokenEncoding = true;
                break;
            }
        }

        if (brokenEncoding) {
            // Encoding is broken, replace the maximal valid sequence with a
            // single replacement character.
            pushReplacement();
            continue;
        }

        if (cp < cpMin || cp > cpMax) {
            // This should never happen because the upper/lower continuation
            // byte should restrict the codepoint to the allowed range.
            throw new TypeError('internal error: codepoint not within [cpMin,cpMax]: ' + cp + ' vs ' + cpMin + '-' + cpMax);
            //pushReplacement();
            //continue;
        }

        // Successfully decoded a UTF-8 codepoint.  We will either:
        // - keep the codepoint (and its UTF-8 encoding) as is, or
        // - if the codepoint is a high surrogate and a valid low
        //   surrogate follows, combine the two codepoints (WTF-8).

        if (cp >= 0xdc00 && cp <= 0xdfff) {
            // Unpaired low surrogate, keep as is.
            unpairedSurrogateCount++;
            pushWtf8(cp);
            continue;
        }

        if (!(cp >= 0xd800 && cp <= 0xdbff)) {
            // Not a high surrogate, keep as is.
            pushWtf8(cp);
            continue;
        }

        // First codepoint is a high surrogate.  Try to decode the
        // following codepoint.  Here we can use a CESU-8 decoder
        // optimized for detecting low surrogates (U+DC00 to U+DFFF).
        //
        // >>> u'\ud7ff'.encode('utf-8')
        // '\xed\x9f\xbf'
        // >>> u'\ud800'.encode('utf-8')
        // '\xed\xa0\x80'
        // >>> u'\udc00'.encode('utf-8')
        // '\xed\xb0\x80'
        // >>> u'\udfff'.encode('utf-8')
        // '\xed\xbf\xbf'
        // >>> u'\ue000'.encode('utf-8')
        // '\xee\x80\x80'

        let bytesLeft = u8.length - i;
        let validSurrogatePair = (bytesLeft >= 3 &&
            u8[i] === 0xed &&
            u8[i + 1] >= 0xa0 && u8[i + 1] <= 0xbf &&
            u8[i + 2] >= 0x80 && u8[i + 2] <= 0xbf);
        if (validSurrogatePair) {
            // Valid low surrogate follows.  Decode and combine.
            let hi = cp;
            let lo = (u8[i] - 0b11100000) * 64 * 64 +
                (u8[i + 1] - 0b10000000) * 64 +
                (u8[i + 2] - 0b10000000);
            cp = 0x10000 + (hi - 0xd800) * 0x400 + (lo - 0xdc00);
            i += 3;
            combinedSurrogateCount++;
            pushWtf8(cp);
        } else {
            // Not a valid surrogate, emit 'cp' as is.
            unpairedSurrogateCount++;
            pushWtf8(cp);
            continue;
        }
    }

    // WTF-8 result, matches Duktape 3.x runtime.
    let resultWtf8 = encodeCodepoints(codepointResultWtf8, false);
    let outputUint8ArrayWtf8 = new Uint8Array(resultWtf8);
    let outputDebugStringWtf8 = getDebugStringForArray(codepointResultWtf8);

    // CESU-8 result.
    let resultCesu8 = encodeCodepoints(codepointResultCesu8, true);
    let outputUint8ArrayCesu8 = new Uint8Array(resultCesu8);
    let outputDebugStringCesu8 = getDebugStringForArray(codepointResultCesu8);

    return {
        isSymbol: false,
        inputUint8Array: new Uint8Array(u8),
        inputDebugString: getDebugStringForU8(u8),
        outputUint8ArrayWtf8,
        outputUint8ArrayCesu8,
        outputCodepointsWtf8: codepointResultWtf8,
        outputCodepointsCesu8: codepointResultCesu8,
        outputDebugStringWtf8,
        outputDebugStringCesu8,
        byteLengthWtf8: outputUint8ArrayWtf8.length,
        byteLengthCesu8: outputUint8ArrayCesu8.length,
        charLengthWtf8: codepointResultWtf8.length,
        charLengthCesu8: codepointResultCesu8.length,
        asciiCount,
        bmpCount,
        nonBmpCount,
        combinedSurrogateCount,
        unpairedSurrogateCount,
        replacementCharCount
    };
}

function sanitizeToWtf8(u8, args) {
    if (!(typeof u8 === 'object' && u8 !== null && u8 instanceof Uint8Array)) {
        throw new TypeError('input must be a Uint8Array');
    }

    // Special check for symbol strings.
    let allowSymbol = !(args && args.allowSymbol === false);
    if (allowSymbol && u8.length >= 1 && (u8[0] === 0x80 || u8[0] === 0x81 || u8[0] === 0x82 || u8[0] === 0xff)) {
        return wtf8SanitizeSymbol(u8);
    } else {
        return wtf8SanitizeString(u8);
    }
}

// From http://unicode.org/review/pr-121.html:
//
// The following table illustrates the application of these alternative policies
// for an example of conversion of UTF-8 to UTF-16, the most common kind of
// conversion for which the differences are apparent and for which a recommended
// practice would be desirable for interoperability:
//
//       61      F1      80      80      E1      80      C2      62
// 1   U+0061  U+FFFD                                          U+0062
// 2   U+0061  U+FFFD                  U+FFFD          U+FFFD  U+0062
// 3   U+0061  U+FFFD  U+FFFD  U+FFFD  U+FFFD  U+FFFD  U+FFFD  U+0062
//
// The UTC has indicated a tentative preference for option #2, but is interested
// in feedback on what would be the best recommended practice, and reasons for that
// choice. The UTC also requests feedback about which products or libraries are
// known to follow these or other policies for replacement of ill-formed subsequences
// on conversion or validation.

function testPr121() {
    // Example given in PR-121.
    let res = sanitizeToWtf8(new Uint8Array([0x61, 0xf1, 0x80, 0x80, 0xe1, 0x80, 0xc2, 0x62]));
    //console.log(res);
    if (res.outputDebugStringWtf8 !== 'a<fffd><fffd><fffd>b') {
        //console.log(res);
        throw new TypeError('wtf8 sanitize self test failed');
    }
}

function testUnicode110000() {
    let res = sanitizeToWtf8(new Uint8Array([0x41, 0xf4, 0x90, 0x80, 0x80]));
    //console.log(res);
    if (res.outputDebugStringWtf8 !== 'A<fffd><fffd><fffd><fffd>') {
        //console.log(res);
        throw new TypeError('wtf8 sanitize self test failed');
    }
}

function testByteRanges() {
    // Some basic boundary testing of valid byte ranges.
    // Also compare against TextDecoder() output which should match
    // for decoding and replacement character behavior except when
    // U+D800 to U+DFFF are involved.

    let R = 'REPLACEMENT';
    let I = 'INPUT';
    let tests = [
        { input: [ 0x00 ], output: [ I ] },
        { input: [ 0x40 ], output: [ I ] },
        { input: [ 0x7f ], output: [ I ] },

        { input: [ 0x80 ], output: [ R ] },
        { input: [ 0x80, 0x80 ], output: [ R, R ] },
        { input: [ 0xa0 ], output: [ R ] },
        { input: [ 0xbf ], output: [ R ] },
        { input: [ 0xbf, 0xbf ], output: [ R, R ] },

        { input: [ 0xc0 ], output: [ R ] },
        { input: [ 0xc0, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xc0, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xc0, 0x80 ], output: [ R, R ] },
        { input: [ 0xc0, 0xbf ], output: [ R, R ] },
        { input: [ 0xc0, 0xc0 ], output: [ R, R ] },

        { input: [ 0xc1 ], output: [ R ] },
        { input: [ 0xc1, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xc1, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xc1, 0x80 ], output: [ R, R ] },
        { input: [ 0xc1, 0xbf ], output: [ R, R ] },
        { input: [ 0xc1, 0xc0 ], output: [ R, R ] },

        { input: [ 0xc2, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xc2, 0x80 ], output: [ I ] },  // smallest valid 2-byte: U+0080
        { input: [ 0xc2, 0xbf ], output: [ I ] },
        { input: [ 0xc2, 0xc0 ], output: [ R, R ] },

        { input: [ 0xc3, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xc3, 0x80 ], output: [ I ] },
        { input: [ 0xc3, 0xbf ], output: [ I ] },
        { input: [ 0xc3, 0xc0 ], output: [ R, R ] },

        { input: [ 0xdf, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xdf, 0x80 ], output: [ I ] },
        { input: [ 0xdf, 0xbf ], output: [ I ] },  // highest valid 2-byte: U+07FF
        { input: [ 0xdf, 0xc0 ], output: [ R, R ] },

        { input: [ 0xe0 ], output: [ R ] },
        { input: [ 0xe0, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xe0, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xe0, 0x80 ], output: [ R, R ] },
        { input: [ 0xe0, 0x80, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xe0, 0x80, 0xbf ], output: [ R, R, R ] },
        { input: [ 0xe0, 0x9f ], output: [ R, R ] },
        { input: [ 0xe0, 0x9f, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xe0, 0x9f, 0xbf ], output: [ R, R, R ] },
        { input: [ 0xe0, 0xa0 ], output: [ R ] },  // lower limit for 2nd byte 0xa0 so valid here (but truncated)
        { input: [ 0xe0, 0xa0, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xe0, 0xa0, 0x80 ], output: [ I ] },  // smallest valid 3-byte: 0x800
        { input: [ 0xe0, 0xa0, 0xbf ], output: [ I ] },
        { input: [ 0xe0, 0xbf, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xe0, 0xbf, 0x80 ], output: [ I ] },
        { input: [ 0xe0, 0xbf, 0xbf ], output: [ I ] },

        { input: [ 0xe1 ], output: [ R ] },
        { input: [ 0xe1, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xe1, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xe1, 0x80 ], output: [ R ] },
        { input: [ 0xe1, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xe1, 0x80, 0xbf ], output: [ I ] },
        { input: [ 0xe1, 0x9f ], output: [ R ] },
        { input: [ 0xe1, 0x9f, 0x80 ], output: [ I ] },
        { input: [ 0xe1, 0x9f, 0xbf ], output: [ I ] },
        { input: [ 0xe1, 0xa0 ], output: [ R ] },
        { input: [ 0xe1, 0xa0, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xe1, 0xa0, 0x80 ], output: [ I ] },
        { input: [ 0xe1, 0xa0, 0xbf ], output: [ I ] },
        { input: [ 0xe1, 0xbf, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xe1, 0xbf, 0x80 ], output: [ I ] },
        { input: [ 0xe1, 0xbf, 0xbf ], output: [ I ] },

        // Unlike with TextDecoder(), initial byte 0xED does not cause 2nd
        // byte to have upper limit 0x9F because U+D800 to U+DFFF must be
        // allowed for WTF-8 so these pass through as is (when otherwise valid).
        { input: [ 0xed ], output: [ R ] },
        { input: [ 0xed, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xed, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xed, 0x80 ], output: [ R ] },
        { input: [ 0xed, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xed, 0x80, 0xbf ], output: [ I ] },
        { input: [ 0xed, 0x9f ], output: [ R ] },
        { input: [ 0xed, 0x9f, 0x80 ], output: [ I ] },
        { input: [ 0xed, 0x9f, 0xbf ], output: [ I ] },
        { input: [ 0xed, 0xa0 ], output: [ R ], textDecoderOutput: [ R, R ] },  // TextDecoder() rejects the 2nd byte so two replacements
        { input: [ 0xed, 0xa0, 0x7f ], output: [ R, 0x7f ], textDecoderOutput: [ R, R, 0x7f ] },
        { input: [ 0xed, 0xa0, 0x80 ], output: [ I ], textDecoderOutput: [ R, R, R ] },
        { input: [ 0xed, 0xa0, 0xbf ], output: [ I ], textDecoderOutput: [ R, R, R ] },
        { input: [ 0xed, 0xbf, 0x7f ], output: [ R, 0x7f ], textDecoderOutput: [ R, R, 0x7f ] },
        { input: [ 0xed, 0xbf, 0x80 ], output: [ I ], textDecoderOutput: [ R, R, R ] },
        { input: [ 0xed, 0xbf, 0xbf ], output: [ I ], textDecoderOutput: [ R, R, R ] },

        // Valid surrogate pairs get combined.  Just a few point checks here.
        { input: [ 0xed, 0xa0, 0x80, 0xed, 0xb0, 0x80 ], output: [ 0xf0, 0x90, 0x80, 0x80 ], textDecoderOutput: [ R, R, R, R, R, R ] },  // U+D800 U+DC00 => U+10000
        { input: [ 0xed, 0xa0, 0x80, 0xed, 0xbf, 0xbf ], output: [ 0xf0, 0x90, 0x8f, 0xbf ], textDecoderOutput: [ R, R, R, R, R, R ] },  // U+D800 U+DFFF => U+103FF
        { input: [ 0xed, 0xaf, 0xbf, 0xed, 0xb0, 0x80 ], output: [ 0xf4, 0x8f, 0xb0, 0x80 ], textDecoderOutput: [ R, R, R, R, R, R ] },  // U+DBFF U+DC00 => U+10FC00
        { input: [ 0xed, 0xaf, 0xbf, 0xed, 0xbf, 0xbf ], output: [ 0xf4, 0x8f, 0xbf, 0xbf ], textDecoderOutput: [ R, R, R, R, R, R ] },  // U+DBFF U+DFFF => U+10FFFF

        { input: [ 0xee ], output: [ R ] },
        { input: [ 0xee, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xee, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xee, 0x80 ], output: [ R ] },
        { input: [ 0xee, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xee, 0x80, 0xbf ], output: [ I ] },
        { input: [ 0xee, 0x9f ], output: [ R ] },
        { input: [ 0xee, 0x9f, 0x80 ], output: [ I ] },
        { input: [ 0xee, 0x9f, 0xbf ], output: [ I ] },
        { input: [ 0xee, 0xa0 ], output: [ R ] },
        { input: [ 0xee, 0xa0, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xee, 0xa0, 0x80 ], output: [ I ] },
        { input: [ 0xee, 0xa0, 0xbf ], output: [ I ] },
        { input: [ 0xee, 0xbf, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xee, 0xbf, 0x80 ], output: [ I ] },
        { input: [ 0xee, 0xbf, 0xbf ], output: [ I ] },

        { input: [ 0xef ], output: [ R ] },
        { input: [ 0xef, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xef, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xef, 0x80 ], output: [ R ] },
        { input: [ 0xef, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xef, 0x80, 0xbf ], output: [ I ] },
        { input: [ 0xef, 0x9f ], output: [ R ] },
        { input: [ 0xef, 0x9f, 0x80 ], output: [ I ] },
        { input: [ 0xef, 0x9f, 0xbf ], output: [ I ] },
        { input: [ 0xef, 0xa0 ], output: [ R ] },
        { input: [ 0xef, 0xa0, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xef, 0xa0, 0x80 ], output: [ I ] },
        { input: [ 0xef, 0xa0, 0xbf ], output: [ I ] },
        { input: [ 0xef, 0xbf, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xef, 0xbf, 0x80 ], output: [ I ] },
        { input: [ 0xef, 0xbf, 0xbf ], output: [ I ] },

        { input: [ 0xf0 ], output: [ R ] },
        { input: [ 0xf0, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xf0, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xf0, 0x80 ], output: [ R, R ] },
        { input: [ 0xf0, 0x80, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf0, 0x80, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf0, 0x8f ], output: [ R, R ] },
        { input: [ 0xf0, 0x8f, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf0, 0x8f, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf0, 0x90 ], output: [ R ] },  // lower limit for 2nd byte 0x90 so valid here (but truncated)
        { input: [ 0xf0, 0x90, 0x80 ], output: [ R ] },
        { input: [ 0xf0, 0x90, 0x80, 0x80 ], output: [ I ] },  // smallest valid 4-byte: U+10000
        { input: [ 0xf0, 0xbf, 0xbf, 0xbf ], output: [ I ] },
        { input: [ 0xf0, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },

        { input: [ 0xf1 ], output: [ R ] },
        { input: [ 0xf1, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xf1, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xf1, 0x80 ], output: [ R ] },
        { input: [ 0xf1, 0x80, 0x80 ], output: [ R ] },
        { input: [ 0xf1, 0x80, 0x80, 0x80 ], output: [ I ] },  // U+40000
        { input: [ 0xf1, 0x8f ], output: [ R ] },
        { input: [ 0xf1, 0x8f, 0x80 ], output: [ R ] },
        { input: [ 0xf1, 0x8f, 0x80, 0x80 ], output: [ I ] },  // U+4F000
        { input: [ 0xf1, 0x90 ], output: [ R ] },
        { input: [ 0xf1, 0x90, 0x80 ], output: [ R ] },
        { input: [ 0xf1, 0x90, 0x80, 0x80 ], output: [ I ] },  // U+50000
        { input: [ 0xf1, 0xbf, 0xbf, 0xbf ], output: [ I ] },
        { input: [ 0xf1, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },

        { input: [ 0xf3 ], output: [ R ] },
        { input: [ 0xf3, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xf3, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xf3, 0x80 ], output: [ R ] },
        { input: [ 0xf3, 0x80, 0x80 ], output: [ R ] },
        { input: [ 0xf3, 0x80, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xf3, 0x8f ], output: [ R ] },
        { input: [ 0xf3, 0x8f, 0x80 ], output: [ R ] },
        { input: [ 0xf3, 0x8f, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xf3, 0x90 ], output: [ R ] },
        { input: [ 0xf3, 0x90, 0x80 ], output: [ R ] },
        { input: [ 0xf3, 0x90, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xf3, 0xbf, 0xbf, 0xbf ], output: [ I ] },
        { input: [ 0xf3, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },

        { input: [ 0xf4 ], output: [ R ] },
        { input: [ 0xf4, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xf4, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xf4, 0x80 ], output: [ R ] },
        { input: [ 0xf4, 0x80, 0x80 ], output: [ R ] },
        { input: [ 0xf4, 0x80, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xf4, 0x8f ], output: [ R ] },
        { input: [ 0xf4, 0x8f, 0x80 ], output: [ R ] },
        { input: [ 0xf4, 0x8f, 0x80, 0x80 ], output: [ I ] },
        { input: [ 0xf4, 0x90 ], output: [ R, R ] },  // upper limit for 2nd byte 0x8f so invalid here
        { input: [ 0xf4, 0x90, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf4, 0x90, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf4, 0xbf, 0xbf, 0xbf ], output: [ R, R, R, R ] },
        { input: [ 0xf4, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },

        // F5-F7 is technically a leading byte for a 4-byte encoding but
        // encoded values are > U+10FFFF so F5-F7 are rejected.
        { input: [ 0xf5 ], output: [ R ] },
        { input: [ 0xf5, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xf5, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xf5, 0x80 ], output: [ R, R ] },
        { input: [ 0xf5, 0x80, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf5, 0x80, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf5, 0x8f ], output: [ R, R ] },
        { input: [ 0xf5, 0x8f, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf5, 0x8f, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf5, 0x90 ], output: [ R, R ] },
        { input: [ 0xf5, 0x90, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf5, 0x90, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf5, 0xbf, 0xbf, 0xbf ], output: [ R, R, R, R ] },
        { input: [ 0xf5, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },

        { input: [ 0xf7 ], output: [ R ] },
        { input: [ 0xf7, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xf7, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xf7, 0x80 ], output: [ R, R ] },
        { input: [ 0xf7, 0x80, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf7, 0x80, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf7, 0x8f ], output: [ R, R ] },
        { input: [ 0xf7, 0x8f, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf7, 0x8f, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf7, 0x90 ], output: [ R, R ] },
        { input: [ 0xf7, 0x90, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf7, 0x90, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf7, 0xbf, 0xbf, 0xbf ], output: [ R, R, R, R ] },
        { input: [ 0xf7, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },

        // F8-FF are all invalid so rejected as initial byte.
        { input: [ 0xf8 ], output: [ R ] },
        { input: [ 0xf8, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xf8, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xf8, 0x80 ], output: [ R, R ] },
        { input: [ 0xf8, 0x80, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf8, 0x80, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf8, 0x8f ], output: [ R, R ] },
        { input: [ 0xf8, 0x8f, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf8, 0x8f, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf8, 0x90 ], output: [ R, R ] },
        { input: [ 0xf8, 0x90, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xf8, 0x90, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xf8, 0xbf, 0xbf, 0xbf ], output: [ R, R, R, R ] },
        { input: [ 0xf8, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },

        { input: [ 0xff ], output: [ R ] },
        { input: [ 0xff, 0x41 ], output: [ R, 0x41 ] },
        { input: [ 0xff, 0x7f ], output: [ R, 0x7f ] },
        { input: [ 0xff, 0x80 ], output: [ R, R ] },
        { input: [ 0xff, 0x80, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xff, 0x80, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xff, 0x8f ], output: [ R, R ] },
        { input: [ 0xff, 0x8f, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xff, 0x8f, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xff, 0x90 ], output: [ R, R ] },
        { input: [ 0xff, 0x90, 0x80 ], output: [ R, R, R ] },
        { input: [ 0xff, 0x90, 0x80, 0x80 ], output: [ R, R, R, R ] },
        { input: [ 0xff, 0xbf, 0xbf, 0xbf ], output: [ R, R, R, R ] },
        { input: [ 0xff, 0xc0, 0xbf, 0xbf ], output: [ R, R, R, R ] },
    ];

    function prepOutput(output, input) {
        return output.map((v) => {
            if (v === 'REPLACEMENT') { return [ 0xef, 0xbf, 0xbd ]; }
            if (v === 'INPUT') { return input; }
            return [v];
        }).flat();
    }

    for (let { input, output, textDecoderOutput } of tests) {
        let out = prepOutput(output, input);
        let wtf8Res = sanitizeToWtf8(new Uint8Array(input), { allowSymbol: false });
        let res1 = wtf8Res.outputUint8ArrayWtf8;
        if (res1.length !== out.length) {
            //console.log(input, out, res1);
            throw new TypeError('wtf8 sanitize self test failed, length mismatch');
        }
        for (let i = 0; i < res1.length; i++) {
            if (res1[i] !== out[i]) {
                //console.log(input, out, res1);
                throw new TypeError('wtf8 sanitize self test failed, difference at index ' + i);
            }
        }

        let tdOut = prepOutput(textDecoderOutput || output, input);
        let res2 = new TextEncoder().encode(new TextDecoder().decode(new Uint8Array(input)));
        if (res2.length !== tdOut.length) {
            //console.log(input, tdOut, res2);
            throw new TypeError('wtf8 sanitize self test failed, length mismatch for TextDecoder output');
        }
        for (let i = 0; i < res2.length; i++) {
            if (res2[i] !== tdOut[i]) {
                //console.log(input, tdOut, res2);
                throw new TypeError('wtf8 sanitize self test failed, difference at index ' + i + ' for TextDecoder output');
            }
        }
    }
}

function runSelfTests() {
    testPr121();
    testUnicode110000();
    testByteRanges();
}
runSelfTests();

exports.sanitizeToWtf8 = sanitizeToWtf8;
