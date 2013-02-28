/*===
3
65
0
0
===*/

/* Undefined arguments are coerced with ToUint16(), yielding zero
 * codepoints.
 */

try {
    var t = String.fromCharCode(65,undefined,undefined);
    print(t.length);
    print(t.charCodeAt(0));
    print(t.charCodeAt(1));
    print(t.charCodeAt(2));
} catch (e) {
    print(e.name);
}

