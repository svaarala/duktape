/*===
true
true
false
true
false
true
===*/

function test() {
    var fox = '\u{1f98a}';  // U+D83E U+DD8A
    var str = fox + 'foobar' + fox;

    print(str.startsWith(fox));
    print(str.startsWith('\ud83e'));
    print(str.startsWith('\udd8a'));
    print(str.endsWith(fox));
    print(str.endsWith('\ud83e'));
    print(str.endsWith('\udd8a'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
