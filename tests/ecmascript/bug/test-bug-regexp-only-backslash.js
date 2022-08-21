/*
 *  RegExp created with a single backslash should cause a SyntaxError.
 *  For some reason it was parsed as a legit regexp at some point.
 */

/*===
SyntaxError
===*/

try {
    var re = new RegExp("\\");  // regexp with a single backslash
    print('never here');
} catch (e) {
    print(e.name);
}
