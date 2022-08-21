/*@include util-string.js@*/

/*===
"fooX<U+DCA9>bar"
"foo<U+D83D>Xbar"
"fooX<U+DCA9>bar"
"foo<U+D83D>Xbar"
"fooXbar"
===*/

// WTF-8 non-BMP codepoints must behave like surrogate pairs in .replace().
function test() {
    var s = 'foo\u{1f4a9}bar';  // U+1F4A9 surrogate pairs: U+D83D U+DCA9
    var t;

    t = s.replace('\ud83d', 'X');
    safePrintString(t);

    t = s.replace('\udca9', 'X');
    safePrintString(t);

    t = s.replace(/[\ud800-\udbff]/, 'X');
    safePrintString(t);

    t = s.replace(/[\udc00-\udfff]/, 'X');
    safePrintString(t);

    t = s.replace(/[\ud800-\udfff]+/, 'X');
    safePrintString(t);
}

test();
