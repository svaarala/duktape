/*
 *  https://github.com/svaarala/duktape/issues/2049
 */

/*===
still here
===*/

try {
    // Note: variable names don't make much sense.
    var origEval = new Proxy(Function, {});
    eval = origEval.bind();
    eval();
    print('still here');
} catch (e) {
    print(e.stack || e);
}

/*===
function called
this: mythis
args: 1 2 3 4 undefined
still here
===*/

try {
    // Extend original repro case, ensure target gets called.
    var origEval = new Proxy(function (a, b, c, d, e) {
        print('function called');
        print('this:', this);
        print('args:', a, b, c, d, e);
    }, {});
    eval = origEval.bind('mythis', 1);
    eval(2, 3, 4);
    print('still here');
} catch (e) {
    print(e.stack || e);
}

/*===
done
===*/

print('done');
