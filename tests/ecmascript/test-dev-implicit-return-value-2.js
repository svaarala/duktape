/*
 *  Implicit return value of global code, eval code, and function code.
 *
 *  The return value of a function unless it returns with an explicit
 *  value is always 'undefined', see E5 Sections 11.2.3 and 13.2.1.
 *
 *  The return value of a program (global code or eval code) is the
 *  value of its last statement.  A 'return' statement outside of a
 *  function body is, in fact, a SyntaxError (E5 Section 12.9).
 *  See E5 Sections 14, 15.1.2.1.
 */

/*===
3 number
undefined undefined
3 number
===*/

var t;

t = eval("1+2");
print(t, typeof t);

function f1(a,b) { a+b; }
function f2(a,b) { return a+b; }

t = f1(1,2);
print(t, typeof t);

t = f2(1,2);
print(t, typeof t);
