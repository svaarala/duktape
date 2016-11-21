/*
 *  Demonstrate a bug in Object.defineProperty() accessor-to-data-property
 *  convertion when a previous value has a finalizer which modifies the object
 *  which is going through Object.defineProperty().
 *
 *  See comments in test-bug-object-delprop-eidx-1.js.
 *
 *  https://github.com/svaarala/duktape/pull/1096
 */

/*===
Object.defineProperty() to convert .prop to a data property
finalizer, modify object
finalizer done
Object.defineProperty() done
{prop:"replaced",oops:345,"dummy-0":"foo","dummy-1":"foo","dummy-2":"foo","dummy-3":"foo","dummy-4":"foo","dummy-5":"foo","dummy-6":"foo","dummy-7":"foo","dummy-8":"foo","dummy-9":"foo"}
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

    Object.defineProperty(obj, 'prop', { get: new Function(''), configurable: true, enumerable: true });
    Object.getOwnPropertyDescriptor(obj, 'prop').get.prototype = null;  // break circular ref
    Duktape.fin(Object.getOwnPropertyDescriptor(obj, 'prop').get, myFinalizer);

    obj.oops = 345;

    print('Object.defineProperty() to convert .prop to a data property');
    Object.defineProperty(obj, 'prop', { value: 'replaced' });
    print('Object.defineProperty() done');

    print(Duktape.enc('jx', obj));
}

try {
    accessorToDataTest();
} catch (e) {
    print(e.stack || e);
}
