/*
 *  Long chain of references going to refcount zero.
 *
 *  In a basic refcount implementation this causes recursive C invocations
 *  of the handler for zero reference count.
 */

/*===
still here
===*/

var x;
var t;
var i;

for (i = 0; i < 100000; i++) {
	t = {};
	t.ref = x;
	x = t;
}

x = null;  /* refcount to zero in the whole chain */

/* FIXME: some gc stats thing here? */
print("still here");

