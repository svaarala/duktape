/*
 *  ECMAScript regexp pattern character production does not allow literal
 *  curly braces in any position, but many ECMAScript regexp engines allow
 *  them when the meaning is unambiguous.  Since Duktape 1.5.0 Duktape also
 *  allows literal curly braces in regexps.
 */

// Behavior is custom because e.g. quantifier digit limits are Duktape specific.
/*---
{
    "custom": true
}
---*/

var t;

/*===
a{abc}
a{1b}
a{2,b}
===*/

// Any invalid character cancels quantifier parsing, and causes the left
// curly brace to be treated as a literal (i.e. same as /\{/).

t = /a{.*}/.exec("aa{abc}");
print(t[0]);
t = /a{1.}/.exec("aa{1b}");
print(t[0]);
t = /a{2,.}/.exec("aa{2,b}");
print(t[0]);

/*===
a{abc}
===*/

// Unescaped right (closing) brace is allowed anywhere outside a quantifier
// because it's unambiguous.

t = /a\{.*}/.exec("aa{abc}");
print(t[0]);

/*===
a{1}
a{1,2}
===*/

// Valid quantifier except for the closing brace: quantifier parsing is
// cancelled and left curly brace is treated as a literal.

t = /a{1\}/.exec("aa{1}");
print(t[0]);
t = /a{1,2\}/.exec("aa{1,2}");
print(t[0]);

/*===
{1111111111111111111111111
===*/

// Do not fail on digits before , or }.

t = /{1111111111111111111111111/.exec('{1111111111111111111111111');
print(t[0]);

/*===
a{}
a{,}
a{1,2,3}
===*/

// On any quantifier parsing failure, treat as a literal brace.

t = /a{}/.exec('a{}');
print(t[0]);

t = /a{,}/.exec('a{,}');
print(t[0]);

t = /a{1,2,3}/.exec('a{1,2,3}');
print(t[0]);

/*===
{1111111111111111111111111,}
{1111111111111111111111111,2222222222222222222222222222}
{1111,1111111111}
xxxxxxxxxxx
===*/

// Duktape has an internal limitation on the maximum number of quantifier
// digits: in this case the limits are exceeded and the quantifier is
// rejected and the curly brace is then parsed as a literal.  At the moment
// the maximum number of digits allowed for quantifier min/max value is 9.

t = /{1111111111111111111111111,}/.exec('{1111111111111111111111111,}foo');
print(t[0]);

t = /{1111111111111111111111111,2222222222222222222222222222}/.exec('{1111111111111111111111111,2222222222222222222222222222}');
print(t[0]);

t = /{1111,1111111111}/.exec('{1111,1111111111}foo');
print(t[0]);

// Here the max limit is exactly 9 digits so it's treated as a valid quantifier.
t = /x{11,111111111}/.exec('xxxxxxxxxxx');
print(t[0]);
