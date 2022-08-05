/*
 *  Dot matching, should not match line terminators; multiline mode etc.
 */

var r, t;

/*===
foo
fii
null
===*/

r = /f../;
t = r.exec('foo'); print(t);
t = r.exec('fii'); print(t);
t = r.exec('fi\n'); print(t);  // no match because line terminator

/* XXX: add more cases */
