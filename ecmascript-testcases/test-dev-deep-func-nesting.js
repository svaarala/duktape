/*
 *  Currently the compiler requires O(2^N) passes to compile inner
 *  functions (where N is function nesting level).  This needs to
 *  be fixed, but for now the compiler token limit should protect
 *  against seemingly infinite blocking in this case.
 */

/*---
{
    "custom": true,
    "slow": true
}
---*/

/*===
RangeError
===*/

function deepInnerFunctionTest() {
    var txt = '';
    var i;

    for (i = 0; i < 30; i++) {
        txt = 'function func' + i + '() { ' + txt + ' }';
    }
    eval(txt);
}

try {
    deepInnerFunctionTest();
    print('returned');
} catch (e) {
    print(e.name);
}

