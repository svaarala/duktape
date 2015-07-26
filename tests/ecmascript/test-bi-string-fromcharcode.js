/*
 *  String.fromCharCode()
 *
 *  NOTE: Duktape default behavior is to use ToUint32() instead of ToUint16()
 *  for codepoint coercion.  This is done to better support non-BMP strings.
 *  This makes the test case tricky to write: instead of using e.g. codepoint
 *  0x10041 (ToUint16() coerces to 0x41) use 0x100000041 (ToUint16() and
 *  ToUint32() coerces to 0x41).
 */

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

    // ToUint16/ToUint32 coercion, whole numbers
    pv(String.fromCharCode(4294967361, 4294967362, 1035087118403));
    pv(String.fromCharCode(-4294967231, -4294967230, -64424509373));

    // ToUint16/ToUint32 coercion, positive values, round to zero (floor)
    pv(String.fromCharCode(65.0, 65.1, 65.5, 65.9));

    // ToUint16/ToUint32 coercion, negative values, round to zero (ceil)
    // -65536 + 65 = -65471
    // -4294967296 + 65 = -4294967231
    pv(String.fromCharCode(-4294967231.0, -4294967231.1, -4294967231.5, -4294967231.9));

    // Note that coercion round to zero means that even if even if:
    // ToUint16(65.9) = 65, ToUint16(65.9 - 65536) = ToUint16(-65470.1) = 66!
    // ToUint32(65.9) = 65, ToUint32(65.9 - 4294967296) = ToUint32(-4294967230.1) = 66!
    pv(String.fromCharCode(65.9, 65.9 - 4294967296));

    // ToUint16/ToUint32 coercion, side effect order
    pv(String.fromCharCode(mkObj('1st', '65', 66),
                           mkObj('2nd', '67', 68),
                           mkObj('3rd', '69', 70)));

    // non-BMP (and modulo 2**32)
    pv2(String.fromCharCode(0, 127, 255, 0x345, 0x4567, 0x100006789, 0x10000ffff));

    // undefined arguments are coerced with ToUint16() (ToUin32() now),
    // yielding zero codepoints; they are different from arguments not
    // given at all

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
