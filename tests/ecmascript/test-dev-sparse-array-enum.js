/*
 *  Array enumeration order is not strictly specified.  This test case
 *  compares behavior against the currently desired semantics:
 *
 *    - If dense, enumerate array keys first, then other keys
 *
 *    - When converting to sparse, re-add keys so that array keys
 *      are first (i.e. preserve order when abandoning array part)
 *
 *    - If sparse, maintain key insertion order only
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
foo
10000
array keys
0
1
2
3
4
foo
10000
10
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
