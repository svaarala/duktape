/*
 *  A finalizer won't be re-executed by heap destruction if the object is
 *  still in the heap when forced finalizer execution begins.
 *
 *  This is a bit difficult to arrange: heap destruction begins with a few
 *  normal GC rounds so normal unreachable objects will be finalized on the
 *  first round and the second round frees them.  What we need is for the last
 *  normal GC round to run a finalizer, but with the object still sitting
 *  uncollected in the heap.   We arrange this by creating a new finalizable
 *  object on every finalizer run, so that there will always be a finalized
 *  but uncollected object by the time forced finalization begins.
 *
 *  In Duktape 1.3.0 and prior, the finalizer of at least one object would
 *  run twice.
 */

/*===
created object 1
object 1 finalized before heap destruction, create new object
created object 2
object 2 finalized before heap destruction, create new object
created object 3
object 3 finalized during heap destruction, don't create another object
===*/

var objCount = 0;

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
            print('object', myCount, 'finalized before heap destruction, create new object');
            void mkObj();
        }
    });
}

void mkObj();

// Begin heap destruction
