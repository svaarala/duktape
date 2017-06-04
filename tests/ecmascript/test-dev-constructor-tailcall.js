/*
 *  Since Duktape 2.2 constructor calls can be tailcalled.
 */

/*===
object
0
5001
===*/

var count = 0;

function MyConstructor(arg) {
    count++;
    if (arg === 0) {
        this.arg = arg;
        return;
    }
    return new MyConstructor(arg - 1);
}

function test() {
    var res = new MyConstructor(5000);
    print(typeof res);
    print(res.arg);
    print(count);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
