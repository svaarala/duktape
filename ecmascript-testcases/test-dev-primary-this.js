/*
 *  PrimaryExpression -> 'this'
 */

function printThisProperty(propName) {
    print(this[propName]);
}

/*===
Infinity
true
bar
===*/

/* 'this' */

var this_obj = { 'foo': 'bar' };

this.is_global = true;

try {
    // global object
    print(this.Number.POSITIVE_INFINITY);
} catch (e) {
    print(e.name);
}

try {
    // global object
    printThisProperty('is_global');
} catch (e) {
    print(e.name);
}

try {
    // forced binding
    printThisProperty.call(this_obj, 'foo');
} catch (e) {
    print(e.name);
}
