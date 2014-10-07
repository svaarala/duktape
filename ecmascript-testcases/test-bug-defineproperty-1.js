/*===
TypeError
===*/

/* This should end up in TypeError; the property is non-configurable accessor,
 * and the inbound descriptor is a data descriptor (because of its writable
 * property).  There was a bug causing this to be silently ignored.
 *
 * The bug happened because the "writable" flag of the current property is
 * "not defined" initially (accessor property) but is treated like "false"
 * internally.  This causes the algorithm to determine that since writable
 * is being changed from false to false, and enumerable/configurable are
 * similarly not being changed, there is nothing to check.  The attempt to
 * change the property from accessor to data property is thus not caught.
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
