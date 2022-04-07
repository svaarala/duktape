/*===
TypeError
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
