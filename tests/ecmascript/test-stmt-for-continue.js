/*
 *  For-loop + continue had a few control flow bugs in the past.  The compiler
 *  handles an empty C part (for (A;B;C)) differently from a non-empty one, and
 *  there was a bug in the empty case with continue statements.  This testcase
 *  exercises for+continue control paths.
 */

/*===
check
body 0
update
check
body 1
update
check
body 2
update
check
body 3
update
check
body 4
update
check
body 5
update
check
body 6
update
check
body 7
update
check
body 8
update
check
body 9
update
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
check
body 0
update
check
body 1
update
check
body 2
update
check
body 3
update
check
body 4
update
check
body 5
update
check
body 6
update
check
body 7
update
check
body 8
update
check
body 9
update
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

function forContinueTest1a() {
    for (var i = 0; print('check'), i < 10; print('update')) {
        print('body', i);
        i++;
        continue;
        i += 100;
    }
    print(i);
}

function forContinueTest1b() {
    for (var i = 0; print('check'), i < 10;) {
        print('body', i);
        i++;
        continue;
        i += 100;
    }
    print(i);
}


function forContinueTest2a() {
    var i;
    for (i = 0; print('check'), i < 10; print('update')) {
        print('body', i);
        i++;
        continue;
        i += 100;
    }
    print(i);
}

function forContinueTest2b() {
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
    forContinueTest1a();
    forContinueTest1b();
    forContinueTest2a();
    forContinueTest2b();
    forContinueTest3();
    forContinueTest4();
} catch (e) {
    print(e);
}
