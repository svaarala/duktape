/*
 *  Mutate a property access as part of an expression.  Conceptually the lookup
 *  must happen before proceeding with the expression.
 */

/*===
1123
1123
===*/

function test1() {
    var obj = { foo: 123 };
    var alt = { foo: 234 };

    print(obj.foo + (obj = alt, 1000));
}

function test2() {
    var obj = { foo: 123 };
    var alt = { foo: 234 };

    print(obj['foo'] + (obj = alt, 1000));
}

try {
    test1();
    test2();
} catch (e) {
    print(e.stack || e);
}
