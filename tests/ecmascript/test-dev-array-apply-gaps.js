/*
 *  From: http://www.2ality.com/2012/06/dense-arrays.html
 */

/*===
sparse array
,,
3
undefined
false
dense array trick
,,
3
undefined
true
0 undefined
1 undefined
2 undefined
0,1,2
apply trick
0,1,2
3
0
true
0 0
1 1
2 2
0,1,2
===*/

function test() {
    var arr;

    print('sparse array');
    arr = new Array(3);
    print(arr);
    print(arr.length);
    print(arr[0]);
    print('0' in arr);
    arr.forEach(function (v, i) { print(i, v); });

    print('dense array trick');
    arr = Array.apply(null, Array(3));
    print(arr);
    print(arr.length);
    print(arr[0]);
    print('0' in arr);
    arr.forEach(function (v, i) { print(i, v); });
    print(arr.map(function (v, i) { return i; }));

    print('apply trick');
    arr = Array.apply(null, Array(3)).map(Function.prototype.call.bind(Number));
    print(arr);
    print(arr.length);
    print(arr[0]);
    print('0' in arr);
    arr.forEach(function (v, i) { print(i, v); });
    print(arr.map(function (v, i) { return i; }));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
