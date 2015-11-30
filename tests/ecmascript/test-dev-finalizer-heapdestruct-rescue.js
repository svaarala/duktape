/*
 *  An object rescued just before heap destruction will get its finalizer
 *  executed once more in forced finalization.  This is easy to get wrong
 *  because an object may have FINALIZED flag set, but not yet be rescued
 *  (by another mark-and-sweep round).  User code would then expect the
 *  finalizer to run once more - but the FINALIZED flag would prevent it.
 */

/*===
created object 1
object 1 finalized before heap destruction, create new object, rescue current
created object 2
object 2 finalized before heap destruction, create new object, rescue current
created object 3
object 3 finalized during heap destruction, don't create another object
object 2 finalized during heap destruction, don't create another object
object 1 finalized during heap destruction, don't create another object
===*/

var objCount = 0;
var rescued = [];

function mkObj() {
    var obj = {};
    var other = {};
    obj.ref = other; other.ref = obj; other = null;  // ensure cycle -> mark-and-sweep
    var myCount = ++objCount;

    print('created object', myCount);
    Duktape.fin(obj, function finalize(o, heapDestruct) {
        if (heapDestruct) {
            print('object', myCount, 'finalized during heap destruction, don\'t create another object');
        } else {
            print('object', myCount, 'finalized before heap destruction, create new object, rescue current');
            rescued.push(o);
            void mkObj();
        }
    });
}

void mkObj();

// Begin heap destruction

// At the moment object 2 will be finalized on the last normal forced GC round
// and will then be rescued and re-finalized in forced finalization as required
// by finalizer guarantees: the 'heapDestruct' flag is false when the finalizer
// runs for the first time so rescuing is allowed.
//
// Object 3 is first finalized during forced finalization, so that its finalizer
// only runs once.  This is correct: the 'heapDestruct' flag is true when the
// finalizer runs, so rescuing is not supported.
