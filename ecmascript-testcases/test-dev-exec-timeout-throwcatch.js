/*
 *  Execution timeout: throw/catch
 */

/*---
{
    "skip": true
}
---*/

function test() {
    function thrower() { throw new Error('aiee'); }

    for (;;) {
        try {
            thrower();
        } catch (e) {
            ;
        }
    }
}

try {
    test();
} catch (e) {
    print(e.name);
}
