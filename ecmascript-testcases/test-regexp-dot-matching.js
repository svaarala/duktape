/*
 *  Dot matching, should not match line terminators; multiline mode etc.
 */

/*---
{
    "skip": true
}
---*/

/*FIXME*/

var r, t;

/*===
foo
fii
null
===*/

try {
    r = /f../;
    t = r.exec('foo'); print(t);
    t = r.exec('fii'); print(t);
    t = r.exec('fi\n'); print(t);  // no match because line terminator
} catch (e) {
    print(e.name);
}
