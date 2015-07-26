/*
 *  Property access as a left-hand side expression (E5 Sections 11.2.1, 11.13,
 *  11.3.1, 11.3.2, 11.4.4, 11.4.5).
 */

/*===
1
2
===*/

x = {};

x.foo = 1;
print(x.foo);

x['bar'] = 2;
print(x.bar);

/* FIXME: more tests */
