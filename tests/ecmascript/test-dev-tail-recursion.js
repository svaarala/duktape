/*
 *  Tail recursion is not a guaranteed property of a compliant
 *  implementation.  In fact neither Rhino nor V8 seems to have it.
 *  Since we do, test that it works.
 */

/*---
{
    "custom": true,
    "comment": "breaks with DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY"
}
---*/

/*===
1000000
===*/

/* Very basic case */

function sum1(a, b) {
	if (b == 0) {
		return a;
	}
	return sum1(a + 1, b - 1);
}

try {
    print(sum1(0, 1000000));
} catch (e) {
    print(e.name);
}

/*===
1000000
1000000
===*/

/* The return expression may contain preceding stuff as long as its evaluation
 * ends with a "call + return" pair with nothing in the middle.  Here we test
 * that a comma expression works (although this is not that important as such).
 */

var count = 0;
function commaop() {
	count++;
}

function sum2(a, b) {
	if (b == 0) {
		return a;
	}
	return commaop(), sum2(a + 1, b - 1);
}

try {
    print(sum2(0, 1000000));
} catch (e) {
    print(e.name);
}

print(count);

/*===
25
RangeError
===*/

/* FIXME: Change limit errors to RangeError? */

/* Since both tail calls and eval calls are handled by control flags of the
 * CALL instruction, it's worth testing that if both happen at the same time,
 * things work as expected.
 *
 * The call target to a combined eval+tail call is a native function, so no
 * tail calling actually takes place.  This test just ensures that the calling
 * process still works correctly and fulfills eval semantics.
 */

function tail_eval(a,b) {
	if (b == 0) {
		return a;
	}
	return eval("tail_eval(a + 1, b - 1)");
}

try {
    // This should fit in callstack even without tailcalls.
    // C recursion limit is now 60, and each tail_eval()
    // level requires two C calls (one for eval, one for
    // bytecode executor)
    print(tail_eval(0, 25));
} catch (e) {
    print(e.name);
}

try {
    // this should probably not
    print(tail_eval(0, 100000));
} catch (e) {
    print(e.name);
}
