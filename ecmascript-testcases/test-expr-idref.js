/*
 *  Identifier reference (E5 Section 11.1.2).
 */

/*===
Infinity
var 1
var 2
===*/

/* global identifier */
print(Infinity);

/* var1 defined into global object ("with"-like object binding) */
var1 = "var 1";
print(var1);

/* var2 defined explicitly into global object */
var var2 = "var 2";
print(var2);

/* FIXME */
