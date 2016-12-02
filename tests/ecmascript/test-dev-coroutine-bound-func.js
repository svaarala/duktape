/*
 *  Coroutine with initial bound function.
 */

/*===
mythread starting
object mythis
["FOO","BAR","foo"]
1
2
3
undefined
===*/

function test() {
    function mythread(a,b,c) {
        print('mythread starting');
        print(typeof this, this);
        print(JSON.stringify([a,b,c]));
        Duktape.Thread.yield(1);
        Duktape.Thread.yield(2);
        Duktape.Thread.yield(3);
    }

    var T = new Duktape.Thread(mythread.bind('mythis', 'FOO', 'BAR'));

    // Only one argument can be given for the initial call, but the bound
    // arguments are still prepended as normal.
    print(Duktape.Thread.resume(T, 'foo'));

    // No difference in further resumes.
    print(Duktape.Thread.resume(T, 'foo'));
    print(Duktape.Thread.resume(T, 'foo'));
    print(Duktape.Thread.resume(T, 'foo'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
