/*===
false
false
true
===*/

try {
    print('' === 'foo');
    print('' > 'foo');
    print('' < 'foo');
} catch (e) {
    print(e);
}
