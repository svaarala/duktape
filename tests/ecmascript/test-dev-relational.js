/*
 *  Very basic relational tests
 */

/* XXX: add associativity tests */

/*===
true false false
true true false
false false true
false true true
===*/

print(1 < 2, 2 < 2, 3 < 2);
print(1 <= 2, 2 <= 2, 3 <= 2);
print(1 > 2, 2 > 2, 3 > 2);
print(1 >= 2, 2 >= 2, 3 >= 2);

/*===
true
true
false
===*/

var obj = {foo:1, bar:2};

print('foo' in obj);
print('bar' in obj);
print('quux' in obj);

/*===
true
false
false
===*/

var num = new Number(123);

print(num instanceof Number);
print(123 instanceof Number);
print('foo' instanceof Number);
