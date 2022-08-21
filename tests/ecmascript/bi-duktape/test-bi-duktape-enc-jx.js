/*
 *  Duktape.enc() JX
 */

/*---
custom: true
---*/

/*===
[undefined,null,undefined,undefined,undefined,1,undefined]
===*/

function test() {
    // Array gaps: 'undefined'.
    print(Duktape.enc('jx', [ void 0, null,,,, 1,, ]));
}

test();
