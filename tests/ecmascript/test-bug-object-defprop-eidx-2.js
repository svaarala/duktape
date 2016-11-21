/*
 *  Demonstrate a bug in Object.defineProperty() data-to-accessor-property
 *  convertion when a previous value has a finalizer which modifies the object
 *  which is going through Object.defineProperty().
 *
 *  For some reason this test doesn't trigger the bug in practice.
 *
 *  See comments in test-bug-object-delprop-eidx-1.js.
 *
 *  https://github.com/svaarala/duktape/pull/1096
 */

/*===
Object.defineProperty() to convert .prop to an accessor property
finalizer, modify object
finalizer done
Object.defineProperty() done
{prop:"getter",oops:"keep","dummy-0":"foo","dummy-1":"foo","dummy-2":"foo","dummy-3":"foo","dummy-4":"foo","dummy-5":"foo","dummy-6":"foo","dummy-7":"foo","dummy-8":"foo","dummy-9":"foo"}
===*/

var obj = {};

function myFinalizer() {
    var i;

    print('finalizer, modify object');

    delete obj.first;
    for (i = 0; i < 10; i++) {
        // i limit 2 is enough to trigger the bug now, overshoot a bit.
        obj['dummy-' + i] = 'foo';
    }

    print('finalizer done');
}

function accessorToDataTest() {
    obj.first = 123;

    Object.defineProperty(obj, 'prop', { value: {}, configurable: true, enumerable: true });
    Duktape.fin(Object.getOwnPropertyDescriptor(obj, 'prop').value, myFinalizer);

    obj.oops = 'keep';

    print('Object.defineProperty() to convert .prop to an accessor property');
    Object.defineProperty(obj, 'prop', { get: new Function('return "getter";'), set: new Function() });
    print('Object.defineProperty() done');

    print(Duktape.enc('jx', obj));
}

try {
    accessorToDataTest();
} catch (e) {
    print(e.stack || e);
}
