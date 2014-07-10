/*
 *  V8 and Rhino both allow an unescaped brace to appear literally.
 *  Technically this seems to be incorrect because Ecmascript RegExp
 *  syntax does not allow reserved characters like '{' to appear as
 *  PatternCharacters.
 */

/*---
{
    "nonstandard": true,
    "knownissue": "other engines allow an unescaped brace to appear literally (e.g. /{/), Duktape does not (which seems correct but is against real world behavior)"
}
---*/

/* FIXME: this now tests V8/Rhino behavior, is this desired? */

/*===
4
===*/

try {
    var re = eval("/{/");
    var res = re.exec('foo {');
    print(res.index);
} catch (e) {
    print(e);
}

