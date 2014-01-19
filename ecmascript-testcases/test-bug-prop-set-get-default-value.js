/*
 *  If a 'set' or a 'get' is not given when setting an accessor, the missing
 *  property should default to undefined.  When reading back the descriptor,
 *  there should always be a 'set' and a 'get' key for an accessor, with the
 *  value undefined if the function has not been set.
 *
 *  - E5.1 Section 8.6.1, Table 6: [[Get]] and [[Set]] values are either
 *    (function) Object or undefined
 *
 *  - E5.1 Section 8.12.9, step 4.b.i: [...] If the value of an attribute field
 *    of Desc is absent, the attribute of the newly created property is set to
 *    its default value.
 */

/*===
set true function
get true undefined
enumerable true boolean false
configurable true boolean false
set true undefined
get true function
enumerable true boolean false
configurable true boolean false
===*/

function test() {
    var obj = {};
    var pd;

    Object.defineProperty(obj, 'prop1', {
        set: function() { print('prop1 set'); }
    });

    Object.defineProperty(obj, 'prop2', {
        get: function() { print('prop2 get'); }
    });

    function printDesc(prop) {
        pd = Object.getOwnPropertyDescriptor(obj, prop);
        print('set', 'set' in pd, typeof pd.set);
        print('get', 'get' in pd, typeof pd.get);
        print('enumerable', 'enumerable' in pd, typeof pd.enumerable, pd.enumerable);
        print('configurable', 'configurable' in pd, typeof pd.configurable, pd.configurable);
    }

    printDesc('prop1');
    printDesc('prop2');
}

try {
    test();
} catch (e) {
    print(e);
}
