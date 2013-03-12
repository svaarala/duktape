
/*---
{
    "custom": true
}
---*/

/*===
1
0
-1
===*/

/* Test for a bug: strncmp() was used for string comparison, which is
 * a problem because strncmp() stops comparing at a NUL.
 */

try {
    print('foo\u0000f'.localeCompare('foo\u0000e'));
    print('foo\u0000f'.localeCompare('foo\u0000f'));
    print('foo\u0000f'.localeCompare('foo\u0000g'));
} catch (e) {
    print(e);
}

