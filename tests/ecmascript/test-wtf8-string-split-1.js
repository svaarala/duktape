/*@include util-string.js@*/

/*===
"foo,bar"
"foo,<U+DD8A>bar"
"foo<U+D83E>,bar"
"f,o,o,<U+D83E>,<U+DD8A>,b,a,r"
"foo,bar,quux,,baz,"
"foo,<U+DD8A>bar,<U+DD8A>quux,<U+DD8A>,<U+DD8A>baz,<U+DD8A>"
"foo<U+D83E>,bar<U+D83E>,quux<U+D83E>,<U+D83E>,baz<U+D83E>,"
"f,o,o,<U+D83E>,<U+DD8A>,b,a,r,<U+D83E>,<U+DD8A>,q,u,u,x,<U+D83E>,<U+DD8A>,<U+D83E>,<U+DD8A>,b,a,z,<U+D83E>,<U+DD8A>"
===*/

function test() {
    var fox = '\u{1f98a}';  // U+D83E U+DD8A
    var str = 'foo' + fox + 'bar';

    safePrintString(str.split(fox).join(','));
    safePrintString(str.split('\ud83e').join(','));
    safePrintString(str.split('\udd8a').join(','));
    safePrintString(str.split('').join(','));

    str = 'foo' + fox + 'bar' + fox + 'quux' + fox + fox + 'baz' + fox;
    safePrintString(str.split(fox).join(','));
    safePrintString(str.split('\ud83e').join(','));
    safePrintString(str.split('\udd8a').join(','));
    safePrintString(str.split('').join(','));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
