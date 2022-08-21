/*
 *  Tests corner cases for \b and \B at start and end of input.
 *
 *  IsWordChar() is defined in E5 Section 15.10.2.6 to be false if
 *  evaluated for a character outside the input string.  The \b and
 *  \B assertions may thus match even at the start or the end of
 *  the input string.
 *
 *  Below the '=' character is used as a non-word-character.
 */

var t;

/*===
null
a=
=a
null
ab
null
null
==
===*/

/* normal cases */

t = /.\b./.exec('ab');   /* IsWordChar: true -> true */
print(t);

t = /.\b./.exec('a=');   /* IsWordChar: true -> false */
print(t[0]);

t = /.\b./.exec('=a');   /* IsWordChar: false -> true */
print(t[0]);

t = /.\b./.exec('==');   /* IsWordChar: false -> false */
print(t);

t = /.\B./.exec('ab');   /* IsWordChar: true -> true */
print(t[0]);

t = /.\B./.exec('a=');   /* IsWordChar: true -> false */
print(t);

t = /.\B./.exec('=a');   /* IsWordChar: false -> true */
print(t);

t = /.\B./.exec('==');   /* IsWordChar: false -> false */
print(t[0]);

/*===
a
null
null
=
===*/

/* \b or \B at start of input */

t = /\b./.exec('a');   /* IsWordChar: false -> true */
print(t[0]);

t = /\b./.exec('=');   /* IsWordChar: false -> false */
print(t);

t = /\B./.exec('a');   /* IsWordChar: false -> true */
print(t);

t = /\B./.exec('=');   /* IsWordChar: false -> false */
print(t[0]);

/*===
a
null
null
=
===*/

/* \b or \B at end of input */

t = /.\b/.exec('a');   /* IsWordChar: true -> false */
print(t[0]);

t = /.\b/.exec('=');   /* IsWordChar: false -> false */
print(t);

t = /.\B/.exec('a');   /* IsWordChar: true -> false */
print(t);

t = /.\B/.exec('=');   /* IsWordChar: false -> false */
print(t[0]);

/*===
null

===*/

/* \b or \B for empty string */

t = /\b/.exec('');   /* IsWordChar: false -> false */
print(t);

t = /\B/.exec('');   /* IsWordChar: false -> false */
print(t[0]);
