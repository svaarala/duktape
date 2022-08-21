/*
 *  Execution timeout: yield/resume
 */

/*---
custom: true
skip: true
---*/

function test() {
    function mythread() { return 123; }

    for (;;) {
        var t = new Duktape.Thread(mythread);
        Duktape.Thread.resume(t);
    }
}

try {
    test();
} catch (e) {
    print(e.name);
}
