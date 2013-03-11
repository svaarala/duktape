/*===
TypeError
===*/

/* This should end up in TypeError; the property is non-configurable accessor,
 * and the inbound descriptor is a data descriptor (because of its writable
 * property).  There was a bug causing this to be silently ignored.
 */

try {
    var obj = Object.create(Object.prototype, {
        foo: { enumerable: true, configurable: false, get: function(){}, set: function() {} }
    });

    Object.defineProperty(obj, 'foo', {
        writable: false, enumerable: true, configurable: false, unknown: 'should ignore'
    });

    print('should not succeed');
} catch (e) {
    print(e.name);
}

