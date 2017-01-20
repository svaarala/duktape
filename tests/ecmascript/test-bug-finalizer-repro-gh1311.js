/*
 *  Pure JS repro for one issue in https://github.com/svaarala/duktape/issues/1311.
 */

/*===
Error: a pig ate it
done
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

print('done');
