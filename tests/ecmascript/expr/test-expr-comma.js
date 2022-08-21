/*
 *  Comma operator (E5 Section 11.14).
 */

/*===
345
123
123 234 345
345
foo
bar
123
===*/

var t, s;

// Note: this is parsed as: (t = 123),234,345, so 't' gets 123
// while the comma expression value is 345
s = (t = 123,234,345);
print(s);
print(t);

print(123,234,345);   // in call expression comma is interpreted as arg separator

print((123,234,345)); // in parens it becomes a comma operator

print((print('foo'), print('bar'), 123));   // eval order test
