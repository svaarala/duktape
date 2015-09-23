/*
 *  Exercise various code paths related to break/continue handling.
 */

/*===
testBreak1
0
testContinue1
0
1
2
testMultiLevelBreak1
0 0
testMultiLevelContinue1
0 0
1 0
2 0
testBreakThroughFinally1
start of outer 0
0 0
finally
end of function
testContinueThroughFinally1
start of outer 0
0 0
finally
start of outer 1
1 0
finally
start of outer 2
2 0
finally
end of function
testBreakCapturesFinally1
start of outer 0
0 0
finally
Error: dummy
end of function
testContinueCapturesFinally1
start of outer 0
0 0
finally
Error: dummy
end of function
testBreakThroughMultipleFinally1
start of outer 0
0 0
finally 1
finally 2
end of function
testContinueThroughMultipleFinally1
start of outer 0
0 0
finally 1
finally 2
start of outer 1
1 0
finally 1
finally 2
start of outer 2
2 0
finally 1
finally 2
end of function
testBreakConvertedToReturn1
start of outer 0
0 0
finally
123
testContinueConvertedToReturn1
start of outer 0
0 0
finally
123
===*/

/* Basic single level break/continue.  Current compiler turns these into
 * JUMP opcodes.
 */

function testBreak1() {
    for (var i = 0; i < 3; i++) {
        print(i);
        break;
    }
}
function testContinue1() {
    for (var i = 0; i < 3; i++) {
        print(i);
        continue;
    }
}

/* Basic multilevel break/continue.  These are compiled into an actual BREAK
 * or CONTINUE opcode now.
 */

function testMultiLevelBreak1() {
 label:
    for (var i = 0; i < 3; i++) {
        for (var j = 0; j < 3; j++) {
            print(i, j);
            break label;
        }
    }
}

function testMultiLevelContinue1() {
 label:
    for (var i = 0; i < 3; i++) {
        for (var j = 0; j < 3; j++) {
            print(i, j);
            continue label;
        }
    }
}

/* Break through a finally, finally doesn't change completion. */

function testBreakThroughFinally1() {
 label:
    for (var i = 0; i < 3; i++) {
        print('start of outer', i);

        try {
            for (var j = 0; j < 3; j++) {
                print(i, j);
                break label;
            }
        } finally {
            print('finally');
        }

        print('end of outer', i);
    }

    print('end of function');
}

/* Continue through a finally, finally doesn't change completion. */

function testContinueThroughFinally1() {
 label:
    for (var i = 0; i < 3; i++) {
        print('start of outer', i);

        try {
            for (var j = 0; j < 3; j++) {
                print(i, j);
                continue label;
            }
        } finally {
            print('finally');
        }

        print('end of outer', i);
    }

    print('end of function');
}

/* Break captured by finally. */

function testBreakCapturesFinally1() {
    try {
     label:
        for (var i = 0; i < 3; i++) {
            print('start of outer', i);

            try {
                for (var j = 0; j < 3; j++) {
                    print(i, j);
                    break label;
                }
            } finally {
                print('finally');
                throw new Error('dummy');
            }

            print('end of outer', i);
        }
    } catch (e) {
        print(e);
    }

    print('end of function');
}

/* Continue captured by finally. */

function testContinueCapturesFinally1() {
    try {
     label:
        for (var i = 0; i < 3; i++) {
            print('start of outer', i);

            try {
                for (var j = 0; j < 3; j++) {
                    print(i, j);
                    continue label;
                }
            } finally {
                print('finally');
                throw new Error('dummy');
            }

            print('end of outer', i);
        }
    } catch (e) {
        print(e);
    }

    print('end of function');
}

/* Break through multiple finally parts, finally doesn't change completion. */

function testBreakThroughMultipleFinally1() {
 label:
    for (var i = 0; i < 3; i++) {
        print('start of outer', i);

        try {
            try {
                for (var j = 0; j < 3; j++) {
                    print(i, j);
                    break label;
                }
            } finally {
                print('finally 1');
            }
        } finally {
            print('finally 2');
        }

        print('end of outer', i);
    }

    print('end of function');
}

/* Continue through multiple finally parts, finally doesn't change completion. */

function testContinueThroughMultipleFinally1() {
 label:
    for (var i = 0; i < 3; i++) {
        print('start of outer', i);

        try {
            try {
                for (var j = 0; j < 3; j++) {
                    print(i, j);
                    continue label;
                }
            } finally {
                print('finally 1');
            }
        } finally {
            print('finally 2');
        }

        print('end of outer', i);
    }

    print('end of function');
}

/* Break captured by a finally and converted into a return. */

function testBreakConvertedToReturn1() {
 label:
    for (var i = 0; i < 3; i++) {
        print('start of outer', i);

        try {
            for (var j = 0; j < 3; j++) {
                print(i, j);
                break label;
            }
        } finally {
            print('finally');
            return 123;
        }

        print('end of outer', i);
    }

    print('end of function');
}

/* Continue captured by a finally and converted into a return. */

function testContinueConvertedToReturn1() {
 label:
    for (var i = 0; i < 3; i++) {
        print('start of outer', i);

        try {
            for (var j = 0; j < 3; j++) {
                print(i, j);
                continue label;
            }
        } finally {
            print('finally');
            return 123;
        }

        print('end of outer', i);
    }

    print('end of function');
}

try {
    print('testBreak1');
    testBreak1();

    print('testContinue1');
    testContinue1();

    print('testMultiLevelBreak1');
    testMultiLevelBreak1();

    print('testMultiLevelContinue1');
    testMultiLevelContinue1();

    print('testBreakThroughFinally1');
    testBreakThroughFinally1();

    print('testContinueThroughFinally1');
    testContinueThroughFinally1();

    print('testBreakCapturesFinally1');
    testBreakCapturesFinally1();

    print('testContinueCapturesFinally1');
    testContinueCapturesFinally1();

    print('testBreakThroughMultipleFinally1');
    testBreakThroughMultipleFinally1();

    print('testContinueThroughMultipleFinally1');
    testContinueThroughMultipleFinally1();

    print('testBreakConvertedToReturn1');
    print(testBreakConvertedToReturn1());

    print('testContinueConvertedToReturn1');
    print(testContinueConvertedToReturn1());
} catch (e) {
    print(e);
}
