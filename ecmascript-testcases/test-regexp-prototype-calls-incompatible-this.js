/*
 *  E5 Section 15.10.6 requires a TypeError if RegExp.prototype methods
 *  are called with a 'this' value which is not a RegExp.
 */

/*===
TypeError
TypeError
TypeError
===*/

var funcs = [
    RegExp.prototype.exec,
    RegExp.prototype.test,
    RegExp.prototype.toString,
];

for (var i = 0; i < funcs.length; i++) {
    var f = funcs[i];
    try {
        f('foo');
        print('no error');
    } catch (e) {
        print(e.name);
    }
}
