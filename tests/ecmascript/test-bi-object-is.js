/*
 *  Object.is()
 */

/*===
basic test
function true false true
2
true
false
false
true
true
0 0
1 1
2 2
3 3
4 4
5 5
6 6
7 7
8 8
9 9
10 10
11 11
12 12
===*/

function basicTest() {
    var pd = Object.getOwnPropertyDescriptor(Object, 'is');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Object.is.length);

    // SameValue speciality is to take zero sign into account and to compare
    // NaNs are true.
    print(Object.is(-0, -0));
    print(Object.is(-0, +0));
    print(Object.is(+0, -0));
    print(Object.is(+0, +0));
    print(Object.is(0/0, 0/0));

    var values = [
        undefined, null, true, false, -1/0, -0, 0, 1/0, 123,
        '', 'foo', {}, []
    ];
    values.forEach(function (v1, i1) {
        values.forEach(function (v2, i2) {
            var res = Object.is(i1, i2);
            if (res) {
                print(i1, i2);  // print true comparisons only
            }
        });
    });
}

try {
    print('basic test');
    basicTest();
} catch (e) {
    print(e.stack || e);
}
