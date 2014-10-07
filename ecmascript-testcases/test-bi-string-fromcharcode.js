/*===
string 0 
string 1 A
string 2 AB
string 3 ABC
string 3 ABC
string 3 ABC
string 4 AAAA
string 4 AAAA
string 2 AB
valueOf() for 1st
valueOf() for 2nd
valueOf() for 3rd
string 3 BDF
string 7 0 127 255 837 17767 26505 65535
string 3 65 0 0
string 100 ABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJ
===*/

function fromCharCodeTest() {
    function pv(x) {
        print(typeof x, x.length, x);
    }

    function pv2(x) {
        var tmp = [];
        tmp.push(typeof x);
        tmp.push(x.length);
        for (i = 0; i < x.length; i++) {
            tmp.push(x.charCodeAt(i));
        }
        print(tmp.join(' '));
    }

    function mkObj(name, strval, numval) {
        return {
            toString: function() { print('toString() for ' + name); return strval; },
            valueOf: function() { print('valueOf() for ' + name); return numval }
        };
    }

    // basic tests
    pv(String.fromCharCode());
    pv(String.fromCharCode(65));
    pv(String.fromCharCode(65, 66));
    pv(String.fromCharCode(65, 66, 67));

    // ToUint16 coercion, whole numbers
    pv(String.fromCharCode(65601, 6553666, 6553600067));
    pv(String.fromCharCode(-65471, -6553534, -6553599933));

    // ToUint16 coercion, positive values, round to zero (floor)
    pv(String.fromCharCode(65.0, 65.1, 65.5, 65.9));

    // ToUint16 coercion, negative values, round to zero (ceil)
    // -65536 + 65 = -65471
    pv(String.fromCharCode(-65471.0, -65471.1, -65471.5, -65471.9));

    // Note that coercion round to zero means that even if even if
    // ToUint16(65.9) = 65, ToUint16(65.9 - 65536) = ToUint16(-65470.1) = 66!
    pv(String.fromCharCode(65.9, 65.9 - 65536));

    // ToUint16 coercion, side effect order
    pv(String.fromCharCode(mkObj('1st', '65', 66),
                           mkObj('2nd', '67', 68),
                           mkObj('3rd', '69', 70)));

    // non-BMP (and modulo 2**16)
    pv2(String.fromCharCode(0, 127, 255, 0x345, 0x4567, 0x56789, 0x10ffff));

    // undefined arguments are coerced with ToUint16(), yielding zero
    // codepoints; they are different from arguments not given at all

    pv2(String.fromCharCode(65, undefined, undefined));

    // a lot of arguments; this is now limited by Duktape stack so we can't
    // put a REALLY long string here (separate bug test case for longer
    // strings now)
    pv(String.fromCharCode(
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74));
}

try {
    fromCharCodeTest();
} catch (e) {
    print(e);
}
