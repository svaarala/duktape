/*===
inner 1 2
===*/

function foo(x) {
    return function(y) {
        print('inner', x, y);
    }
}

foo(1)(2);
