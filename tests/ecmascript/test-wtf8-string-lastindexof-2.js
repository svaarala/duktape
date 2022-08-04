/*===
7
7
7
-1
===*/

function test() {
    var x = 'foobar\u{cafe}foo\u{1f98a}';
    print(x.lastIndexOf('foo'));
    print(x.lastIndexOf('foo\u{1f98a}'));
    print(x.lastIndexOf('foo\u{d83e}'));  // High surrogate of U+1F98A
    print(x.lastIndexOf('foo\u{d83f}'));
}

test();
