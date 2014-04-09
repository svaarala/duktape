/*
 *  Do-while statement (E5 Section 12.6.1).
 */

/*===
basic
body, i: 10 count: 0
while
body, i: 9 count: 1
while
body, i: 8 count: 2
while
body, i: 7 count: 3
while
body, i: 6 count: 4
while
body, i: 5 count: 5
while
body, i: 4 count: 6
while
body, i: 3 count: 7
while
body, i: 2 count: 8
while
body, i: 1 count: 9
while
body, i: 0 count: 10
while
final i: -1
===*/

/* Basic loop test with printing and side effect in control flow predicate. */

function basicTest() {
    var count = 0;
    var i = 10;

    do {
        print('body, i:', i, 'count:', count);
    } while(print('while'), count++, i-- > 0);

    print('final i:', i);
}

print('basic');
try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
break
body, i: 10 count: 0
before while
while
body, i: 9 count: 1
before while
while
body, i: 8 count: 2
before while
while
body, i: 7 count: 3
before while
while
body, i: 6 count: 4
before while
while
body, i: 5 count: 5
before while
while
body, i: 4 count: 6
break when i=4
final i: 4
===*/

function breakTest() {
    var count = 0;
    var i = 10;

    do {
        print('body, i:', i, 'count:', count);
        if (i == 4) {
            print('break when i=4');
            break;
        }
        print('before while');
    } while(print('while'), count++, i-- > 0);

    print('final i:', i);
}

print('break');

try {
    breakTest();
} catch (e) {
    print(e);
}

/*===
continue
body, i: 10 count: 0
before while
while
body, i: 9 count: 1
before while
while
body, i: 8 count: 2
before while
while
body, i: 7 count: 3
before while
while
body, i: 6 count: 4
before while
while
body, i: 5 count: 5
before while
while
body, i: 4 count: 6
continue when i=4
while
body, i: 3 count: 7
before while
while
body, i: 2 count: 8
before while
while
body, i: 1 count: 9
before while
while
body, i: 0 count: 10
before while
while
final i: -1
===*/

function continueTest() {
    var count = 0;
    var i = 10;

    do {
        print('body, i:', i, 'count:', count);
        if (i == 4) {
            print('continue when i=4');
            continue;
        }
        print('before while');
    } while(print('while'), count++, i-- > 0);

    print('final i:', i);
}

print('continue');

try {
    continueTest();
} catch (e) {
    print(e);
}
