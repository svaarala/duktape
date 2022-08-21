/*
 *  Debugger statement (E5 Section 12.15).
 */

/*===
before
after
===*/

/* If a debugger is not present or active, this statement
 * should have no observable effect.
 */

print("before");
debugger;
print("after");
