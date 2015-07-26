/*===
0 foo
1 bar
0 foo
1 bar
0 foo
1 bar
===*/

/* g() used to be broken: local_iter would not get assigned to as a
 * left-hand side if it was a register-bound variable.
 */

var arr = ['foo', 'bar'];

function f() {
    for (global_iter in arr) {
        print(global_iter, arr[global_iter]);
    }
}

function g() {
    var local_iter;

    for (local_iter in arr) {
        print(local_iter, arr[local_iter]);
    }
}

function h() {
    for (var local_iter in arr) {
        print(local_iter, arr[local_iter]);
    }
}


f();
g();
h();
