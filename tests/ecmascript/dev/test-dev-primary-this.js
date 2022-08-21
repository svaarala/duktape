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

// global object
print(this.Number.POSITIVE_INFINITY);

// global object
printThisProperty('is_global');

// forced binding
printThisProperty.call(this_obj, 'foo');
