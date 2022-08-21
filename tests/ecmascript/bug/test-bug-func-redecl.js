/*
 *  Broken at some point; regression test.
 */

/*===
redecl
===*/

function foo(x,y) {
    print(x,y);
}

function foo(x,y) {
    print('redecl');
}

foo(1,2);

/*===
second 1 2
===*/

/* Shadowing declarations inside a function (register bound) */

function functest() {
    function foo(x,y) { print('first',x,y); }
    function foo(x,y) { print('second',x,y); }

    foo(1,2);
}

functest();
