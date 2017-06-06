/*
 *  In Duktape 2.2 Reflect.construct() is handled inline in call handling and
 *  is no longer visible in the callstack.
 */

/*---
{
    "custom": true
}
---*/

/*===
MyConstructor
-1 act
-2 MyConstructor
-3 test
-4 global
===*/

function MyConstructor(a, b, c) {
    var idx;

    print('MyConstructor');
    for (idx = -1; ; idx--) {
        var t = Duktape.act(idx);
        if (!t) { break; }
        print(idx, (t.function || {}).name);
    }
}

function test() {
    Reflect.construct(MyConstructor, [ 1, 2, 3 ]);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
