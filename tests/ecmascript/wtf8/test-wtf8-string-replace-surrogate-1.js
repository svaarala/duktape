/*
 *  Small test of String.prototype.replace() handling surrogate pairs.
 */

/*@include util-string.js@*/

/*===
"foo<U+D83D><U+DCA9><U+D83D><U+DCA9>bar"
"fOo<U+D83D><U+DCA9><U+D83D><U+DCA9>bar"
"fooZ<U+DCA9><U+D83D><U+DCA9>bar"
"foo<U+D83D>Z<U+D83D><U+DCA9>bar"
"fooZ<U+DCA9>Z<U+DCA9>bar"
"foo<U+D83D>Z<U+D83D>Zbar"
===*/

function test() {
    var inp = 'foo\u{1f4a9}\u{1f4a9}bar';

    safePrintString(inp);
    safePrintString(inp.replace('o', 'O'));
    safePrintString(inp.replace('\ud83d', 'Z'));
    safePrintString(inp.replace('\udca9', 'Z'));
    safePrintString(inp.replace(/\ud83d/g, 'Z'));
    safePrintString(inp.replace(/\udca9/g, 'Z'));
}

test();
