/*
 *  Properties inherited from Object.prototype must not prevent object
 *  literal creation.  This ensures the internal code setting the object
 *  properties doesn't accidentally invoke side effects, etc.
 */

/*===
{"nonConfigurable":123,"nonWritable":234,"nonConfigurableNonWritable":345,"nonConfigurableSetter":456}
===*/

function test() {
    Object.defineProperties(Object.prototype, {
        nonConfigurable: {
            value: 'ancestor',
            writable: true,
            enumerable: false,
            configurable: false
        },
        nonWritable: {
            value: 'ancestor',
            writable: false,
            enumerable: false,
            configurable: true
        },
        nonConfigurableNonWritable: {
            value: 'ancestor',
            writable: false,
            enumerable: false,
            configurable: false
        },
        nonConfigurableSetter: {
            set: function (v) { print('SETTER CALLED!'); },
            enumerable: false,
            configurable: false
        }
    });

    var obj = {
        nonConfigurable: 123,
        nonWritable: 234,
        nonConfigurableNonWritable: 345,
        nonConfigurableSetter: 456
    };
    print(JSON.stringify(obj));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
