/*
 *  Array for-in enumeration order is not strictly specified in ES5.
 *  That is also the case for ES2015 and ES2016.  However, Duktape 2.x
 *  follows ES2015 [[OwnPropertyKeys]] order also in for-in and Object.keys()
 *  so test for the custom behavior.
 */

/*---
{
    "custom": true
}
---*/

function enumArray(arr) {
    print('array keys');
    for (var i in arr) {
        print(i);
    }
}

/*===
array keys
0
1
2
foo
array keys
0
1
2
3
4
foo
array keys
0
1
2
3
4
10000
foo
array keys
0
1
2
3
4
10
10000
foo
===*/

var a;

// initially dense
a = [1,2,3];
a.foo = 'bar';
enumArray(a);

// new keys, array order maintained (NOT insertion order)
a[4] = 5;
a[3] = 4;
enumArray(a);

// force to sparse
a[10000] = 9999;
enumArray(a);

// array order no longer maintained; now insertion order
a[10] = 9;
enumArray(a);
