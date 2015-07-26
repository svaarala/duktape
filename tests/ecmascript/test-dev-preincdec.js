/*===
10
19
20
9
10
===*/

var x = 10;
print(x);
print(--x + 10);
print(++x + 10);
print(--(x));  // parens are OK
print(++(x));

/*===
10
19
20
9
10
===*/

var obj = {x:10};
print(obj.x);
print(--obj.x + 10);
print(++obj.x + 10);
print(--(obj.x));
print(++(obj.x));

/*===
should print
ReferenceError
===*/

/* Invalid LHS parses but causes a run-time ReferenceError.
 *
 * Rhino and V8 don't print 'should print' which seems incorrect: the
 * UnaryExpression should be evaluated before the ReferenceError occurs.
 */

try {
    eval("++print('should print');");
} catch (e) {
    print(e.name);
}

/*===
NaN
NaN
-1
1
-1
1
0
2
122
124
NaN
NaN
===*/

/* The value is coerced with ToNumber() */

var y;

y = undefined; print(--y);
y = undefined; print(++y);
y = null; print(--y);
y = null; print(++y);
y = false; print(--y);
y = false; print(++y);
y = true; print(--y);   // ToNumber(true) -> 1
y = true; print(++y);
y = '123'; print(--y);  // ToNumber('123') -> 123
y = '123'; print(++y);
y = 'foo'; print(--y);  // ToNumber('foo') -> NaN
y = 'foo'; print(++y);

/*===
999
999
1001
1001
===*/

/* ToNumber() eventually calls valueOf().  Here ToNumber(obj) is thus 1000.
 */

obj = {};
obj.valueOf = function() {
    return 1000;
}
print(--obj);
print(obj);

obj = {};
obj.valueOf = function() {
    return 1000;
}
print(++obj);
print(obj);


/*===
eval
arguments
SyntaxError
SyntaxError
===*/

/* Attempt to use 'eval' or 'arguments' as an identifier name is
 * a SyntaxError in strict mode.
 */

var old_eval = eval;

try {
    // Note: this will clobber eval
    eval("function f1() { ++eval; --eval; print('eval'); }; f1();");
} catch (e) {
    print(e.name);
}
eval = old_eval;

try {
    eval("function f2() { ++arguments; --arguments; print('arguments'); }; f2();");
} catch (e) {
    print(e.name);
}

try {
    /* Rhino allows this, and clobbers eval in the process. */
    eval("function f3() { 'use strict'; ++eval; --eval; }; f3();");
} catch (e) {
    print(e.name);
}
eval = old_eval;  // just in case

try {
    eval("function f4() { 'use strict'; ++arguments; --arguments; }; f4();");
} catch (e) {
    print(e.name);
}
