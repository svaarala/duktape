/*===
false
false
true
false
true
false
===*/

/* Testcase for a bug in comparing strings with embedded NULs.
 * strncmp(), which stops comparing at NULs, was used.
 */

try {
    print('foo\u0000f' < 'foo\u0000e');
    print('foo\u0000f' < 'foo\u0000f');
    print('foo\u0000f' < 'foo\u0000g');

    print('' < '');
    print('' < '\u0000');
    print('\u0000' < '');
} catch (e) {
    print(e);
}
