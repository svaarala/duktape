/*
 *  Reported by Karl Dahlke.
 */

/*===
true
true
true
===*/

var src1 = '/blah/.test("blah")';
var src2 = 'if (1==1) { /blah/.test("blah") }';
var src3 = 'if (1==1) /blah/.test("blah")';

print(eval(src1));
print(eval(src2));

// This form failed to parse at least up to Duktape 2.2.
print(eval(src3));
