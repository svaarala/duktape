/*
 *  While statement (E5 Section 12.10).
 */

/*FIXME*/

/*===
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

try {
    whileTest();
} catch (e) {
    print(e);
}
