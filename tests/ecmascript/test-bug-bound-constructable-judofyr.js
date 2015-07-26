/*
 *  Bug reported by http://www.reddit.com/user/judofyr:
 *
 *  This raises an error (because the bound function is not constructable).
 *  According to MDC this should work:
 *
 *    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function/bind#Bound_functions_used_as_constructors
 *
 *  (and I believe there's code out there that depends on it).
 */

/*===
object
1
===*/

function Thing(value) {
    print(typeof this);
    this.value = value;
}

function test() {
    one = Thing.bind(null, 1);
    var obj = new one;
    print(obj.value);
}

try {
    test();
} catch (e) {
    print(e);
}
