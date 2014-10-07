/*===
[1,2,null,4]
[1,2,null,4,null,null,null,null]
[1,2,null,4,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,"foo"]
2 1 2 entry-in-proto undefined
[1,2]
===*/

/* E5.1 Section 15.12.3, algorithm JA(), step 6 indicates that array elements
 * must be enumerated in ascending array index order, and that the value of
 * the "length" property must be obeyed.  All missing elements are 'undefined'
 * and will serialize as "null".
 *
 * In particular, if an ancestor has an array index property above the Array's
 * "length", it must not be serialized.
 *
 * Also, sparse arrays (internal array part has been abandoned) must still
 * enumerate according to array index order.
 */

function arrayLengthTest1() {
    var arr;
    var k;

    // base case
    arr = [ 1, 2, undefined, 4 ];
    print(JSON.stringify(arr));

    // length must be obeyed even if no elements have been added
    arr = [ 1, 2, undefined, 4 ];
    arr.length = 8;
    print(JSON.stringify(arr));

    arr = [ 1, 2 ];
    arr[100] = 'foo';
    arr[3] = 4;
    print(JSON.stringify(arr));
}

// Note: this test changes Array.prototype so run this last
function arrayLengthTest2() {
    var arr;

    arr = [ 1, 2 ];
    Array.prototype[2] = 'entry-in-proto';

    print(arr.length, arr[0], arr[1], arr[2], arr[3]);

    print(JSON.stringify(arr));
}

try {
    arrayLengthTest1();
    arrayLengthTest2();
} catch (e) {
    print(e.name);
}
