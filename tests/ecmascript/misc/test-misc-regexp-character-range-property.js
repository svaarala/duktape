/*
 *  Test the property of built-in character ranges mentioned in
 *  doc/regular-expressions.txt: if a character x is contained
 *  in the range, its normalized counterpart is also contained
 *  in the range.
 *
 *  This is not a test of the language implementation, so do not
 *  expect variance among implementations.
 *
 */

var ignoreCase = true;
var i, j;

/*===
checking complement property of \d and \D ranges...
checking complement property of \s and \S ranges...
checking complement property of \w and \W ranges...
checking range normalization property for \d...
checking range normalization property for \s...
checking range normalization property for \w...
checking range normalization property for \D...
checking range normalization property for \S...
checking range normalization property for \W...
checking range normalization property for period (".")...
===*/

/* E5 Section 15.10.2.8 */
function canonicalize(x) {
    var t, u;

    if (!ignoreCase) {
        return x;
    }
    t = String.fromCharCode(x);
    t = t.toUpperCase();
    if (t.length != 1) {
        return x;
    }
    u = t.charCodeAt(0);
    if (x >= 128 && u < 128) {
        return x;
    }
    return u;
}

for (i = 0; i < 65536; i++) {
    j = canonicalize(i);
    /*
    if (i != j) {
        print(i, '->', j);
    }
    */
}

function inranges(x, ranges) {
    var i;

    for (i = 0; i < ranges.length; i++) {
        var r1 = ranges[i][0];
        var r2 = ranges[i][1];
        if (x >= r1 && x <= r2) {
            return true;
        }
    }

    return false;
}

/* check the 'normalized char already included in range' property */
function check_normalization_property(ranges) {
    var i, j, k;

    for (i = 0; i < ranges.length; i++) {
        var r1 = ranges[i][0];
        var r2 = ranges[i][1];
        for (j = r1; j <= r2; j++) {
            k = canonicalize(j);
            if (!inranges(k, ranges)) {
                throw new Error('canonicalized counterpart of character ' + j + ' not in range set')
            }
        }
    }
}

/* check range complement property */
function check_complement_property(ranges1, ranges2) {
    var i;

    for (i = 0; i < 65536; i++) {
        var p1 = inranges(i, ranges1);
        var p2 = inranges(i, ranges2);
        if (p1 === p2) {
            throw new Error('character ' + i + ' included/excluded from both a character set and its complement')
        }
    }
}

var ranges_pos_d = [
    [ 0x0030, 0x0039 ],
];

var ranges_pos_s = [
    [ 0x0009, 0x000D ],
    [ 0x0020, 0x0020 ],
    [ 0x00A0, 0x00A0 ],
    [ 0x1680, 0x1680 ],
    [ 0x180E, 0x180E ],
    [ 0x2000, 0x200A ],
    [ 0x2028, 0x2029 ],
    [ 0x202F, 0x202F ],
    [ 0x205F, 0x205F ],
    [ 0x3000, 0x3000 ],
    [ 0xFEFF, 0xFEFF ],
];

var ranges_pos_w = [
    [ 0x0030, 0x0039 ],
    [ 0x0041, 0x005A ],
    [ 0x005F, 0x005F ],
    [ 0x0061, 0x007A ],
];

var ranges_neg_d = [
    [ 0x0000, 0x002F ],
    [ 0x003A, 0xFFFF ],
];

var ranges_neg_s = [
    [ 0x0000, 0x0008 ],
    [ 0x000E, 0x001F ],
    [ 0x0021, 0x009F ],
    [ 0x00A1, 0x167F ],
    [ 0x1681, 0x180D ],
    [ 0x180F, 0x1FFF ],
    [ 0x200B, 0x2027 ],
    [ 0x202A, 0x202E ],
    [ 0x2030, 0x205E ],
    [ 0x2060, 0x2FFF ],
    [ 0x3001, 0xFEFE ],
    [ 0xFF00, 0xFFFF ],
];

var ranges_neg_w = [
    [ 0x0000, 0x002F ],
    [ 0x003A, 0x0040 ],
    [ 0x005B, 0x005E ],
    [ 0x0060, 0x0060 ],
    [ 0x007B, 0xFFFF ],
];

var ranges_pos_period = [
    [ 0x0000, 0x0009 ],
    [ 0x000B, 0x000C ],
    [ 0x000E, 0x2027 ],
    [ 0x202A, 0xFFFF ],
];

print('checking complement property of \\d and \\D ranges...');
check_complement_property(ranges_pos_d, ranges_neg_d);
print('checking complement property of \\s and \\S ranges...');
check_complement_property(ranges_pos_s, ranges_neg_s);
print('checking complement property of \\w and \\W ranges...');
check_complement_property(ranges_pos_w, ranges_neg_w);

print('checking range normalization property for \\d...');
check_normalization_property(ranges_pos_d);
print('checking range normalization property for \\s...');
check_normalization_property(ranges_pos_s);
print('checking range normalization property for \\w...');
check_normalization_property(ranges_pos_w);
print('checking range normalization property for \\D...');
check_normalization_property(ranges_neg_d);
print('checking range normalization property for \\S...');
check_normalization_property(ranges_neg_s);
print('checking range normalization property for \\W...');
check_normalization_property(ranges_neg_w);
print('checking range normalization property for period (".")...');
check_normalization_property(ranges_pos_period);
