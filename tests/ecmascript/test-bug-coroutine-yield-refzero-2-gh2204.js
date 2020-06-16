/*===
RangeError: aiee
done
===*/

function yielder() {
    var yield = Duktape.Thread.yield;
    t = {};
    throw new RangeError('aiee');
}
var t = Duktape.Thread(yielder);
try {
    Duktape.Thread.resume(t);
} catch (e) {
    print(e);
}
print('done');
