// https://github.com/svaarala/duktape/issues/2204

/*===
0
done
===*/

function yielder() {
    var yield = Duktape.Thread.yield;
    t = {};
    yield(0);
}
var t = Duktape.Thread(yielder);
print(Duktape.Thread.resume(t));
print('done');
