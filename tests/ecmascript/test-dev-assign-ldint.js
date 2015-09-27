/*
 *  "x = 1234" compiles to an LDINT if RHS is LDINT compatible.
 *  Check boundary conditions.
 */

/*===
-131073
-131072
-131071
131070
131071
131072
0 -Infinity
0 Infinity
0.5
-131073
-131072
-131071
131070
131071
131072
0 -Infinity
0 Infinity
0.5
-131073
-131072
-131071
131070
131071
131072
0 -Infinity
0 Infinity
0.5
===*/

var glob;

function test() {
    var x;
    var obj = {};

    /* DUK_BC_LDINT_BIAS is 1 << 17 = 131072, so that LDINT range is
     * [-131072, 131071].
     */

    x = -131073; print(x);
    x = -131072; print(x);
    x = -131071; print(x);
    x = 131070; print(x);
    x = 131071; print(x);
    x = 131072; print(x);
    x = -0; print(x, 1/x);
    x = +0; print(x, 1/x);
    x = 0.5; print(x);

    obj.foo = -131073; print(obj.foo);
    obj.foo = -131072; print(obj.foo);
    obj.foo = -131071; print(obj.foo);
    obj.foo = 131070; print(obj.foo);
    obj.foo = 131071; print(obj.foo);
    obj.foo = 131072; print(obj.foo);
    obj.foo = -0; print(obj.foo, 1/obj.foo);
    obj.foo = +0; print(obj.foo, 1/obj.foo);
    obj.foo = 0.5; print(obj.foo);

    glob = -131073; print(glob);
    glob = -131072; print(glob);
    glob = -131071; print(glob);
    glob = 131070; print(glob);
    glob = 131071; print(glob);
    glob = 131072; print(glob);
    glob = -0; print(glob, 1/glob);
    glob = +0; print(glob, 1/glob);
    glob = 0.5; print(glob);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
