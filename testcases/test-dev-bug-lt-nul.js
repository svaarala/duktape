
/*===
false
false
true
===*/

/* Testcase for a bug in comparing strings with embedded NULs.
 * strncmp(), which stops comparing at NULs, was used.
 */

try {
    print('foo\u0000f' < 'foo\u0000e');
    print('foo\u0000f' < 'foo\u0000f');
    print('foo\u0000f' < 'foo\u0000g');
} catch (e) {
    print(e);
}

