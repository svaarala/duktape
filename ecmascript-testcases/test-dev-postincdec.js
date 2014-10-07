/*===
10
20
19
10
9
10
===*/

var x = 10;
print(x);
print(x-- + 10);
print(x++ + 10);
print((x)--);  // parens are OK
print((x)++);
print(x);

/*===
10
20
19
10
9
10
===*/

var obj = {x:10};
print(obj.x);
print(obj.x-- + 10);
print(obj.x++ + 10);
print((obj.x)--);
print((obj.x)++);
print(obj.x);

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
    eval("print('should print')++;");
} catch (e) {
    print(e.name);
}

/*===
NaN NaN
NaN NaN
0 -1
0 1
0 -1
0 1
1 0
1 2
123 122
123 124
NaN NaN
NaN NaN
number 124
===*/

/* The value is coerced with ToNumber().  Note that the value assigned to
 * the post-increment/decrement is the previous value of the target, but
 * even that value will be coerced using ToNumber()!
 *
 * Hence, "y=undefined; print(y--)" will NOT print "undefined" but "NaN".
 */

var y;

y = undefined; print(y--, y);
y = undefined; print(y++, y);
y = null; print(y--, y);
y = null; print(y++, y);
y = false; print(y--, y);
y = false; print(y++, y);
y = true; print(y--, y);   // ToNumber(true) -> 1
y = true; print(y++, y);
y = '123'; print(y--, y);  // ToNumber('123') -> 123
y = '123'; print(y++, y);
y = 'foo'; print(y--, y);  // ToNumber('foo') -> NaN
y = 'foo'; print(y++, y);

y = '123'; print(typeof y++, y);  // ToNumber() is applied to OLD value too

/*===
1000
999
1000
1001
===*/

/* ToNumber() eventually calls valueOf().  Here ToNumber(obj) is thus 1000.
 */

obj = {};
obj.valueOf = function() {
    return 1000;
}
print(obj--);
print(obj);

obj = {};
obj.valueOf = function() {
    return 1000;
}
print(obj++);
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
    eval("function f1() { eval++; eval--; print('eval'); }; f1();");
} catch (e) {
    print(e.name);
}
eval = old_eval;

try {
    eval("function f2() { arguments++; arguments--; print('arguments'); }; f2();");
} catch (e) {
    print(e.name);
}

try {
    /* Rhino allows this, and clobbers eval in the process. */
    eval("function f3() { 'use strict'; eval++; eval--; }; f3();");
} catch (e) {
    print(e.name);
}
eval = old_eval;  // just in case

try {
    eval("function f4() { 'use strict'; arguments++; arguments--; }; f4();");
} catch (e) {
    print(e.name);
}

/*===
ReferenceError
11
===*/

/* Precedence: ++z++ parses as ++(z++).  The expression should cause a
 * ReferenceError because the result of "z++" won't be a valid LHS.
 * However, it is not a SyntaxError, and the postincrement SHOULD take
 * place.
 */

var z = 10;

try {
    /* Note: Rhino won't even parse this, so wrap in an eval to allow Rhino
     * testing of other cases here.
     *
     * Note: V8 will print '10' below, it will detect the invalid LHS case
     * without executing the postincrement.  This seems to be contrary to
     * the specification?
     */
    eval("print(++z++);");
} catch (e) {
    print(e.name);
}
print(z);
