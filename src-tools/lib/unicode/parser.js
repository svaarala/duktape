/*
 *  Parse Unicode data files into an intermediate representation needed
 *  by other tooling.  Retains only codepoint information we actually
 *  need in the other tools; this is not a generic parser.
 *
 *  https://www.unicode.org/reports/tr44/#UnicodeData.txt
 */

'use strict';

function parseHex(v) {
    return Number.parseInt(v, 16);
}

function trimString(v) {
    return v.trim();
}

function parseCodepointSequence(v) {
    v = v.trim();
    if (v === '') {
        return void 0;
    }
    return v.split(' ').map(trimString).map(parseHex);
}

// Parse Unicode data in 'ucd' format, from contents of UnicodeData.txt
// and SpecialCasing.txt.  See https://www.unicode.org/Public/12.1.0/ucd/.
function parseUnicodeText(unicodeData, specialCasing) {
    var haveFirst = null;
    var cpMap = [];  // Gappy array result, codepoint => entry.
    var startTime = Date.now();
    var endTime;

    function checkNoSimpleMappings(cp, parts) {
        var simpleUpperCase = parseCodepointSequence(parts[12]);
        var simpleLowerCase = parseCodepointSequence(parts[13]);
        var simpleTitleCase = parseCodepointSequence(parts[14]);
        if (simpleUpperCase || simpleLowerCase || simpleTitleCase) {
            throw new TypeError('unexpected simple case conversion mappings found for codepoint ' + cp);
        }
    }

    unicodeData.split('\n').forEach(function (line, lineIdx) {
        // Some line types:
        //
        // # Comment.
        // 0034;DIGIT FOUR;Nd;0;EN;;4;4;4;N;;;;;
        // 0041;LATIN CAPITAL LETTER A;Lu;0;L;;;;;N;;;;0061;
        // 01C8;LATIN CAPITAL LETTER L WITH SMALL LETTER J;Lt;0;L;<compat> 004C 006A;;;;N;LATIN LETTER CAPITAL L SMALL J;;01C7;01C9;01C8
        // 3400;<CJK Ideograph Extension A, First>;Lo;0;L;;;;;N;;;;;
        // 4DB5;<CJK Ideograph Extension A, Last>;Lo;0;L;;;;;N;;;;;

        var lineNo = lineIdx + 1;
        if (line === '' || line.startsWith('#')) {
            return;
        }
        var parts = line.split(';');

        // UnicodeData.txt may contain ranges in addition to individual characters.
        // Unpack the ranges into individual characters for the other scripts to use.
        // Range example:
        //
        // 3400;<CJK Ideograph Extension A, First>;Lo;0;L;;;;;N;;;;;
        // 4DB5;<CJK Ideograph Extension A, Last>;Lo;0;L;;;;;N;;;;;

        if (haveFirst) {
            if (!parts[1].endsWith('Last>')) {
                throw new TypeError('cannot parse Unicode range on line ' + lineNo);
            }
            let cp1 = parseHex(haveFirst[0]);
            let gc1 = haveFirst[2];
            let cp2 = parseHex(parts[0]);
            let gc2 = parts[2];
            checkNoSimpleMappings(cp1, haveFirst);
            checkNoSimpleMappings(cp2, parts);
            if (gc1 !== gc2) {
                throw new TypeError('general category mismatch for range starting at codepoint ' + cp1);
            }
            for (let cp = cp1; cp <= cp2; cp++) {
                cpMap[cp] = {
                    cp: cp,
                    gc: gc1,
                    name: haveFirst[1]
                    // Assume no case mappings for range encoded codepoints.
                };
            }
            haveFirst = null;
        } else if (parts[1].endsWith('First>')) {
            haveFirst = parts;
        } else {
            let cp = parseHex(parts[0], 16);
            let simpleUpperCase = parseCodepointSequence(parts[12]);
            let simpleLowerCase = parseCodepointSequence(parts[13]);
            let simpleTitleCase = parseCodepointSequence(parts[14]) || simpleUpperCase;
            let ent = {
                cp: cp,
                gc: parts[2],
                name: parts[1]
            };
            if (simpleUpperCase) {
                ent.uc = simpleUpperCase;
                ent.suc = simpleUpperCase;
            }
            if (simpleLowerCase) {
                ent.lc = simpleLowerCase;
                ent.slc = simpleLowerCase;
            }
            if (simpleTitleCase) {
                ent.tc = simpleTitleCase;
                ent.stc = simpleTitleCase;
            }
            cpMap[cp] = ent;
        }
    });

    if (haveFirst) {
        throw new TypeError('unfinished Unicode range at end of file');
    }

    // Pad with nulls to reach full Unicode length (up to U+10FFFF).
    while (cpMap.length < 0x110000) {
        cpMap.push(null);
    }

    specialCasing.split('\n').forEach(function (line, lineIdx) {
        // # <code>; <lower>; <title>; <upper>; (<condition_list>;)? # <comment>
        var lineNo = lineIdx + 1;
        var m = /^\s*(.*?)\s*(?:#.*)?$/.exec(line);
        if (!m) {
            throw new TypeError('line match failed when parsing SpecialCasing.txt on line ' + lineNo);
        }
        if (m[1] === '') {
            return;
        }
        var parts = m[1].split(';').map(trimString);
        var cp = parseHex(parts[0]);
        var lower = parseCodepointSequence(parts[1]);
        var title = parseCodepointSequence(parts[2]);
        var upper = parseCodepointSequence(parts[3]);
        var ent = cpMap[cp];
        if (!ent) {
            throw new TypeError('unexpected special casing for non-existent codepoint');
        }
        if (parts[4] !== '') {  // special conditions, drop
            return;
        }
        if (lower) {
            ent.lc = lower;
        }
        if (title) {
            ent.tc = title;
        }
        if (upper) {
            ent.uc = upper;
        }
    });

    endTime = Date.now();
    console.debug('parsed unicode data in ' + (endTime - startTime) + ' milliseconds');

    return cpMap;
}
exports.parseUnicodeText = parseUnicodeText;

// Parse Unicode data in 'ucdxml' format, from contents of ucd.all.flat.xml.
// See https://www.unicode.org/Public/12.1.0/ucdxml/.
//
// The flat ucdxml file is very large (> 170MB unpacked) but has all the
// special cases baked in already.  It's too slow to parse on every build,
// but could be used to generate a clean intermediate file or to validate
// the correctness of UnicodeData.txt and SpecialCasing.txt parsing.
/*
function parseUnicodeXml(xmlData) {
    throw new TypeError('unimplemented');
}
exports.parseUnicodeXml = parseUnicodeXml;
*/
