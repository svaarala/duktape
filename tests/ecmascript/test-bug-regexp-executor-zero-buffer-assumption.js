/*
 *  Demonstrate RegExp executor bug in Duktape 1.5.1 when DUK_USE_ZERO_BUFFER_DATA
 *  is unset.  The internal saved pointers are not zeroed which causes memory unsafe
 *  behavior when unused captures are processed on a match.
 *
 *  https://github.com/svaarala/duktape/pull/978
 */

/*===
x,,
===*/

var re = /x|(y)|(z)/;
var res = re.exec('x');
print(res);
