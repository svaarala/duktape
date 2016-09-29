/*
 *  Common idiom of using String as a callback.
 */

/*===
["1","2","3","true","false","[object Object]","","foo,bar"]
===*/

function test() {
    print(JSON.stringify([ 1, 2, 3, true, false, {}, [], [ 'foo', 'bar' ] ].map(String)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
