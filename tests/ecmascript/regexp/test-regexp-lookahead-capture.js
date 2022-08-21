/*
 *  E5 Section 15.10.2.8 notes on backtracking for lookaheads.
 */

var t;

/*===
 aaa
aba a
===*/

t = /(?=(a+))/.exec('baaabac');
print(t[0], t[1]);

t = /(?=(a+))a*b\1/.exec('baaabac');
print(t[0], t[1]);

/*===
baaabaac ba undefined abaac
===*/

t = /(.*?)a(?!(a+)b\2c)\2(.*)/.exec('baaabaac');
print(t[0], t[1], t[2], t[3]);

/*===
xy x undefined
xz undefined x
===*/

t = /(?=(x))xy|(?=(x))xz/.exec('xy');
print(t[0], t[1], t[2]);

t = /(?=(x))xy|(?=(x))xz/.exec('xz');
print(t[0], t[1], t[2]);

/*===
abab ab
null object
===*/

/* lookahead captures 'ab' */
t = /(?=(ab|abc))\1\1/.exec('abab');
print(t[0], t[1]);

/* lookahead still captures 'ab' and backrefs fail; lookahead never tries to capture 'abc' */
t = /(?=(ab|abc))\1\1/.exec('abcabc');
print(t, typeof t);
