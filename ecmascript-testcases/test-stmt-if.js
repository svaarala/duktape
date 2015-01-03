/*
 *  If statement (E5 Section 12.5).
 *
 *  Syntax:
 *
 *    if ( Expression ) Statement else Statement
 *    if ( Expression ) Statement
 *
 *  If statement uses ToBoolean() coercing which doesn't invoke valueOf().
 *  Logical NOT (!!v) doesn't either, so we use number coercion (+v) to
 *  invoke valueOf which then gets ToBoolean() coerced.  This allows printing
 *  the if-else evaluation order.
 */

function mklogged(name, val) {
    return {
        valueOf: function () {
            print('coerced ' + name);
            return val;
        }
    }
}

/*===
coerced argtrue
then
coerced argfalse
else
coerced argtrue
then
coerced argfalse
---
coerced atrue
a
---
coerced atrue
a
---
coerced atrue
a
---
coerced atrue
a
---
coerced afalse
coerced btrue
b
---
coerced afalse
coerced btrue
b
---
coerced afalse
coerced bfalse
coerced ctrue
c
---
coerced afalse
coerced bfalse
coerced cfalse
d
===*/

/* Test basic if-then-else and if-then control flow structures. */

function testIfElse(v) {
    if (+v) {
        print('then');
    } else {
        print('else');
    }
}

function testIf(v) {
    if (+v) {
        print('then');
    }
}

/* Test binding of 'else' in a multiple if-else case. */
function testIfLadder(a, b, c, d) {
    if (+a) {
        print('a');
    } else if (+b) {
        print('b');
    } else if (+c) {
        print('c');
    } else {
        print('d');
    }
}

try {
    testIfElse(mklogged('argtrue', true));
    testIfElse(mklogged('argfalse', false));
} catch (e) {
    print(e.stack || e);
}

try {
    testIf(mklogged('argtrue', true));
    testIf(mklogged('argfalse', false));
} catch (e) {
    print(e.stack || e);
}

try {
    [ mklogged('atrue', true), mklogged('afalse', false) ].forEach(function (a) {
        [ mklogged('btrue', true), mklogged('bfalse', false) ].forEach(function (b) {
            [ mklogged('ctrue', true), mklogged('cfalse', false) ].forEach(function (c) {
                print('---');
                testIfLadder(a, b, c);
            });
        });
    });
} catch (e) {
    print(e.stack || e);
}

/*===
0 else
1 else
2 then
3 else
4 then
5 then
6 else
7 else
8 then
9 then
10 else
11 then
12 then
13 then
14 then
15 then
===*/

/* Test coercion of various primitive values.  Basically this just tests
 * that ToBoolean() is used.
 */

function testIfCoercion() {
    [ undefined, null, true, false, Number.NEGATIVE_INFINITY, -123, -0, 0, 123,
      Number.POSITIVE_INFINITY, Number.NaN, {}, [], function () {},
      mklogged('nevercoerced', true), mklogged('nevercoerced', false) ].forEach(function (v, i) {
        if (v) {
            print(i, 'then');
        } else {
            print(i, 'else');
        }
    });
}

try {
    testIfCoercion();
} catch (e) {
    print(e.stack || e);
}
