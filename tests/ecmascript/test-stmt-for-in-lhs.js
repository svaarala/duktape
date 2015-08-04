/*
 *  The target variable in a for-in statement is a LeftHandSideExpression:
 *
 *      LeftHandSideExpression :
 *          NewExpression
 *          CallExpression
 *
 *  Together, these productions allow a lot of valid left-hand-side values.
 *  Some are useful (e.g. 'x' or 'x.foo') while others are less so (e.g.
 *  [ 1, 2, 3 ]).  All these LHS values must be accepted during compilation
 *  but they can throw a ReferenceError at runtime.
 *
 *  These forms are apparently allowed syntactically to make it possible
 *  to use an implementation specific LHS value which can be returned from
 *  a function and still get assigned to, i.e. "for (func() in bar) { ... }".
 *
 *  There are a lot of expressions that are not allowed and must cause a
 *  SyntaxError, e.g. 'x++', 'a + b', etc.
 *
 *  Currently Duktape (at least up to Duktape 1.1) will accept any expression
 *  as the LHS (not just LeftHandSideExpression), throwing a ReferenceError
 *  at runtime instead of a SyntaxError at compile time (V8 seems to do the
 *  same thing).  Engines seem to vary a lot in how they deal with odd or
 *  invalid LHS values.
 */

/*===
i: 1
0 no error
i: 1
1 no error
obj.foo: 1
2 no error
obj.bar: 1
3 no error
i: 1
4 no error
obj.foo: 1
5 no error
obj.bar: 1
6 no error
7 ReferenceError
8 ReferenceError
9 ReferenceError
10 ReferenceError
11 ReferenceError
rhs
lhs
12 ReferenceError
rhs
should get here
13 no error
14 SyntaxError
15 SyntaxError
16 SyntaxError
17 SyntaxError
18 SyntaxError
19 SyntaxError
20 SyntaxError
21 SyntaxError
22 SyntaxError
23 SyntaxError
24 SyntaxError
===*/

function func() {
}

function MyCons() {
}

function lhsTest() {
    [
        /* Variable declaration variant. */
        'for (var i in ["foo","bar"]) {}; print("i: " + i);',

        /* These are valid and useful LeftHandSideExpressions. */
        'var i; for (i in ["foo","bar"]) {}; print("i: " + i);',
        'var obj={}; for (obj.foo in ["foo","bar"]) {}; print("obj.foo: " + obj.foo);',
        'var obj={}; for (obj["bar"] in ["foo","bar"]) {}; print("obj.bar: " + obj.bar);',

        /* The LHS target may be in parenthesis. */
        'var i; for ((i) in ["foo","bar"]) {}; print("i: " + i);',
        'var obj={}; for ((obj.foo) in ["foo","bar"]) {}; print("obj.foo: " + obj.foo);',
        'var obj={}; for ((obj["bar"]) in ["foo","bar"]) {}; print("obj.bar: " + obj.bar);',

        /* These are valid but not so useful LeftHandSideExpressions,
         * should cause ReferenceErrors at runtime.
         */
        'for (this in ["foo","bar"]) {}; print("this: " + obj.foo);',
        'for ([1,2,3] in ["foo","bar"]) {};',
        'for ({ foo: 1 } in ["foo","bar"]) {};',
        'for (func() in ["foo","bar"]) {};',
        'for (new MyCons() in ["foo","bar"]) {};',

        /* Even if a ReferenceError is thrown, the offending LHS must still
         * be evaluated for its side effects.  (V8 won't print 'lhs' here.)
         */
        'for (print("lhs") in print("rhs"), [0,1]) {};',

        /* If the RHS evaluates to an empty list, the offending LHS is never
         * evaluated and a ReferenceError must not be thrown.
         */
        'for (print("lhs") in print("rhs"), []) {}; print("should get here");',

        /* These are invalid LHS values so a SyntaxError is required.
         * This list is not exhaustive, just a sample here and there.
         * (At least up to Duktape 1.1 these incorrecly cause a ReferenceError.)
         */
        'var x; for (x++ in [0,1]) {}',
        'var foo; for (delete foo in [0,1]) {}',
        'var x; for (void x in [0,1]) {}',
        'var x; for (typeof x in [0,1]) {}',
        'var x; for (++x in [0,1]) {}',
        'var x; for (+x in [0,1]) {}',
        'var a, b; for (a + b in [0,1]) {}',
        'var a, b; for (a * b in [0,1]) {}',
        'var a; for (a >> 2 in [0,1]) {}',
        'var a, b; for (a && b in [0,1]) {}',
        'var a; for (a = 2 in [0,1]) {}',
    ].forEach(function (val, idx) {
        try {
            // The eval() call establishes variables in the current
            // function so avoid clashes.
            eval(val);
            print(idx, 'no error');
        } catch (e) {
            print(idx, e.name);
//            print(e.stack);
        }
    });
}

try {
    lhsTest();
} catch (e) {
    print(e.name);
}
