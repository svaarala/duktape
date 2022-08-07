/*
 *  PrimaryExpression
 */

/* XXX: add more tests; comprehensive tests in separate files for
 * e.g. regexps and number literals
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

/*===
===*/

/* Identifier */

// XXX

/*===
null
true
false
123
65
33
63
88
99
===*/

/* Literal:
 *   -> NullLiteral    -> 'null'
 *   -> BooleanLiteral -> 'true' | 'false'
 *   -> NumericLiteral -> DecimalLiteral | HexIntegerLiteral | OctalIntegerLiteral (compatibility)
 *   -> StringLiteral  -> '"' chars '"' | "'" chars "'"
 *   -> RegularExpressionLiteral
 *
 * OctalLiteral evaluation is inside an eval() call because it is optional.
 */

print(null);
print(true);
print(false);
print(123);
print(0x41);

// = 4*8+1 = 33
print(eval("041"));

// 077 = 7*8 + 7 = 63
print(eval("077"));

// 088 is an invalid NumericLiteral in ES5; ES2015 allows it to be parsed
// as decimal and both V8 and Spidermonkey do so.
print(eval("088"));

// 099 is similar to 088
print(eval("099"));

/*===
1,2,3
===*/

/* ArrayLiteral */

var arr;

arr = [1,2,3];
print(arr);


/*===
1 2
===*/

/* ObjectLiteral */

var obj;

obj = { foo:1, bar:2 };
print(obj.foo, obj.bar);

/*===
===*/

/* Parenthesized expression */
