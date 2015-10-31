/*
 *  Minimal support for 'const'
 */

/*---
{
    "custom": true
}
---*/

/*===
123 234 357
===*/

/* Normal intended use. */

function constBasicTest() {
    const x = 123;
    var y = 234;
    print(x, y, x + y);
}

try {
    constBasicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
1000 234 1234
===*/

/* The minimal 'const' implementation treats a 'const' like 'var' so that the
 * constant is actually a normal variable and thus writable.
 */

function constWriteTest() {
    const x = 123;
    var y = 234;
    x = 1000;
    print(x, y, x + y);
}

try {
    constWriteTest();
} catch (e) {
    print(e.stack || e);
}

/*===
SyntaxError
===*/

/* The minimal 'const' implementation requires an initializer for 'const'
 * (which is not required for 'var').
 */

function noConstInitializerTest() {
    try {
        eval('(function test() { const x; print(x); })')();
    } catch (e) {
        print(e.name);
    }
}

try {
    noConstInitializerTest();
} catch (e) {
    print(e.stack || e);
}

/*===
234
===*/

/* Re-declaring a 'const' as a 'var' is silently allowed because that's treated
 * like re-declaring a variable.
 */

function redeclareConstAsVarTest() {
    try {
        eval('(function test() { const x = 123; var x = 234; print(x); })')();
    } catch (e) {
        print(e.name);
    }
}

try {
    redeclareConstAsVarTest();
} catch (e) {
    print(e.stack || e);
}
