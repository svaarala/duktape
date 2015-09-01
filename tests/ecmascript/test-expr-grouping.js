/*
 *  Grouping operator (E5 Section 11.1.6).
 */

/*===
1
1
3
3
3
11
21
===*/

/* Parenthesis around an expression. */

print(1);
print((1));
print(1+2);
print((1)+(2));
print((1+2));
print(1+2*3+4);
print((1+2)*(3+4));

/*===
number 123
undefined
===*/

/* Parenthesis are allowed even for expression like 'delete (x)', mentioned
 * explicitly in E5 Section 11.1.6.
 */

this.foo = 123;
print(typeof (foo), foo);
delete (foo);
print(typeof (foo));
