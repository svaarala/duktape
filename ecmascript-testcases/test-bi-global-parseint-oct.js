/*
 *  parseInt() behavior for octal (non-standard)
 */

/*---
{
    "nonstandard": true
}
---*/

// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/*===
radix 8
123
129
123
129
-123
-129
83
10
83
10
-83
-10
123
129
123
129
-123
-129
===*/

/* Radix 8 test.
 *
 * These is no standard automatic mechanism for using radix 8 (like
 * "0x" or "0X" prefix for radix 16).  However, both V8 and Rhino seem
 * to use a leading zero to indicate automatic radix 8.
 *
 * This is not standard behavior, so test against this behavior
 * (at least for now).
 */

/* XXX: change Duktape behavior to match V8 and Rhino for octal? */

print('radix 8');

function radix8Test() {
    // this should be interpreted as base-10; V8 and Rhino interpret these
    // as octal (0129 will be parsed as "012" with 9 treated as garbage)

    print(g.parseInt('0123'));
    print(g.parseInt('0129'));
    print(g.parseInt('+0123'));
    print(g.parseInt('+0129'));
    print(g.parseInt('-0123'));
    print(g.parseInt('-0129'));

    // explicit radix 8
    print(g.parseInt('0123', 8));
    print(g.parseInt('0129', 8));
    print(g.parseInt('+0123', 8));
    print(g.parseInt('+0129', 8));
    print(g.parseInt('-0123', 8));
    print(g.parseInt('-0129', 8));

    // explicit radix 10
    print(g.parseInt('0123', 10));
    print(g.parseInt('0129', 10));
    print(g.parseInt('+0123', 10));
    print(g.parseInt('+0129', 10));
    print(g.parseInt('-0123', 10));
    print(g.parseInt('-0129', 10));
}

try {
    radix8Test();
} catch (e) {
    print(e.name);
}
