/*===
"\u{}" SyntaxError
"\u{F}" 7 15
"\u{41}" 8 65
"\u{041}" 9 65
"\u{0041}" 10 65
"\u{CaFe}" 10 51966
"\u{1fa49}" 11 55358 56905
"\u{01fa49}" 12 55358 56905
"\u{001fa49}" 13 55358 56905
"\u{0001fa49}" 14 55358 56905
"\u{00001fa49}" SyntaxError
"\u{0010ffff}" 14 56319 57343
"\u{00110000}" 14 65533 65533 65533 65533
"\u{fdec1234}" 14 65533 65533 65533 65533 65533 65533 65533
done
===*/

function test(str) {
    try {
        var t = Duktape.dec('jx', str);
        var arr = [];
        arr.push(str.length);
        for (var i = 0; i < t.length; i++) {
            arr.push(t.charCodeAt(i));
        }
        print(str, arr.join(' '));
    } catch (e) {
        print(str, e.name);
    }
}

test('"\\u{}"');
test('"\\u{F}"');
test('"\\u{41}"');
test('"\\u{041}"');
test('"\\u{0041}"');
test('"\\u{CaFe}"');
test('"\\u{1fa49}"');
test('"\\u{01fa49}"');
test('"\\u{001fa49}"');
test('"\\u{0001fa49}"');
test('"\\u{00001fa49}"');
test('"\\u{0010ffff}"');
test('"\\u{00110000}"'); // Replaced with U+FFFDs
test('"\\u{fdec1234}"'); // Replaced with U+FFFDs

print('done');
