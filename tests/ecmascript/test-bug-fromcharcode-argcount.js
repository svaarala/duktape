/*===
300
ABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJ
===*/

/* This currently fails due to Duktape register count limits; the failure is
 * not specific to String.fromCharCode().  However, String.fromCharCode() is
 * an example of a function which could conceivably be used with a lot of
 * arguments, to build a large Unicode string.
 */

try {
    var t;

    t = String.fromCharCode(
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74);  // 300 args

    print(t.length);
    print(t);
} catch (e) {
    print(e);
}
