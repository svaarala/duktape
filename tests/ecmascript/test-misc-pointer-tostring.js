/*---
{
    "custom": true
}
---*/

/*===
null
true
===*/

/* Test pointer formatting.  NULL pointers are formatted consistenly and in
 * a platform independent manner.  Non-NULL pointers don't have a fixed format
 * but test for an expected subset.
 */

function pointerTostringTest() {
    var ptr_null = Duktape.Pointer();
    var ptr_nonnull = Duktape.Pointer('dummy');
    var re_ptr_nonnull = /^[0-9a-fA-Fx:]+$/;

    print(ptr_null.toString());
    print(re_ptr_nonnull.test(ptr_nonnull.toString()));
}

try {
    pointerTostringTest();
} catch (e) {
    print(e);
}
