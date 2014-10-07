/*
 *  Development time bug caused by too lenient string-to-array-index
 *  conversion.
 */

/*===
quux
quux
undefined
===*/

try {
    var arr = [ 'foo', 'bar', 'quux', 'baz' ];
    print(arr[2]);
    print(arr['2']);
    print(arr['02']);  // should be undefined, '02' is not an array index compatible string
} catch (e) {
    print(e);
}
