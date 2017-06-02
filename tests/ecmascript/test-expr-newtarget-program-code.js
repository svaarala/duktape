/*
 *  new.target not allowed in program code; this fails without print()
 *  calls due to script SyntaxError.
 */

/*---
{
    "intended_uncaught": true
}
---*/

/*===
===*/

print('start');
print(typeof new.target);
print('done');
