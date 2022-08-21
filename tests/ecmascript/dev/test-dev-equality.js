/*
 *  Basic equality operator tests
 */

/* XXX: fine grained coercion checks */

/*===
false true false
true false true
false true false
true false true
===*/

print(1 == 2, 2 == 2, 3 == 2);
print(1 != 2, 2 != 2, 3 != 2);
print(1 === 2, 2 === 2, 3 === 2);
print(1 !== 2, 2 !== 2, 3 !== 2);

/*===
true
false
false
true
===*/

print(123 == '123');
print(123 === '123');

print(123 != '123');
print(123 !== '123');
