/*
 *  Testcase for JSON parsing of numbers.
 *
 *  The number syntax for JSON differs a bit from Ecmascript syntax.
 */

/*===
json 1.23 1.23
eval 1.23 1.23
json -1.23 -1.23
eval -1.23 -1.23
json +1.23 SyntaxError
eval +1.23 1.23
json 1.23e3 1230
eval 1.23e3 1230
json -1.23e3 -1230
eval -1.23e3 -1230
json +1.23e3 SyntaxError
eval +1.23e3 1230
json 1.23e+3 1230
eval 1.23e+3 1230
json -1.23e+3 -1230
eval -1.23e+3 -1230
json +1.23e+3 SyntaxError
eval +1.23e+3 1230
json 1.23e-3 0.00123
eval 1.23e-3 0.00123
json -1.23e-3 -0.00123
eval -1.23e-3 -0.00123
json +1.23e-3 SyntaxError
eval +1.23e-3 0.00123
json 1234567890e0 1234567890
eval 1234567890e0 1234567890
json 1234567890e+0 1234567890
eval 1234567890e+0 1234567890
json 1234567890e-0 1234567890
eval 1234567890e-0 1234567890
json -1234567890e0 -1234567890
eval -1234567890e0 -1234567890
json +1234567890e0 SyntaxError
eval +1234567890e0 1234567890
===*/

function test() {
    var vals = [
        '1.23',
        '-1.23',
        '+1.23',     // leading plus not allowed in JSON

        '1.23e3',
        '-1.23e3',
        '+1.23e3',   // invalid JSON

        '1.23e+3',   // leading plus is allowed for exponent
        '-1.23e+3',
        '+1.23e+3',  // invalid JSON

        '1.23e-3',
        '-1.23e-3',
        '+1.23e-3',  // invalid JSON

        // a few spot checks related to fast path
        '1234567890e0',
        '1234567890e+0',
        '1234567890e-0',
        '-1234567890e0',
        '+1234567890e0',
    ];

    vals.forEach(function (v) {
        try {
            print('json', v, JSON.parse(v));
        } catch (e) {
            print('json', v, e.name);
        }

        try {
            print('eval', v, eval(v));
        } catch (e) {
            print('eval', v, e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
