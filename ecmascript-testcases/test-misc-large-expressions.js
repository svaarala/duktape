/*
 *  Large expressions consume a lot of temporary registers and may lead to
 *  register shuffling which is exercised by this test case.
 *
 *  If a large expression contains function calls deep inside the expression
 *  structure, register shuffling alone is not enough to handle the call
 *  correctly: an ordinary DUK_OP_CALL cannot reach the registers, so an
 *  indirect call is needed instead.
 */

/*===
large expressions
no func calls
100
200
300
400
500
600
700
800
900
1000
1100
1200
1300
1400
1500
1600
1700
1800
1900
first func call
100
200
300
400
500
600
700
800
900
1000
1100
1200
1300
1400
1500
1600
1700
1800
1900
last func call
100
200
300
400
500
600
700
800
900
1000
1100
1200
1300
1400
1500
1600
1700
1800
1900
all func calls
100
200
300
400
500
600
700
800
900
1000
1100
1200
1300
1400
1500
1600
1700
1800
1900
===*/

function returnTrue() {
    return true;
}

function buildExpr(count, firstFuncCall, lastFuncCall, allFuncCalls) {
    var terms = [];
    var i;

    for (i = 0; i < count; i++) {
        if (allFuncCalls || (i == 0) && firstFuncCall || (i == count - 1) && lastFuncCall) {
            terms.push('returnTrue("dummy")');
        } else {
            terms.push('true');
        }
    }

    return terms.join(' && ');
}

function largeExprTest() {
    var i, limit;

    function test(expr, name, expected) {
        var res;
        try {
            res = eval(expr);
            if (res !== expected) {
                throw new Error('result does not match expected value');
            }
        } catch (e) {
            print('failed ' + name + ': ' + e);
            throw e;
        }
    }

    // Compiler recursion limit currently bites around 2500
    limit = 2000;

    print('no func calls');
    for (i = 1; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }
        test(buildExpr(i, false, false, false), 'no-func-calls:' + i, true);
    }

    print('first func call');
    for (i = 1; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }
        test(buildExpr(i, true, false, false), 'first-func-call:' + i, true);
    }

    print('last func call');
    for (i = 1; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }
        test(buildExpr(i, false, true, false), 'last-func-call:' + i, true);
    }

    print('all func calls');
    for (i = 1; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }
        test(buildExpr(i, false, false, true), 'all-func-calls:' + i, true);
    }
}

print('large expressions')

//print(buildExpr(3, false, false, false));
//print(buildExpr(3, true, false, false));
//print(buildExpr(3, false, true, false));
//print(buildExpr(3, false, false, true));

try {
    largeExprTest();
} catch (e) {
    print(e.stack || e);
}
