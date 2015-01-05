/*
 *  Testcase for RegExp leniency in GitHub issue #39.
 *
 *  Duktape 0.12.0 (and probably 1.0) doesn't pass this test, because the
 *  RegExp parser has no leniency for regexps outside the E5 syntax.
 */

/*===
parse ok
===*/

/* Here the '\1' escape inside a character class is invalid in E5:
 * E5 Section 15.10.2.11 step 3, then E5 Section 15.10.2.19 step 2.
 * V8 accepts this.
 *
 * Not sure what the semantics are: '\1' might be interpreted as a
 * backreference.  It might also be interpreted as a literal '1'
 * (this doesn't seem to be the case when testing manually).
 */
function test() {
    var r = /(\(|\s|\[|\=|:|\+|\.)(('|")([^\\\1]|\\.)*?(\3))/gm;
    print('parse ok');
}

try {
    test();
} catch (e) {
    print(e);
}
