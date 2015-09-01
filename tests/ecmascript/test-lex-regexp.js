/*
 *  Parsing of regexp literals (E5 Section 7.8.5).
 */

/* There are detailed specific RegExp tests, so this is just a really
 * simple first pass.
 */

/* XXX: a few more useful tests here */
/* XXX: division vs. regexp ambiguity tests here */

/*===
foofoobar,foo
===*/

var t = /(.{3})\1bar/.exec('foofoobar');
print(t);
