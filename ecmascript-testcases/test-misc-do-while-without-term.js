/*
 *  There is a bug filed at:
 *
 *    https://bugs.ecmascript.org/show_bug.cgi?id=8
 *
 *  The bug is about the expression:
 *
 *    do{;}while(false)false
 *
 *  which is prohibited in the specification but allowed in actual
 *  implementations.  The syntax error is that a do statement is
 *  supposed to be followed by a semicolon and since there is no
 *  newline following the right parenthesis, an automatic semicolon
 *  should not be allowed.
 */

/*===
OK
===*/

try {
    eval("do{;}while(false)false");
    print("OK");
} catch (e) {
    print(e.name);
}
