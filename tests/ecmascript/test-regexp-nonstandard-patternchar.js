/*
 *  V8 and Rhino both allow an unescaped brace to appear literally.
 *  Technically this seems to be incorrect because Ecmascript RegExp
 *  syntax does not allow reserved characters like '{' to appear as
 *  PatternCharacters.
 */

/*---
{
    "nonstandard": true
}
---*/

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
