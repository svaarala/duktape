/*
 *  Number objects (E5 Section 15.7).
 */

/*---
{
    "skip": true
}
---*/

/* FIXME */

/*===
undefined
true
true
false
===*/

/* When called as a constructor, a Number instance will always have the
 * original Number.prototype regardless of what Number.prototype is now
 * set to.  E5 Section 15.7.2.1.
 *
 * However, Number.prototype is not writable or configurable, so this
 * behavior doesn't need to be implemented explicitly; just ensure that
 * Number.prototype is not writable.
 */

var orig_prototype = Number.prototype;
var repl_prototype = { "foo": "bar" };
Number.prototype = repl_prototype;  /* this write will fail silently */

var num = new Number(123);
print(num.foo);
print(Object.getPrototypeOf(num) === Number.prototype);
print(Object.getPrototypeOf(num) === orig_prototype);
print(Object.getPrototypeOf(num) === repl_prototype);

