/*===
setter called
RangeError: aiee
["foo","bar","quux"]
TypeError
["foo","bar","quux"]
done
===*/

function test() {
    'use strict';

    var A = [ 'foo', 'bar', 'quux' ];
    Object.defineProperty(Object.prototype, '3', {
        set: function () {
            print('setter called');
            throw new RangeError('aiee');
        }
    });
    Object.preventExtensions(A);

    // Even though 'A' is not extensible and a final write will fail, we
    // must still proceed to parent and hit the setter.
    try {
        A[3] = 123;
    } catch (e) {
        print(String(e));
    }
    print(JSON.stringify(A));

    // When not hitting a setter, not extensible triggers TypeError.
    try {
        A[4] = 123;
    } catch (e) {
        print(e.name);
    }
    print(JSON.stringify(A));

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
