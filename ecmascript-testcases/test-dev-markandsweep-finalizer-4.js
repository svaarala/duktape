/*
 *  Heap destruction runs a few mark-and-sweep passes to ensure finalizers
 *  have a chance to run even if no explicit mark-and-sweep happens after
 *  such objects have been created.
 */

/*===
finalizing
finalizing
===*/

function fin(o) {
    // because order of finalization is not guaranteed, don't print
    // object name
    print('finalizing');
}

var obj1 = { name: 'obj1' };
var obj2 = { name: 'obj2' };
Duktape.fin(obj1, fin);
Duktape.fin(obj2, fin);
obj1.ref = obj2;  // use circular reference to prevent refcount collection
obj2.ref = obj1;
obj1 = null;
obj2 = null;

// obj1 and obj2 are unreachable but mark-and-sweep has not usually
// happened; mark-and-sweeps are executed forcibly by duk_destroy_heap()
// after the program is complete
