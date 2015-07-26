/*
 *  Compilation and execution of eval code (E5 Sections 13, 10.4.2).
 */

/*FIXME*/
/*FIXME: this binding, variable environment, variable declarations through eval, etc*/

/*===
foo
===*/

print(eval("'foo'"));

/*===
123
===*/

/* var is an 'empty' statement, so 123 remains as the return value */

print(eval("123; var t=10;"));
