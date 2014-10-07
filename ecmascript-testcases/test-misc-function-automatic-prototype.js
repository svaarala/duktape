/*
 *  A newly created function always has a fresh object as its
 *  prototype.
 *
 *  E5 Sections 15.3.2.1, 15.3.5.2, 15.3.4.5.
 */

var f, g;
var a;

/*===
object
true
object
false
object
object
===*/

f = function() { print("f"); };
print(typeof f.prototype);
print(f.prototype.constructor === f);

function func() { print("func"); }
print(typeof func.prototype);

print(f === func);

a = {
   get x() { print(typeof arguments.callee.prototype); },
   set x() { print(typeof arguments.callee.prototype); }
};

t = a.x;
a.x = 1;

/*===
f 1 2 undefined
undefined
===*/

f = function(x,y) { print("f", this, x, y); };
g = f.bind(1, 2);
g();

/* g.prototype should be missing, see E5 Section 15.3.4.5 */
/* Note: v8 fails this test */
d = Object.getOwnPropertyDescriptor(g, 'prototype');
print(typeof d);
