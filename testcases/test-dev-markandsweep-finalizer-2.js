/*
 *  Test Duktape specific mark-and-sweep initiated finalizer calling.
 *  In particular, test that "rescuing" and re-finalizing works
 *  correctly.
 *
 *  Note: the finalization order of 'a' and 'b' is not well defined,
 *  so finalizers should not print out anything as it cannot be
 *  checked.
 */

/*===
object
object
object
object
undefined
undefined
===*/

var a = {};
var b = {};
var rescue;

function finalizer_a(x) {
	if (rescue) {
		a = x;
	}
}

function finalizer_b(x) {
	if (rescue) {
		b = x;
	}
}

__duk__.setFinalizer(a, finalizer_a);
__duk__.setFinalizer(b, finalizer_b);

/* circular refs */
a.ref = b;
b.ref = a;

rescue = true;
a = undefined;
b = undefined;
__duk__.gc();
print(typeof a);
print(typeof b);

rescue = true;
a = undefined;
b = undefined;
__duk__.gc();
print(typeof a);
print(typeof b);

rescue = false;
a = undefined;
b = undefined;
__duk__.gc();
print(typeof a);
print(typeof b);

