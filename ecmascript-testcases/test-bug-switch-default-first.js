/*
 *  Reported by Andreas Oman (andoma)
 *
 *  https://github.com/svaarala/duktape/issues/155
 */

/*===
10
11
10
10
11
11
10
11
10
===*/

// In Duktape 1.1.x this incorrectly returns 10 when v == 1.
function test1(v) {
    switch(v) {
    default:
    case 0:
        return 10;
    case 1:
        return 11;
    }
}

// Swapping case 0 and default would fix it; returns 11.
function test2(v) {
    switch(v) {
    case 0:
        return 10;
    default:
    case 1:
        return 11;
    }
}

// Adding a dummy case also fixed it; returns 11.
function test3(v) {
    switch(v) {
    case 99:
    default:
    case 0:
        return 10;
    case 1:
        return 11;
    }
}

try {
    print(test1(0));
    print(test1(1));
    print(test1(2));

    print(test2(0));
    print(test2(1));
    print(test2(2));

    print(test3(0));
    print(test3(1));
    print(test3(2));
} catch (e) {
    print(e.stack || e);
}
