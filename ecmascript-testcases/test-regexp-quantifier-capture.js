/*
 *  Capture for repeated group.
 */

/*===
xyxzxw xw
===*/

var t;

/* the last group repetition gets the captured value */
t = /(x.)+/.exec("axyxzxwa");
print(t[0], t[1]);

/*===
null object
aabbcc cc c
===*/

/* first backref matches 'a', second matches 'b', causing a match failure */
t = /((.)\2){3}/.exec('aabacc')
print(t, typeof t);

/* each backref matches different char, last 'c' retained as capture */
t = /((.)\2){3}/.exec('aabbcc')
print(t[0], t[1], t[2]);
