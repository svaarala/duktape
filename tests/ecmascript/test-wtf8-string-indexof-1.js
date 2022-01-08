/*===
3
3
4
0
3
19
0
===*/

function test() {
    var fox = '\u{1f98a}';  // U+D83E U+DD8A
    var str = 'foo' + fox + 'bar' + fox + 'quux' + fox + 'baz';

    print(str.indexOf(fox));
    print(str.indexOf('\ud83e'));
    print(str.indexOf('\udd8a'));
    print(str.indexOf(''));
    print(str.indexOf('', 3));
    print(str.indexOf('', 999));
    print(str.indexOf('', -999));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
