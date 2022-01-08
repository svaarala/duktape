/*===
14
14
15
19
3
19
0
===*/

function test() {
    var fox = '\u{1f98a}';  // U+D83E U+DD8A
    var str = 'foo' + fox + 'bar' + fox + 'quux' + fox + 'baz';

    print(str.lastIndexOf(fox));
    print(str.lastIndexOf('\ud83e'));
    print(str.lastIndexOf('\udd8a'));
    print(str.lastIndexOf(''));
    print(str.lastIndexOf('', 3));
    print(str.lastIndexOf('', 999));
    print(str.lastIndexOf('', -999));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
