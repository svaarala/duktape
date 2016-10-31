/*
 *  String.fromCodePoint()
 *
 */

/*===
string 0
string 1 65
string 2 65 66
string 3 65 66 67
string 8 0 65535 55296 56320 56264 56883 56319 57343
RangeError
RangeError
RangeError
RangeError
RangeError
string 1 0
RangeError
RangeError
RangeError
RangeError
string 100 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74 65 66 67 68 69 70 71 72 73 74
valueOf() for foo
valueOf() for bar
valueOf() for quux
string 3 10123 10321 10234
===*/

function fromCodePointTest() {
    function pv(x) {
        var tmp = [];
        tmp.push(typeof x);
        tmp.push(x.length);
        for (var i = 0; i < x.length; i++) {
            tmp.push(x.charCodeAt(i));
        }
        print(tmp.join(' '));
    }
    function test(inp) {
        var res;
        try {
            res = String.fromCodePoint.apply(String, inp);
            pv(res);
        } catch (e) {
            print(e.name);
            //print(e.stack);
            return;
        }
    }

    function mkObj(name, strval, numval) {
        return {
            toString: function() { print('toString() for ' + name); return strval; },
            valueOf: function() { print('valueOf() for ' + name); return numval; }
        };
    }

    // basic tests
    test([]);
    test([ 65 ]);
    test([ 65, 66 ]);
    test([ 65, 66, 67 ]);

    // surrogate pair encoding
    test([ 0, 0xffff, 0x10000, 0x102233, 0x10ffff ]);

    // basic out-of-bounds cases
    test([ -1 ]);
    test([ 0x110000 ]);

    // fractions not accepted
    test([ 0.1 ]);
    test([ -0.1 ]);
    test([ 0x10ffff + 0.1 ]);

    // negative zero IS accepted
    test([ -0 ]);

    // infinity and NaN are not (doesn't match SameValue check)
    test([ 1 / 0 ]);
    test([ -1 / 0 ]);
    test([ 0 / 0 ]);

    // While String.fromCharCode() uses a simple ToUint16() (ToUint32() by
    // default in Duktape) which allows modulo values, String.fromCodePoint()
    // uses a SameValue(v, ToInteger(v)) check which is much stricter.
    test([ 0x41, 0x10041, 0x100000041 ]);

    // a lot of arguments; this is now limited by Duktape stack so we can't
    // put a REALLY long string here (separate bug test case for longer
    // strings now)
    test([
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74
    ]);

    // coercion side effects
    test([ mkObj('foo', '123', 10123), mkObj('bar', '321', 10321), mkObj('quux', '234', 10234) ]);
}

try {
    fromCodePointTest();
} catch (e) {
    print(e);
}
