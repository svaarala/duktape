/*===
2 foo,bar
5 foo,bar,,,
true
5 ,bar,,,
5 ,bar,,,
===*/

try {
    var arr = [ 'foo', 'bar' ];
    Object.preventExtensions(arr);
    print(arr.length, String(arr));
    arr.length = 5;  // allowed, .length exists already
    print(arr.length, String(arr));
    print(delete arr[0]);  // still allowed
    print(arr.length, String(arr));
    arr[0] = 'foo';  // rejected
    print(arr.length, String(arr));
} catch (e) {
    print(e.stack || e);
}
