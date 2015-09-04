/*
 *  Expression statement (E5 Section 12.4).
 */

/*===
valueOf -> 123
valueOf -> 234
eval -> 801
===*/

function test() {
    var obj;

    // Basic expression statements
    123;
    1 + 2 + 3;

    // Expression statement with side effects, side effects must
    // be invoked.
    +({ valueOf: function () { print('valueOf -> 123'); return 123; } });
    obj = { valueOf: function () { print('valueOf -> 234'); return 234; } };
    +obj;

    // In eval code an expression statement yields a value
    print('eval -> ' + eval("345 + 456;"));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
