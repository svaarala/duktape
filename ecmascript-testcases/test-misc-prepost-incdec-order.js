/*
 *  Evaluation order of pre/post increment and decrement.
 */

/*===
1 1 3 3 3 1
===*/

x = 1;
print(x, x++, ++x, x, x--, --x);
