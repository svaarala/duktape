/*
 *  While statement (E5 Section 12.10).
 */

/*===
basic
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

function basicTest() {
    var count = 0;
    var i = 10;

    while (print('while'), count++, i-- > 0) {
        print('body, i:', i, 'count:', count);
    }
    print('final i:', i);
}

print('basic');

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
break and continue
while
body, i: 9 count: 1
end body
while
body, i: 8 count: 2
continue at 6 and 8
while
body, i: 7 count: 3
end body
while
body, i: 6 count: 4
continue at 6 and 8
while
body, i: 5 count: 5
end body
while
body, i: 4 count: 6
break at 4
final i: 4
===*/

function breakContinueTest() {
    var count = 0;
    var i = 10;

    while (print('while'), count++, i-- > 0) {
        print('body, i:', i, 'count:', count);
        if (i == 8 || i == 6) {
            print('continue at 6 and 8');
            continue;
        }
        if (i == 4) {
            print('break at 4');
            break;
        }
        print('end body');
    }
    print('final i:', i);
}


print('break and continue');

try {
    breakContinueTest();
} catch (e) {
    print(e);
}

/*===
misc
4
3
2
1
0
final -1
final -1
4
3
2
1
0
final -1
4
3
2
1
final 1
in while
4
in while
3
in while
2
in while
1
in while
0
in while
final -1
===*/

function whileTest() {
    var i;

    i = 5;
    while (i--) {
        print(i);
    }
    print('final', i);

    i = 0;
    while (i--) {
        print(i);
    }
    print('final', i);

    i = 5;
    while (i--) {
        print(i);
        continue;
    }
    print('final', i);

    i = 5;
    while (i--) {
        print(i);
        if (i >= 4) { continue; }
        if (i == 1) { break; }
    }
    print('final', i);

    i = 5;
    while (print('in while'), i--) {
        print(i);
    }
    print('final', i);
}

print('misc');

try {
    whileTest();
} catch (e) {
    print(e);
}
