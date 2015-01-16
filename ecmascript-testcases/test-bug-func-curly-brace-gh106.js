/*
 *  https://github.com/svaarala/duktape/issues/106
 */

/*===
SyntaxError
SyntaxError
===*/

function test1() {
    var src = 'function foo() [}; foo;';
    print(typeof eval(src));
}

function test2() {
    var src = '(function () [})';
    print(typeof eval(src));
}

try {
    test1();
} catch (e) {
    print(e.name);
}

try {
    test2();
} catch (e) {
    print(e.name);
}
