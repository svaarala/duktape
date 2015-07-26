/*
 *  Multiple case clauses map to same action.
 */

/*===
-2 out-of-range even-or-out-of-range
-1 out-of-range even-or-out-of-range
0 even even-or-out-of-range
1 odd odd
2 even even-or-out-of-range
3 odd odd
4 even even-or-out-of-range
5 odd odd
6 even even-or-out-of-range
7 odd odd
8 even even-or-out-of-range
9 odd odd
10 out-of-range even-or-out-of-range
11 out-of-range even-or-out-of-range
12 out-of-range even-or-out-of-range
===*/

function classify1(v) {
    switch (v) {
    case 0:
    case 2:
    case 4:
    case 6:
    case 8:
        return 'even';
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
        return 'odd';
    default:
        return 'out-of-range';
    }
}

function classify2(v) {
    switch (v) {
    default:
    case 0:
    case 2:
    case 4:
    case 6:
    case 8:
        return 'even-or-out-of-range';
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
        return 'odd';
    }
}

function test() {
    var i;

    for (i = -2; i <= 12; i++) {
        print(i, classify1(i), classify2(i));
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
