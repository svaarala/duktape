/*===
inner 1 2
===*/

function foo(x) {
    return function(y) {
        print('inner', x, y);
    }
}

try {
    foo(1)(2);
} catch (e) {
    print(e.name);
}
