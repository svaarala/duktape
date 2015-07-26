/*
 *  Large expressions consume a lot of temporary registers and may lead to
 *  register shuffling which is exercised by this test case.
 *
 *  If a large expression contains function calls deep inside the expression
 *  structure, register shuffling alone is not enough to handle the call
 *  correctly: an ordinary DUK_OP_CALL cannot reach the registers, so an
 *  indirect call is needed instead.  Object literal related instructions
 *  (NEWOBJ, NEWARR, MPUTOBJ, MPUTARR) also need some indirection.
 *
 *  In addition to indirection, several instructions need shuffling which
 *  is exercised in this test case.
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
 *
 *  Other opcodes to test:
 *
 *    - NEWARR and MPUTARR: array literal
 *    - NEWOBJ and MPUTOBJ: object literal
 *    - LDTHIS: 'this'
 *    - LDTRUE and LDFALSE: true and false literals
 *    - TYPEOF: typeof of a local variable or a constant
 *    - TYPEOFID: typeof of a non-local variable
 */

var extVar = 0;  // dummy assignment target

/*---
{
    "slow": true
}
---*/

/*===
large expressions
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
123
===*/

function returnTrue() {
    return true;
}

function MyConstructor() {
}

function buildExpr(count) {
    var terms = [];
    var i;

    /* The returned expression is a giant &&-expression with a lot of terms.
     * Every term should be "truthy" to keep the evaluation going.  The last
     * term is always a fixed string which is tested against.
     */

    for (i = 0; i < count - 1; i++) {
        switch (i % 14) {
        case 0:
            // constructor call
            terms.push('new MyConstructor("dummy")');
            break;
        case 1:
            // external variable call
            terms.push('returnTrue("dummy")');
            break;
        case 2:
            // local variable call
            terms.push('returnTrueLocal("dummy")');
            break;
        case 3:
            // property call
            terms.push('obj.ret("dummy")');
            break;
        case 4:
            // object literal
            terms.push('{foo:1,bar:2}');
            break;
        case 5:
            // array literal
            terms.push('["foo","bar"]');
            break;
        case 6:
            // true and false
            terms.push('(false || true)');
            break;
        case 7:
            // typeof local variable
            terms.push('typeof returnTrueLocal');  // 'function' -> truthy
            break;
        case 8:
            // typeof constant
            terms.push('typeof "strval"');  // 'string' -> truthy
            break;
        case 9:
            // typeof non-local variable
            terms.push('typeof Math');      // 'string' -> truthy
            break;
        case 10:
            // setter and getter in object literal
            terms.push('{ get foo() {}, set foo(x) {} }');
            break;
        case 11:
            // setter and getter in object literal, call getter to ensure it work
            terms.push('({ get foo() { return true; }, set foo(x) {} }).foo');
            break;
        case 12:
            // setter and getter in object literal, call setter to ensure it work
            terms.push('(({ get foo() {}, set foo(x) { return true; } }).foo = 234)');
            break;
        case 13:
            // assignment to external variable (PUTVAR)
            terms.push('(extVar = 123)');
            break;
       }
    }
    terms.push('"last-term"');

    return "(function() {\n" +
           "    var returnTrueLocal = returnTrue;\n" +
           "    var obj = { ret: returnTrue };\n" +
           "    return " + terms.join(' && ') + ";\n" +
           "})();";
}

function largeExprTest() {
    var i, limit;
    var expr;
    var res;

    // Compiler recursion limit currently bites around 2500
    limit = 2000;

    for (i = 2; i < limit; i++) {
        if ((i % 100) == 0) { print(i); }

        expr = buildExpr(i);
        //print(expr);

        try {
            res = eval(expr)
            if (res !== 'last-term') {
                throw new Error('result does not match expected value, result was: ' + res);
            }
        } catch (e) {
            print('failed with i=' + i + ': ' + e);
            throw e;
        }
    }

    print(extVar);
}

print('large expressions')

try {
    largeExprTest();
} catch (e) {
    print(e.stack || e);
}
