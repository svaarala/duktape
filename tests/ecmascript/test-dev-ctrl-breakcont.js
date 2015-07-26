/*
 *  Some test cases for label matching cases.
 *
 *  Especially, test 'continue' through a switch.
 */

/*===
before break
after do
before continue
after do
===*/

/* fast break (handled with a JUMP) */

try {
    do {
        print("before break");
        break;
        print("after break");
    } while(0);
    print("after do");
} catch (e) {
    print(e.name);
}

try {
    done = 0;
    do {
        if (done) {
            break;
        }
        print("before continue");
        done = 1;
        continue;
        print("after continue");
    } while(0);
    print("after do");
} catch (e) {
    print(e.name);
}

/*===
break caught by finally
broke out
continue caught by finally
broke out
===*/

/* slow break across a try-catch boundary, handled with BREAK */

try {
    do {
        try {
            break;
        } finally {
            print("break caught by finally");  // caught, but rethrown
        }
    } while(true);
    print("broke out");
} catch (e) {
    print(e.name);
}

/* slow continue across a try-catch boundary, handled with CONTINUE */

try {
    done = 0;
    do {
        if (done) {
            break;
        }
        try {
            done = 1;
            continue;
        } finally {
            print("continue caught by finally");  // caught, but rethrown
        }
    } while(true);
    print("broke out");
} catch (e) {
    print(e.name);
}

/*===
===*/

/* FIXME: same tests for 'with' */

/*===
3
2
1
loop done
===*/

/* A continue with an empty label from inside a switch should bypass the
 * switch statement (which registers no empty label for 'continue') and
 * may match an empty label outside the switch.
 */

try {
    count = 3;
    do {
        print(count);
        count -= 1;
        if (count == 0) {
            break;
        }

        switch(1) {
        case 1:
            // this continue should go to top of the do-loop
            continue;
        }

        print("never here");
    } while(true);
    print("loop done");
} catch (e) {
    print(e.name);
}
