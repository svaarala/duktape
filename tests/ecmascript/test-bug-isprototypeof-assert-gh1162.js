/*
 *  Assert failure in Duktape 1.5.1
 *
 *  https://github.com/svaarala/duktape/issues/1162
 */

/*===
false
===*/

try {
    print(this.parseFloat.isPrototypeOf(this.Object.prototype));
} catch (e) {
    print(e.stack || e);
}
