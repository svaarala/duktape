/*
 *  In Duktape 2.1 mark-and-sweep can run while we process finalize_list.
 *
 *  Exercise the feature by running GC during finalization and seeing that
 *  finalizer decisions still come out correctly.  In particular, an object
 *  must not be rescued if it's unreachable except for references coming
 *  from finalize_list.
 */

/*===
gc 1
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
finalizer called
finalizer exiting
gc 1 returned
gc 2
gc 2 returned
finCount: 10
===*/

var finCount = 0;

function fin(o) {
    finCount++;
    print('finalizer called');
    Duktape.gc();
    Duktape.gc();
    print('finalizer exiting');

    // Create a lot of collectable circular garbage with explicit GC.
    // This has no outward effect as such, but can be seen in memory
    // behavior with massif.  In Duktape 2.1 memory usage will be flat,
    // in Duktape 2.0 and prior it will increase during finalizer
    // execution.

    for (var i = 0; i < 1e5; i++) {
        var obj = { foo: 123 };
        obj.ref = {};
        obj.ref.ref = obj;

        obj = null;
        Duktape.gc();
    }
}

function createObjects() {
    // Create a set of finalizable objects.
    var arr = [];
    while (arr.length < 10) {
        var obj = {};
        Duktape.fin(obj, fin);
        arr.push(obj);
    }

    // Place the objects into a circular reference relation.
    for (var i = 0; i < arr.length; i++) {
        arr[i].ref = arr[(i + 1) % arr.length];
    }

    return arr;
}

var arr = createObjects();
Duktape.gc();

// Make the whole set unreachable at the same time.  Because of
// the circular references, finalization only happens after an
// explicit GC.
arr = null;

print('gc 1');
Duktape.gc();
print('gc 1 returned');
print('gc 2');
Duktape.gc();  // rescue/free
print('gc 2 returned');

print('finCount:', finCount);
