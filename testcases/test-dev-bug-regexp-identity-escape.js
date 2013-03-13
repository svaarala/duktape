/*===
object
object $
===*/

/* RegExp identity escapes were broken at one point, caused RegExp SyntaxError. */

try {
    var re = eval("/\\$/");  /* /\$/ should parse as an identity escape, i.e. same as /$/ */
    print(typeof re);
    var m = re.exec('foo$bar');
    print(typeof m, m[0]);
} catch (e) {
    print(e);
}

