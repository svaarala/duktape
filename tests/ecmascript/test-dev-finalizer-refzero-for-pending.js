/*
 *  Two objects in a reference loop on finalize_list, both having finalizers.
 *  Each finalizer will break the reference to the other object.
 *
 *  Assume X's finalizer executes first.  It deletes X.ref, so that Y's refcount
 *  drops to zero and Y gets refzero processed while still on finalize_list.
 *  This causes heap list corruption because refzero should only happen for
 *  objects on heap_allocated.
 *
 *  This is avoided by preincrementing refcounts when an object is inserted to
 *  finalize_list (cf. how mark-and-sweep considers finalize_list objects
 *  reachability roots).  This ensures that the refcount remains >= 1 at all
 *  times, and can only drop to zero once the finalizer for the object is
 *  complete.
 */

/*===
gc 1
lose refs
gc 2
fin x
fin y
still here
===*/

var x = {};
var y = {};
x.ref = y;
y.ref = x;

Duktape.fin(x, function (v) { print('fin x'); v.ref = null; });
Duktape.fin(y, function (v) { print('fin y'); v.ref = null; });

print('gc 1');
Duktape.gc();
print('lose refs');
x = y = null;
print('gc 2');
Duktape.gc();

print('still here');
