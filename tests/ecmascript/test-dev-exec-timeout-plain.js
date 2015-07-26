/*
 *  Execution timeout: plain bytecode execution
 */

/*---
{
    "skip": true
}
---*/

function test() {
    for (;;) ;
}

try {
    test();
} catch (e) {
    print(e.name);
}
