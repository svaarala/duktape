/*
 *  Function constructor argument may end with a single line comment.
 *  See https://github.com/svaarala/duktape/issues/1754.
 */

/*===
foo
===*/

try {
    print(new Function('return "foo" //lastline')());
} catch (e) {
    print(e.stack || e);
}
