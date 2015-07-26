/*
 *  The compiler used to require O(2^N) passes to copmile inner functions
 *  (where N is function nesting level).  Deeply nested functions would
 *  hit the compiler "token limit" and cause a RangeError to prevent the
 *  compiler from taking an inordinate amount of time.
 *
 *  This has now been fixed (Duktape 0.10.0) so test that deep nesting
 *  works properly.
 */

/*---
{
    "custom": true,
    "slow": true
}
---*/

/*===
returned
===*/

function deepInnerFunctionTest() {
    var txt = '';
    var i;

    for (i = 0; i < 1000; i++) {
        txt = 'function func' + i + '() { ' + txt + ' }';
    }
    //print(txt);
    eval(txt);
}

try {
    deepInnerFunctionTest();
    print('returned');
} catch (e) {
    print(e.name);
}
