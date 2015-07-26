/*---
{
    "comment": "breaks with DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY"
}
---*/

/*===
undefined
undefined
TypeError
TypeError
function function
function function
true
true
true
true
===*/

/* The 'caller' and 'arguments' properties of a strict function instance
 * are set to the thrower function of E5 Section 13.2.3.  See E5 Section
 * 13.2.1, step 19.
 *
 * A non-strict function instance should not have a 'caller' nor an
 * 'arguments' property at all.
 *
 * This testcase breaks with DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY.
 */

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

// the thrower is required to be the same object
try {
    pd1 = Object.getOwnPropertyDescriptor(g, 'caller');
    pd2 = Object.getOwnPropertyDescriptor(g, 'arguments');
    print(typeof pd1.get, typeof pd1.set);
    print(typeof pd2.get, typeof pd2.set);
    print(pd1.get === pd1.set);
    print(pd2.get === pd2.set);
    print(pd1.get === pd2.get);
    print(pd1.set === pd2.set);
} catch (e) {
    print(e.name);
}
