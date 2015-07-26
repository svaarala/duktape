/*
 *  Print a sum of pre/post increment and decrement operations on
 *  the same value.  Ensures that temporary results are not incorrectly
 *  bound to the variable register (each value needs a temporary register
 *  because it may be further modified).
 */

/*===
19
===*/

x = 1;

print(
(++x) /* 2 */
+
(x++) /* 2 */
+
x     /* 3 */
+
(++x) /* 4 */
+
(--x) /* 3 */
+
(x--) /* 3 */
+
x     /* 2 */

      /* = 19 */
);
