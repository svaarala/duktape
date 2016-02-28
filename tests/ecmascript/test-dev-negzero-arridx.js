/*
 *  The number -0 is accepted as an "array index" and triggers exotic .length
 *  behavior because ToString(-0) == "0" which is in canonical form and thus a
 *  valid array index.
 *
 *  The string "-0" does *not* trigger the exotic behavior because it's not in
 *  canonical form.  Same goes for "+0".
 */

/*===
1 ["foo"]
1 ["foo"]
0 []
0 []
===*/

function test() {
    var arr;

    arr = [];
    arr[-0] = 'foo';
    print(arr.length, JSON.stringify(arr));

    arr = [];
    arr['0'] = 'foo';
    print(arr.length, JSON.stringify(arr));

    arr = [];
    arr['+0'] = 'foo';
    print(arr.length, JSON.stringify(arr));

    arr = [];
    arr['-0'] = 'foo';
    print(arr.length, JSON.stringify(arr));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
