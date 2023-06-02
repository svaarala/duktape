/*
 *  ES2021 decimal separator
 */

/*===
1000000
4294967295
2882400255
128
42798
1000000
1000000000000
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
1000000
===*/

function test() {
    function f(x) {
        try {
            print(eval(x));
        } catch (e) {
            print(e.name);
        }
    }

    f('1_000_000');
    f('0xffff_ffff');
    f('0xABCD_EFFF');
    f('0b1000_0000');
    f('0o123_456');
    f('1_000_000.000_000');
    f('1e1_2');
    f('1__000');
    f('1000__');
    f('1.000__');
    f('1.__000');
    f('1._000');
    f('1_e6');
    f('1e_6');
    f('0x_ffff');
    f('0xff__ff');
    f('0b1_2');

    // Also allowed in strict mode.
    f('(function () { "use strict"; return 1_000_000; })()');
}

test();
