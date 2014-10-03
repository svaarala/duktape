/*===
y 2
x 1
===*/

/* This had a bug at some point. */

function f(x) {
    return function(y) {
        print('y', y);
        print('x', x);
    }
}

f(1)(2);
