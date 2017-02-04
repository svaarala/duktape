/*
 *  Bug testcase for recursive mark-and-sweep entry from refzero triggered
 *  voluntary GC, https://github.com/svaarala/duktape/pull/1347.
 */

/*===
0
1
2
3
4
5
6
7
8
9
done
===*/

function myFinalizer(obj) {
    var t;
    var i, j;

    for (i = 0; i < 10; i++) {
        // We need to cause a lot of allocation activity without running
        // out of memory (which is possible because GC cannot currently run
        // inside a finalizer).  Use property table resizes to create the
        // activity, and then trigger a voluntary GC from refzero.

        t = {};

        for (j = 1; j < 1e3; j++) {
            t['prop' + j] = true;
        }
        for (j = 0; j < 1e3; j++) {
            delete t['prop' + j];
        }

        t = null;
    }
}

function test() {
    var i;
    var obj;

    for (i = 0; i < 10; i++) {
        print(i);

        obj = {};
        obj.ref = {};
        obj.ref.back = obj;  // circular refs

        Duktape.fin(obj, myFinalizer);
        Duktape.fin(obj.ref, myFinalizer);

        obj = null;
        Duktape.gc();
        Duktape.gc();
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
