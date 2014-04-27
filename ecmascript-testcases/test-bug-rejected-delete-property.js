/*
 *  Rejected delete property had a bug which caused the value stack to be out
 *  of balance when returning to the bytecode executor.  This test used to fail
 *  with assertions enabled, when the base object for deletion was a string
 *  (e.g. attempt to delete str.length or a virtual index).
 */

/*===
true
false
false
===*/

function test() {
    // must be non-strict so that delete failure is silent

    var str = 'foo';

    print(delete str.nonexistent);
    print(delete str.length);
    print(delete str[1]);
}

try {
    test();
} catch (e) {
    print(e);
}
