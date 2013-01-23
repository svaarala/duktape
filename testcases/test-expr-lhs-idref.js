/*
 *  Identifier reference as a left-hand side expression (E5 Sections 11.2, 11.13,
 *  11.3.1, 11.3.2, 11.4.4, 11.4.5).
 */

/*===
1
3
===*/

x = 1;
print(x);

x = 2;
(((x))) = 3;
print(x);

/* FIXME: inside function */
/* FIXME: more tests */
