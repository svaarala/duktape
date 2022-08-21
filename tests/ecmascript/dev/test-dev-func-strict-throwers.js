/*---
comment: "breaks with DUK_USE_NONSTD_FUNC_CALLER_PROPERTY"
---*/

/*===
TypeError
TypeError
TypeError
TypeError
===*/

function f() { }

function g() { 'use strict' }

var pd1, pd2;

try {
    print(f.caller);
} catch (e) {
    print(e.name);
}

try {
    print(f.arguments);
} catch (e) {
    print(e.name);
}

try {
    print(g.caller);
} catch (e) {
    print(e.name);
}

try {
    print(g.arguments);
} catch (e) {
    print(e.name);
}
