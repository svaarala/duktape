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
true
true
false
false
false
true
false
true
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

    print('' < 'foo');
    print('' <= 'foo');
    print('' > 'foo');
    print('' >= 'foo');
    print('' == 'foo');
    print('' != 'foo');
    print('' === 'foo');
    print('' !== 'foo');
} catch (e) {
    print(e);
}
