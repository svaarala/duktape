/*
 *  Weak references
 *
 *  The current weak reference model is that an object is marked as being
 *  weak as a whole with the following effects:
 *
 *  - All own properties are treated as weak.
 *  - Property keys are treated as strong.
 *  - The internal prototype and inherited properties are treated as strong.
 *
 *  There are several internal cases for internal references:
 *
 *    - Ordinary property
 *    - Setter reference of an accessor property
 *    - Getter reference of an accessor property
 *    - Ordinary property residing in the array part
 *
 *  This testcase comments on how things are supposed to work, but some of the
 *  underlying weak reference operations can only be observed from debug logs.
 */

function finalizer() {
    print('finalizer');
}

function finalizer1() {
    print('finalizer1');
}

function finalizer2() {
    print('finalizer2');
}

function finalizer3() {
    print('finalizer3');
}

var rescueTmp = null;
function finalizerRescue(v) {
    if (v.alreadyRescued) {
        print('finalizerRescue: already rescued, allow free');
        rescueTmp = null;
        return;
    }
    print('finalizerRescue: rescue on the first time');
    rescueTmp = v;
    v.alreadyRescued = true;
}

/*===
basic test
before 1st gc
true
after 1st gc
before 2nd gc
finalizer
true
after 2nd gc
true object
before 3rd gc
true
after 3rd gc
false undefined
===*/

function basicTest() {
    /*
     *  weak --[ref]--> obj
     */

    var obj = {};
    Duktape.fin(obj, finalizer);

    var weak = { ref: obj };
    Duktape.weak(weak);

    /* Force GC to see that 'obj' is still strongly reachable. */

    print('before 1st gc');
    print(Duktape.gc());
    print('after 1st gc');

    /* Remove strong reference and run GC.  The finalizer will run on
     * this GC round but the weak reference won't yet be removed.
     */

    obj = null;

    print('before 2nd gc');
    print(Duktape.gc());
    print('after 2nd gc');

    /* At this point the finalizer has been executed and the value will be
     * swept on the next round.
     *
     * At the moment the property can still be read back through the weak
     * reference, despite being finalized.  Ideally it would read back as
     * undefined, so that it wouldn't get accidentally rescued through the
     * weak reference.
     */

    print('ref' in weak, typeof weak.ref);

    /* Force GC for a third time.  Because the finalizer did not rescue the
     * object, it will be swept on this round, and the weak reference will
     * be deleted.
     */

    print('before 3rd gc');
    print(Duktape.gc());
    print('after 3rd gc');

    /* After the previous GC, the property no longer exists. */

    print('ref' in weak, typeof weak.ref);
}

try {
    print('basic test');
    basicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
weak ref type test
weak ref in entry part
gc 1
gc 2
finalizer
true object
get: undefined
set: undefined
gc 3
false undefined
get: undefined
set: undefined
weak ref in getter slot
gc 1
gc 2
finalizer
true undefined
get: function
set: undefined
gc 3
false undefined
get: undefined
set: undefined
weak ref in setter slot
gc 1
gc 2
finalizer
true undefined
get: undefined
set: function
gc 3
false undefined
get: undefined
set: undefined
weak ref in array part
gc 1
gc 2
finalizer
true object
gc 3
false undefined
weak refs in both setter and getter slot, removed at the same time
gc 1
gc 2
finalizer1
finalizer2
true undefined
get: function
set: function
gc 3
false undefined
get: undefined
set: undefined
weak refs in both setter and getter slot, removed one at a time
gc 1
gc 2
finalizer1
true undefined
get: function
set: function
gc 3
true undefined
get: undefined
set: function
gc 4
finalizer2
true undefined
get: undefined
set: function
gc 5
false undefined
get: undefined
set: undefined
===*/

function weakRefTypeTest() {
    /* Go through the internal cases where internal references may reside.
     * This affects the process of removing weak references for objects
     * about to be swept by mark-and-sweep.
     */

    var val, val1, val2;
    var weak;

    function checkRef() {
        print('ref' in weak, typeof weak.ref);
        print('get:', typeof (Object.getOwnPropertyDescriptor(weak, 'ref') || {}).get);
        print('set:', typeof (Object.getOwnPropertyDescriptor(weak, 'ref') || {}).set);
    }

    print('weak ref in entry part');
    val = {};
    Duktape.fin(val, finalizer);
    weak = { ref: val };
    Duktape.weak(weak);
    print('gc 1'); Duktape.gc();
    val = null;
    print('gc 2'); Duktape.gc();
    checkRef();
    print('gc 3'); Duktape.gc();
    checkRef();

    print('weak ref in getter slot');
    val = function () {};
    Duktape.fin(val, finalizer);
    weak = {};
    Object.defineProperty(weak, 'ref', {
        get: val, enumerable: true, configurable: true
    });
    Duktape.weak(weak);
    print('gc 1'); Duktape.gc();
    val = null;
    print('gc 2'); Duktape.gc();
    checkRef();
    print('gc 3'); Duktape.gc();
    checkRef();

    print('weak ref in setter slot');
    val = function () {};
    Duktape.fin(val, finalizer);
    weak = {};
    Object.defineProperty(weak, 'ref', {
        set: val, enumerable: true, configurable: true
    });
    Duktape.weak(weak);
    print('gc 1'); Duktape.gc();
    val = null;
    print('gc 2'); Duktape.gc();
    checkRef();
    print('gc 3'); Duktape.gc();
    checkRef();

    print('weak ref in array part');
    val = {};
    Duktape.fin(val, finalizer);
    weak = [ val ];
    Duktape.weak(weak);
    print('gc 1'); Duktape.gc();
    val = null;
    print('gc 2'); Duktape.gc();
    print('0' in weak, typeof weak[0]);
    print('gc 3'); Duktape.gc();
    print('0' in weak, typeof weak[0]);

    print('weak refs in both setter and getter slot, removed at the same time');
    val1 = function () {};
    val2 = function () {};
    Duktape.fin(val1, finalizer1);
    Duktape.fin(val2, finalizer2);
    weak = {};
    Object.defineProperty(weak, 'ref', {
        get: val1, set: val2, enumerable: true, configurable: true
    });
    Duktape.weak(weak);
    print('gc 1'); Duktape.gc();
    val1 = null; val2 = null;
    print('gc 2'); Duktape.gc();
    checkRef();
    print('gc 3'); Duktape.gc();
    checkRef();

    print('weak refs in both setter and getter slot, removed one at a time');
    val1 = function () {};
    val2 = function () {};
    Duktape.fin(val1, finalizer1);
    Duktape.fin(val2, finalizer2);
    weak = {};
    Object.defineProperty(weak, 'ref', {
        get: val1, set: val2, enumerable: true, configurable: true
    });
    Duktape.weak(weak);
    print('gc 1'); Duktape.gc();
    val1 = null;
    print('gc 2'); Duktape.gc();
    checkRef();
    print('gc 3'); Duktape.gc();
    checkRef();
    val2 = null;
    print('gc 4'); Duktape.gc();
    checkRef();
    print('gc 5'); Duktape.gc();
    checkRef();
}

try {
    print('weak ref type test');
    weakRefTypeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
finalizer rescue test
gc 1
gc 2
finalizerRescue: rescue on the first time
true object
gc 3
true object
gc 4
finalizerRescue: already rescued, allow free
true object
gc 5
false undefined
===*/

function finalizerRescueTest() {
    /* Test what happens when a finalizer resuces the target of a weak reference,
     * and check that it gets handled correctly when the same object is finalized
     * (this time for good) for the second time.
     */

    function checkRef() {
        print('ref' in weak, typeof weak.ref);
    }

    var obj = {};
    Duktape.fin(obj, finalizerRescue);
    var weak = { ref: obj };
    Duktape.weak(weak);

    print('gc 1'); Duktape.gc();
    obj = null;
    print('gc 2'); Duktape.gc();
    checkRef();
    print('gc 3'); Duktape.gc();
    checkRef();
    rescueTmp = null;
    print('gc 4'); Duktape.gc();
    checkRef();
    print('gc 5'); Duktape.gc();
    checkRef();
}

try {
    print('finalizer rescue test');
    finalizerRescueTest();
} catch (e) {
    print(e.stack || e);
}

/*===
weak object loop test
gc 1
gc 2
finalizer1
finalizer2
true
gc 3
false
===*/

function weakObjectLoopTest() {
    /* Test a scenario where a weak reference points to an object in a
     * reference loop.  Ensure that finalizers are executed for all
     * objects in the loop.
     *
     *    weak --[weakRef]--> obj1 --[ref]--> obj2
     *                         ^               |
     *                         |               |
     *                         `-----[ref]-----'
     */

    var obj1 = {};
    var obj2 = {};
    obj1.ref = obj2;
    obj2.ref = obj1;
    Duktape.fin(obj1, finalizer1);
    Duktape.fin(obj2, finalizer2);

    var weak = Duktape.weak({ weakRef: obj1 });

    print('gc 1'); Duktape.gc();
    obj1 = null; obj2 = null;
    print('gc 2'); Duktape.gc();
    print('weakRef' in weak);
    print('gc 3'); Duktape.gc();
    print('weakRef' in weak);
}

try {
    print('weak object loop test');
    weakObjectLoopTest();
} catch (e) {
    print(e.stack || e);
}

/*===
weak reference chain
gc 1
finalizer1
gc 2
gc 3
finalizer2
weak3.ref: true
gc 4
weak3.ref: false
finalizer3
gc 5
===*/

function weakReferenceChainTest() {
    /* Test scenario where a weak object references another and there's also
     * a weak reference loop.  Ensure weak objects are finalized.
     *
     *    weak1 --[ref]--> weak2 --[ref]--> weak3
     *                       ^                |
     *                       |                |
     *                       `-----[ref]------'
     */

    var weak1 = Duktape.weak({});
    var weak2 = Duktape.weak({});
    var weak3 = Duktape.weak({});
    weak1.ref = weak2;
    weak2.ref = weak3;
    weak3.ref = weak2;
    Duktape.fin(weak1, finalizer1);
    Duktape.fin(weak2, finalizer2);
    Duktape.fin(weak3, finalizer3);

    // Nothing happens yet - all weakN are strongly referenced.
    print('gc 1'); Duktape.gc();

    // Remove reference to weak1, so that it is no longer reachable
    // weakly or strongly.  It gets collected immediately because
    // its refcount drops to zero.
    weak1 = null;
    print('gc 2'); Duktape.gc();

    // Remove reference to weak2, so that it only remains reachable
    // through weak3.  Because the weak3->weak2 reference is weak,
    // weak2 gets finalized here.
    weak2 = null;
    print('gc 3'); Duktape.gc();
    print('weak3.ref:', 'ref' in weak3);  // reference still exists
    print('gc 4'); Duktape.gc();
    print('weak3.ref:', 'ref' in weak3);  // reference no longer exists

    // Remove reference to weak3, so that weak3 gets finalized.  Because
    // weak3's refcount drops to zero, it gets finalized immediately.
    weak3 = null;
    print('gc 5'); Duktape.gc();
}

try {
    print('weak reference chain');
    weakReferenceChainTest();
} catch (e) {
    print(e.stack || e);
}
