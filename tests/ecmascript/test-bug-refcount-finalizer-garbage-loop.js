/*
 *  Bug test for a Duktape 2.0 and prior refzero finalizer limitation for
 *  creating new garbage.  This has been fixed in Duktape 2.1.
 *
 *  When the finalizer runs and calls Object.getOwnPropertyNames(), an
 *  enumerator object referencing the object being finalized is created.
 *  When the refzero finalizer exits, the object will then have a refcount
 *  > 0, but will actually be unreachable because it's only reachable via
 *  the enumerator which gets collected right after the finalizer call
 *  returns.
 *
 *  So, when the finalizer exits, the enumerator object's refzero falls
 *  to zero and it is queued to the refzero_list for processing.  But
 *  it's not yet processed when the decision to rescue/free the finalized
 *  object is made, so the object is rescued and queued back to the heap.
 *
 *  When the enumerator object is refzero processed, it gets freed and
 *  refcount finalized, which causes the finalized object's refcount to
 *  fall to zero, too -- and the object is queued to refzero_list again.
 *
 *  This loop then continues forever.  The same happens with any object
 *  which references the object being finalized without being in a reference
 *  loop.
 *
 *  Duktape 2.1 fixes the issue by running finalizers outside of refzero_list
 *  processing, so that the newly created enumerator (or other "wrapper"
 *  object) gets DECREF processed immediately, and the keep/rescue decision
 *  is then made with up-to-date refcounts.
 */

/*===
no finalizer
caught Error: thrown by constructor
add finalizer
finalizer called for object
caught Error: thrown by constructor
done
finalizer called for prototype
===*/

var thisPointer;
var sanityCount = 0;

function finalizer(o) {
    if (o === Foo.prototype) {
        print('finalizer called for prototype');
        return;
    }

    print('finalizer called for object');
    if (++sanityCount > 1000) {
        // Break the finalizer loop for testing.  Without this, the loop
        // would go on forever.
        print('sanity limit reached');
        return;
    }

    // Create a temporary object referencing the object being finalized.
    // When temp is assigned null, the temporary object gets refzero queued
    // to refzero_list, but won't be actually processed because we're already
    // processing the finalizer for the current object inside refzero_list
    // handling.
    var temp = { name: 'temp', ref: o };
    //temp.foo = { bar: temp };  // ref loop
    temp = null;

    // If 'temp' was in a reference loop, it would only be collectable via
    // mark-and-sweep, and the *second* finalization round would then be
    // mark-and-sweep driven, avoiding the infinite loop.

    // This would cause the same issue.
    // void Object.getOwnPropertyNames(o);
}

function Foo() {
    thisPointer = String(Duktape.Pointer(this));

    // If the object is placed into a reference loop, the finalization will
    // be handled via mark-and-sweep which works fine.

    /*
    this.foo = {};
    this.foo.bar = this;
    */

    this.name = 'Foo instance';

    throw new Error('thrown by constructor');
}

print('no finalizer');
try {
    new Foo();
} catch (e) {
    print('caught', e);
}

print('add finalizer');
Duktape.fin(Foo.prototype, finalizer);
Duktape.gc();

try {
    new Foo();
} catch (e) {
    print('caught', e);
}

print('done');
