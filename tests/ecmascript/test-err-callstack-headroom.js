/*
 *  Error handling callstack headroom (GH-191)
 *
 *  When a RangeError is thrown due to the Ecmascript callstack limit being
 *  reached, there should be enough headroom for at least 10 further recursions.
 *  Prior to Duktape 1.3, this wasn't the case and a DoubleError would be
 *  generated if the callstack limit was reached and, e.g. Duktape.errCreate
 *  is set.
 */

/*---
{
    "custom": true
}
---*/

/*===
10
9
8
7
6
5
4
3
2
1
RangeError: callstack limit
===*/

try {
	function recurse(n) {
		print(n);
		if (--n > 0) recurse(n);
	}

	Duktape.errCreate = function(e) {
		recurse(10);
		return e;
	};

	function f() { f(); }
	f();
}
catch(e) {
	print(e);
}
