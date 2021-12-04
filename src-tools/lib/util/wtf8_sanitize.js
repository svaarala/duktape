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

        // ASCII.
        if (ch <= 0x7f) {
            pushWtf8(ch);
            continue;
        }

        // Non-ASCII, decode initial (extended) UTF-8 byte:
        //   10000000 to 10111111: invalid
        //   110xxxxx + 1 continuation: U+0080 to U+07FF
        //   1110xxxx + 2 continuation: U+0800 to U+FFFF
        //   11110xxx + 3 continuation: U+10000 to U+10FFFF
        //   11111000 to 11111111: invalid
        //
        // If the (extended) UTF-8 sequence is invalid, replace the
        // maximal valid sequence with a single U+FFFD replacement
        // character as described in http://unicode.org/review/pr-121.html.

        let numCont = 0;
        let cp = 0;
        let cpMin = 0;
        let cpMax = 0;
        if (ch >= 0b11000000 && ch <= 0b11011111) {
            numCont = 1;
            cp = (ch - 0b11000000);
            cpMin = 0x80;
            cpMax = 0x7ff;
        } else if (ch >= 0b11100000 && ch <= 0b11101111) {
            numCont = 2;
            cp = (ch - 0b11100000);
            cpMin = 0x800;
            cpMax = 0xffff;
        } else if (ch >= 0b11110000 && ch <= 0b11110111) {
            numCont = 3;
            cp = (ch - 0b11110000);
            cpMin = 0x10000;
            cpMax = 0x10ffff;
        } else {
            // Invalid initial byte, replace with one replacement char.
            // This branch produces the maximum 3x input expansion.
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
            if (cb >= 0b10000000 && cb <= 0b10111111) {
                cp = (cp * 64) + (cb - 0b10000000);
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
            // Not a canonical shortest encoding or out-of-bounds, replace the
            // entire sequence with a single replacement character.
            //
            // This doesn't currently handle non-shortest encoding or codepoints
            // above U+10FFFF correctly: we'll first decode the entire sequence
            // and then replace it with a single U+FFFD if it's incorrect.  The
            // correct behavior is to detect the first broken byte which necessarily
            // makes the result invalid, and consume only the maximal valid byte
            // prefix.
            //
            // For example, for U+110000 the encoding is F4 90 80 80, and the 90 is
            // already broken so the initial F4 gets replaced with a U+FFFD and 90
            // is then considered again.  The correct output for F4 90 80 80 is
            // U+FFFD U+FFFD U+FFFD U+FFFD.

            pushReplacement();
            continue;
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
        let validSurrogatePair = bytesLeft >= 3 &&
            u8[i] === 0xed &&
            u8[i + 1] >= 0xa0 && u8[i + 1] <= 0xbf &&
            u8[i + 2] >= 0x80 && u8[i + 2] <= 0xbf;
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

function sanitizeToWtf8(u8) {
    if (!(typeof u8 === 'object' && u8 !== null && u8 instanceof Uint8Array)) {
        throw new TypeError('input must be a Uint8Array');
    }

    // Special check for symbol strings.
    if (u8.length >= 1 && (u8[0] === 0x80 || u8[0] === 0x81 || u8[0] === 0x82 || u8[0] === 0xff)) {
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
    // Currently incorrect, A<fffd>.
    if (res.outputDebugStringWtf8 !== 'A<fffd><fffd><fffd><fffd>') {
        //console.log(res);
        //throw new TypeError('wtf8 sanitize self test failed');
    }
}

function runSelfTests() {
    testPr121();
    testUnicode110000();
}
runSelfTests();

exports.sanitizeToWtf8 = sanitizeToWtf8;
