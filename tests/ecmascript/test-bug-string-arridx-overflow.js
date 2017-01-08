/*
 *  Bug testcase for duk_hstring arridx overflow handling.
 */

/*===
0
===*/

function test() {
    var arr = [];
    arr['7394299990'] = 1;  // not in 32-bit range, should not affect arr.length

    print(arr.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
