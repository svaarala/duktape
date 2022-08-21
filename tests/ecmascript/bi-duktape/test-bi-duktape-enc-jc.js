/*
 *  Duktape.enc() JC
 */

/*---
custom: true
---*/

/*===
[{"_undef":true},null,{"_undef":true},{"_undef":true},{"_undef":true},1,{"_undef":true}]
===*/

function test() {
    // Array gaps: '{"_undef":true}'
    print(Duktape.enc('jc', [ void 0, null,,,, 1,, ]));
}

test();
