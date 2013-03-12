/* Empty strings must be treated as equal.  Because there is a special
 * code path for these, test as a special case.
 */

/*===
false
true
false
true
true
false
true
false
===*/

try {
    print('' < '');
    print('' <= '');
    print('' > '');
    print('' >= '');
    print('' == '');
    print('' != '');
    print('' === '');
    print('' !== '');
} catch (e) {
    print(e);
}

