/*
 *  Tests for the 'caller' property of an arguments object created
 *  for a non-strict callee.
 */

/*---
{
    "comment": "breaks with DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY"
}
---*/

/*===
object foo bar
undefined undefined
123 123
dummy dummy
g
TypeError
===*/

// non-strict callee
function f(x,y) { return arguments; }

// strict caller
function g() { 'use strict'; return f('foo', 'bar'); }

// dummy non-strict function
function dummy() {}

f.myName = 'f';
g.myName = 'g';
dummy.myName = 'dummy';

function test() {
    // Create arguments object for the case where a strict function (g)
    // calls a non-strict caller (f).
    a = g();
    print(typeof a, a[0], a[1]);

    // Initially there is no 'caller' property at all, and thus no
    // special behavior.
    print(Object.getOwnPropertyDescriptor(a, "caller"), a.caller);

    // Setting 'caller' to a non-function value triggers no special
    // behavior.

    a.caller = 123;
    print(Object.getOwnPropertyDescriptor(a, "caller").value, a.caller);

    // Setting 'caller' to a non-strict function also triggers no
    // special behavior.

    a.caller = dummy;
    print(Object.getOwnPropertyDescriptor(a, "caller").value.myName, a.caller.myName);

    // Setting 'caller' to a strict function (any strict function, but
    // here we set it to 'g') triggers special behavior.

    a.caller = g;

    // this is OK, the special behavior *only* happens at the [[Get]] level
    print(Object.getOwnPropertyDescriptor(a, "caller").value.myName);

    // this fails due to special behavior in [[Get]]
    print(a.caller.myName);
}

try {
    test();
} catch (e) {
    print(e.name);
}
