/*
 *  Pure JS repro for one issue in https://github.com/svaarala/duktape/issues/1311.
 */

/*---
custom: true
---*/

/*===
Error: a pig ate it
===*/

function Foo() {
    throw new Error('a pig ate it');
}

Duktape.fin(Foo.prototype, function(o) {});

try {
    new Foo();
} catch (e) {
    print(e);
}
