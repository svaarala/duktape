/* XXX: for eval/global code */
/* XXX: test for handling of 'valued' and 'non-valued' statements */

/*===
3
3
===*/

/* basic case */
print(eval('1+2;'));

/* an empty statement (like debugger) must not alter non-empty previous result */
print(eval('1+2; debugger;'));
