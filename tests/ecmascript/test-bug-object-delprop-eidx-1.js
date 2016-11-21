/*
 *  Demonstrate bug in internal delprop helper when previous value has a
 *  finalizer which modifies the same object which is going through a delprop.
 *  https://github.com/svaarala/duktape/pull/1096
 */

/*===
deleting obj.prop
finalizer, modify object
finalizer done
deleted obj.prop
{oops:345,"dummy-0":"foo","dummy-1":"foo","dummy-2":"foo","dummy-3":"foo","dummy-4":"foo","dummy-5":"foo","dummy-6":"foo","dummy-7":"foo","dummy-8":"foo","dummy-9":"foo"}
===*/

var obj = {};

function myFinalizer() {
    var i;

    print('finalizer, modify object');

    // To cause the runtime e_idx to change, delete the 'first' property which
    // leaves an entry part gap.  Then insert enough properties to cause a
    // property table resize, so that 'prop' moves into the gap.  Delprop
    // continues updating the slot before this move, i.e. the property
    // which follows 'prop'.

    delete obj.first;
    for (i = 0; i < 10; i++) {
        // i limit 2 is enough to trigger the bug now, overshoot a bit.
        obj['dummy-' + i] = 'foo';
    }

    print('finalizer done');
}

function test() {
    obj.first = 123;  // gets deleted
    obj.prop = {};    // gets deleted
    obj.oops = 345;
    Duktape.fin(obj.prop, myFinalizer);

    print('deleting obj.prop');
    delete obj.prop;
    print('deleted obj.prop');

    // When the bug triggers, 'oops' get deleted while 'prop' remains,
    // but 'prop' value is undefined.  The delprop update has updated
    // two unrelated slots due to the finalizer activity.
    print(Duktape.enc('jx', obj));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
