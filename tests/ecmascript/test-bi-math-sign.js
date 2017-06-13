/*
 *  Math.sign()
 */

/*@include util-number.js@*/

/*===
function true false true
1
-1
-1
-0
0
1
1
NaN
0
NaN
valueOf 1
1
valueOf 2
toString 2
-1
valueOf 3
toString 3
NaN
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'sign');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.sign.length);

    [ -1/0, -123.1, -0, +0, 123.1, 1/0,
      0/0, '', 'x',
      { valueOf: function () { print('valueOf 1'); return 123 },
        toString: function () { print('toString 1'); return -321; } },
      { valueOf: function () { print('valueOf 2'); return {} },  // fails
        toString: function () { print('toString 2'); return -321; } },
      { valueOf: function () { print('valueOf 3'); return {} },  // fails
        toString: function () { print('toString 3'); return 'x'; } },  // -> NaN
    ].forEach(function (v, i) {
        x = Math.sign(v);
        print(toStringExact(x));
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
