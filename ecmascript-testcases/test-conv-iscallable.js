/*
 *  IsCallable() (E5 Section 9.11).
 *
 *  IsCallable() cannot be directly tested, but Function.prototype.bind()
 *  provides a very easy primitive for testing it.  E5 Section 15.3.4.5, step
 *  2 will throw a TypeError if IsCallable(func) is false.
 */

function indirectIsCallable(x) {
    Function.prototype.bind.call(x);
}

/*===
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
no error
no error
no error
===*/

var values = [ undefined, null, true, false, 123.0, "foo",
               {}, [], function () {}, Object.prototype.toLocaleString,
               Object.prototype.toLocaleString.bind('foo') ];
var i;

for (i = 0; i < values.length; i++) {
    try {
        indirectIsCallable(values[i]);
        print('no error');
    } catch (e) {
        print(e.name);
    }
}
