/*
 *  Test Duktape specific reference counter initiated finalizer calling.
 *  In particular, test that "rescuing" and re-finalizing an object
 *  multiple times works correctly.
 */

/*---
{
    "custom": true
}
---*/

/*===
rescued
object
rescued
object
not rescued
undefined
===*/

var o = { foo: "bar" };
var rescue;

function finalizer(x) {
	if (rescue) {
		o = x;
		print("rescued");
	} else {
		print("not rescued");
	}
}

Duktape.fin(o, finalizer);

rescue = true;
o = undefined;  // refzero, finalize, gets rescued
print(typeof o);

rescue = true;
o = undefined;  // refzero, finalize, gets rescued
print(typeof o);

rescue = false;
o = undefined;  // refzero, finalize, not rescue
print(typeof o);
