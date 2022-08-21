// https://github.com/svaarala/duktape/issues/2204

/*---
custom: true
---*/

/*===
0
===*/

function yielder() {
    var yield = Duktape.Thread.yield;
    t = {};
    yield(0);
}
var t = Duktape.Thread(yielder);
print(Duktape.Thread.resume(t));
