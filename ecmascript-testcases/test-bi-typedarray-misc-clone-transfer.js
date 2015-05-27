/*
 *  Cloning and transfers (Section 9 of Khronos TypedArray)
 */

/*---
{
    "custom": true,
    "skip": true
}
---*/

/*===
clone and transfer test
===*/

function cloneAndTransferTest() {
    // Section 9 of the Khronos specification provides some guidance for cloning
    // and transfering, but it doesn't specify anything that we can test now.
}

try {
    print('clone and transfer test');
    cloneAndTransferTest();
} catch (e) {
    print(e.stack || e);
}
