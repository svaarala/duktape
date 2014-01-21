/*
 *  For-in statement (E5 Section 12.6.4).
 */

/*---
{
    "knownissue": true
}
---*/

/*===
SyntaxError
===*/

try {
    /* 'a+b' is not a valid LeftHandSideExpression -> SyntaxError required */
    eval("for (a+b in [0,1]) {}");
    print("never here");
} catch (e) {
    print(e.name);
}

/* FIXME: other tests */

/*FIXME:break*/
/*FIXME:continue*/

