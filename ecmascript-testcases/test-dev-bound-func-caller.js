/*
 *  The 'caller' property for functions is rather interesting: it is a
 *  thrower for strict functions and all bound functions (strict AND
 *  non-strict).  It is undefined for non-strict non-bound functions.
 *
 *  V8 behavior mostly agrees but 'caller' will be set to 'null' for
 *  non-strict functions (instead of being undefined).  Rhino behavior
 *  does not agree; bound non-strict functions don't have a 'caller'
 *  property in Rhino.
 *
 *  E5.1 Sections 15.3.5, 15.3.4.5.
 */

/*===
undefined
TypeError
TypeError
TypeError
===*/

function test1a() {
    function f() {}
    print(f.caller);
}

function test1b() {
    function f() {}
    g = f.bind('dummy');
    print(g.caller);
}

function test2a() {
    function f() { 'use strict'; }
    print(f.caller);
}

function test2b() {
    function f() { 'use strict'; }
    g = f.bind('dummy');
    print(g.caller);
}

try {
    test1a();
} catch (e) {
    print(e.name);
}

try {
    test1b();
} catch (e) {
    print(e.name);
}

try {
    test2a();
} catch (e) {
    print(e.name);
}

try {
    test2b();
} catch (e) {
    print(e.name);
}
