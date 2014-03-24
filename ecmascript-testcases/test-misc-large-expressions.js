/*
 *  Large expressions consume a lot of temporary registers and may lead to
 *  register shuffling which is exercised by this test case.
 *
 *  If a large expression contains function calls deep inside the expression
 *  structure, register shuffling alone is not enough to handle the call
 *  correctly: an ordinary DUK_OP_CALL cannot reach the registers, so an
 *  indirect call is needed instead.
 *
 *  To exercise all the indirect call related opcodes:
 *
 *    - DUK_OP_CALL: large expressions are enough
 *    - DUK_OP_NEW: use constructor call
 *    - DUK_OP_CSVAR: call through a non-local identifier (not a local variable)
 *    - DUK_OP_CSREG: call through a local variable
 *    - DUK_OP_CSPROP: call through a property
 *
 *  Constructor calls (DUK_OP_NEW) are independent of call setups, so it
 *  suffices to test DUK_OP_NEW with any of the call setup variants.
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

function MyConstructor() {
}

function buildExpr(count, firstFuncCall, lastFuncCall, allFuncCalls, callStyle) {
    var terms = [];
    var i;

    for (i = 0; i < count; i++) {
        if (allFuncCalls || (i == 0) && firstFuncCall || (i == count - 1) && lastFuncCall) {
            if (callStyle === 'newextvar') {
                terms.push('new MyConstructor("dummy")');
            } else if (callStyle === 'extvar') {
                terms.push('returnTrue("dummy")');
            } else if (callStyle === 'locvar') {
                terms.push('returnTrueLocal("dummy")');
            } else {
                terms.push('obj.ret("dummy")');
            }
        } else {
            terms.push('true');
        }
    }

    return "(function() {\n" +
           "    var returnTrueLocal = returnTrue;\n" +
           "    var obj = { ret: returnTrue };\n" +
           "    return " + terms.join(' && ') + ";\n" +
           "})();";
}

function largeExprTest() {
    var i, limit;

    function test(expr, name, expectTrue) {
        var res;
        try {
            res = eval(expr);
            if (expectTrue && res !== true) {
                throw new Error('result does not match expected value');
            } else if (!expectTrue && (typeof res !== 'object')) {
                // Constructor calls return an object ("truthy")
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
        test(buildExpr(i, false, false, false, 'ignore'), 'no-func-calls:' + i, true);
    }

    print('first func call');
    for (i = 2; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }
        test(buildExpr(i, true, false, false, 'newextvar'), 'first-func-call-newextvar:' + i, true);
        test(buildExpr(i, true, false, false, 'extvar'), 'first-func-call-extvar:' + i, true);
        test(buildExpr(i, true, false, false, 'locvar'), 'first-func-call-locvar:' + i, true);
        test(buildExpr(i, true, false, false, 'object'), 'first-func-call-object:' + i, true);
    }

    print('last func call');
    for (i = 2; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }
        test(buildExpr(i, false, true, false, 'newextvar'), 'last-func-call-newextvar:' + i, false);
        test(buildExpr(i, false, true, false, 'extvar'), 'last-func-call-extvar:' + i, true);
        test(buildExpr(i, false, true, false, 'locvar'), 'last-func-call-locvar:' + i, true);
        test(buildExpr(i, false, true, false, 'object'), 'last-func-call-object:' + i, true);
    }

    print('all func calls');
    for (i = 2; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }
        test(buildExpr(i, false, false, true, 'newextvar'), 'all-func-calls-newextvar:' + i, false);
        test(buildExpr(i, false, false, true, 'extvar'), 'all-func-calls-extvar:' + i, true);
        test(buildExpr(i, false, false, true, 'locvar'), 'all-func-calls-locvar:' + i, true);
        test(buildExpr(i, false, false, true, 'object'), 'all-func-calls-object:' + i, true);
    }
}

print('large expressions')

if (false) {
    print(buildExpr(3, false, false, false, null));
    print(buildExpr(3, true, false, false, 'newextvar'));
    print(buildExpr(3, true, false, false, 'extvar'));
    print(buildExpr(3, true, false, false, 'locvar'));
    print(buildExpr(3, true, false, false, 'object'));
    print(buildExpr(3, false, true, false, 'newextvar'));
    print(buildExpr(3, false, true, false, 'extvar'));
    print(buildExpr(3, false, true, false, 'locvar'));
    print(buildExpr(3, false, true, false, 'object'));
    print(buildExpr(3, false, false, true, 'newextvar'));
    print(buildExpr(3, false, false, true, 'extvar'));
    print(buildExpr(3, false, false, true, 'locvar'));
    print(buildExpr(3, false, false, true, 'object'));
}

try {
    largeExprTest();
} catch (e) {
    print(e.stack || e);
}
