/*===
check
body 0
check
body 1
check
body 2
check
body 3
check
body 4
check
body 5
check
body 6
check
body 7
check
body 8
check
body 9
check
10
check
body 0
check
body 1
check
body 2
check
body 3
check
body 4
check
body 5
check
body 6
check
body 7
check
body 8
check
body 9
check
10
body 0
body 1
body 2
body 3
body 4
body 0
body 1
body 2
body 3
body 4
===*/

/* For-loop continue must recheck condition on every loop.  There was a bug
 * in this behavior.
 */

function forContinueTest1() {
    for (var i = 0; print('check'), i < 10;) {
        print('body', i);
        i++;
        continue;
        i += 100;
    }
    print(i);
}

function forContinueTest2() {
    var i;
    for (i = 0; print('check'), i < 10;) {
        print('body', i);
        i++;
        continue;
        i += 100;
    }
    print(i);
}

function forContinueTest3() {
    for (var i in [1,2,3,4,5]) {
        print('body', i);
        continue;
    }
}

function forContinueTest4() {
    var i;
    for (i in [1,2,3,4,5]) {
        print('body', i);
        continue;
    }
}


try {
    forContinueTest1();
    forContinueTest2();
    forContinueTest3();
    forContinueTest4();
} catch (e) {
    print(e);
}

