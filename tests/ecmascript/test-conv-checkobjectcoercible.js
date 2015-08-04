/*
 *  CheckObjectCoercible() (E5 Section 9.10).
 *
 *  This primitive cannot be directly tested.  It is referenced in the
 *  specification almost exclusively for String.prototype functions, like
 *  charAt().
 *
 *  We use String.prototype.charAt() for indirect testing.
 */

function indirectCheckObjectCoercible(x) {
    String.prototype.charAt.call(x, 0);
}

/*===
TypeError
TypeError
no error
no error
no error
no error
no error
no error
no error
===*/

var values = [ undefined, null, true, false, 123.0, "foo", {}, [], function () {} ];
var i;

for (i = 0; i < values.length; i++) {
    try {
        indirectCheckObjectCoercible(values[i]);
        print('no error');
    } catch (e) {
        print(e.name);
    }
}
