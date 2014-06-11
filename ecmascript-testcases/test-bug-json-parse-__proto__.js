/*
 *  Bug in test262 test case: ch15/15.12/15.12.2/S15.12.2_A1
 *
 *  Duktape does not parse __proto__ as a concrete property when ES6
 *  __proto__ property is enabled.
 */

/*===
[object Object]
object
true
false
true
[object Object]
===*/

function test() {
    var x = JSON.parse('{"__proto__":[]}');
    print(x);
    print(typeof x);
    print(x instanceof Object);
    print(x instanceof Array);
    print('__proto__' in x);
    print(Object.getOwnPropertyDescriptor(x, '__proto__'));
}

try {
    test();
} catch (e) {
    print(e);
}
