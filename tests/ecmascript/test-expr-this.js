/*
 *  'this' keyword (E5 Section 11.1.1).
 */

/*===
Infinity
func: true false
func: false true
===*/

/* at global level 'this' is the global object */
print(this.Infinity);

function test() {
    var obj = {};
    var tmp;
    var global = new Function('return this;')();  // non-strict 'this' is global

    // when calling through an object property, the object is bound
    // to 'this' for the call
    obj.func = function () { print('func: ' + (this === obj) + ' ' + (this === global)); };
    obj.func();

    // when the same function is called without a property lookup,
    // 'this' is undefined but gets coerced to global object in call
    // handling when the target function is not strict
    tmp = obj.func;
    tmp();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
