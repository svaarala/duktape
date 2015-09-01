/*
 *  Switch statement (E5 Section 12.11).
 */

/*===
simple case with fallthrough
123
234
empty
default only
default
matching case, only one case
123
no matching case, only one case
match default and fallthrough
default
123
234
strict equality for case comparison: -0 equals +0
negative zero
===*/

function test() {
    var x;

    print('simple case with fallthrough');
    x = 123;
    switch (x) {
    case 321: print(321); break
    case 123: print(123); /* no break */
    case 234: print(234); break;
    case 345: print(345); break;
    default: print('default'); break;
    }

    print('empty');
    x = 123;
    switch (x) {
    }

    print('default only');
    x = 123;
    switch (x) {
    default: print('default');
    }

    print('matching case, only one case');
    x = 123;
    switch (x) {
    case 123: print(123);
    }

    print('no matching case, only one case');
    x = 123;
    switch (x) {
    case 321: print(321);
    }

    print('match default and fallthrough');
    x = 999;
    switch (x) {
    case 321: print(321); break
    default: print('default'); /* no break */
    case 123: print(123); /* no break */
    case 234: print(234); break;
    case 345: print(345); break;
    }

    print('strict equality for case comparison: -0 equals +0');
    x = +0;
    switch (x) {
    case 123: print(123); break;
    case -0: print('negative zero'); break;
    case 0: print('positive zero'); break;
    default: print('default'); break;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
