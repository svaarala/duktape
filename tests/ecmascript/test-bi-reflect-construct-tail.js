/*
 *  In Duktape 2.2 Reflect.construct() is handled inline in call handling and
 *  doesn't involve a native call.  ECMAScript-to-ECMAScript Reflect.construct()
 *  calls are thus only limited by callstack limit (not recursion C limit).
 *
 *  Reflect.construct() can be used in tailcall position as long as the function
 *  making the call is also a constructor function, so that both functions share
 *  the same return value constructor postprocessing.
 */

/*---
{
    "custom": true
}
---*/

/*===
count: 5001
RangeError
count: 1000001
===*/

var count;

function MyConstructor(n) {
    count++;
    if (n <= 0) {
        return;
    }
    var tmp = Reflect.construct(MyConstructor, [ n - 1 ]);
    return tmp;
}

function MyConstructorTailcall(n) {
    count++;
    if (n <= 0) {
        return;
    }
    return Reflect.construct(MyConstructorTailcall, [ n - 1 ]);
}

function test() {
    // Ecma-to-Ecma Reflect.construct() call: limited by call stack.
    count = 0;
    new MyConstructor(5000);
    print('count:', count);

    // Here the count is too large and call stack limit is reached.
    count = 0;
    try {
        new MyConstructor(1e6);
        print('count:', count);
    } catch (e) {
        print(e.name);
    }

    // No limit for tailcall variant.
    count = 0;
    new MyConstructorTailcall(1e6);
    print('count:', count);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
