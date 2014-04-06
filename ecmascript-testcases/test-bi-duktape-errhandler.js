/*
 *  Testcases for error handler behavior from Ecmascript code point of view.
 */

/*===
errhandler
- no error handler
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain error handler (sets foo and bar)
error: ReferenceError: identifier 'aiee' undefined, foo: 1, bar: 2
error: URIError: fake uri error, foo: 1, bar: 2
- error handler throws an error
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: ReferenceError: identifier 'zork' undefined, foo: undefined, bar: undefined
- non-callable error handler
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: call target not callable, foo: undefined, bar: undefined
- "undefined" (but set) error handler
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: call target not callable, foo: undefined, bar: undefined
- delete error handler property
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- error handler as an accessor property is ignored
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain error handler, follows into resumed thread
error: ReferenceError: identifier 'aiee' undefined, foo: bar, bar: quux
error: URIError: fake uri error, foo: bar, bar: quux
error: ReferenceError: identifier 'aiee' undefined, foo: bar, bar: quux
error: URIError: fake uri error, foo: bar, bar: quux
- plain error handler, called in yield/resume when isError is true
caught in resume
error: URIError: fake uri error (resume), foo: bar, bar: quux
caught yield
error: URIError: fake uri error (yield), foo: bar, bar: quux
===*/

function errHandlerTest() {
    var thr;

    function errPrint(err) {
        print('error: ' + String(err) + ', foo: ' + String(err.foo) +
              ', bar: ' + String(err.bar));
        //print(err.stack);
    }

    // Error thrown from Duktape internals
    function errTest1() {
        try {
            aiee;
        } catch (e) {
            errPrint(e);
        }
    }

    // Error thrown from Ecmascript
    function errTest2() {
        try {
            throw new URIError('fake uri error');
        } catch (e) {
            errPrint(e);
        }
    }

    // Note: error thrown from C code with the Duktape C API is tested
    // with API test cases.

    /*
     *  Normal, default case: no error handler
     */

    print('- no error handler');
    errTest1();
    errTest2();

    /*
     *  Basic error handler.
     */

    print('- plain error handler (sets foo and bar)');
    Duktape.errhnd = function (err) {
        err.foo = 1;
        err.bar = 2;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  If an error handler causes an error, that error won't be augmented.
     *
     *  There is some inconsistent behavior here now.  If the original error
     *  is thrown by Duktape itself (referencing 'aiee') the second error
     *  causes the error to be replaced with a DoubleError.  However, if the
     *  original error is thrown by Ecmascript code (throw X) the error from
     *  error handler will replace the original error as is (here it will be
     *  a ReferenceError caused by referencing 'zork').
     */

    print('- error handler throws an error');
    Duktape.errhnd = function (err) {
        err.foo = 1;
        zork;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  If error handler is set but is not callable, original error is
     *  replaced with another error - either DoubleError or TypeError.
     *
     *  The same inconsistency appears here as well: if original error
     *  is thrown by Duktape internally, the final result is a DoubleError,
     *  otherwise a TypeError.
     */

    print('- non-callable error handler');
    Duktape.errhnd = 123;
    errTest1();
    errTest2();

    /*
     *  Setting to undefined/null does *not* remove the error handler, but
     *  will still cause DoubleError/TypeError.
     */

    print('- "undefined" (but set) error handler');
    Duktape.errhnd = undefined;
    errTest1();
    errTest2();

    /*
     *  The proper way to remove an error handler is to delete the property.
     */

    print('- delete error handler property');
    delete Duktape.errhnd;
    errTest1();
    errTest2();

    /*
     *  An accessor errhnd is ignored.
     */

    print('- error handler as an accessor property is ignored');
    Object.defineProperty(Duktape, 'errhnd', {
        get: function () { return function(err) { err.foo = 'called'; return err; } },
        set: function () { throw new Error('setter called'); },
        enumerable: true,
        configurable: true
    });
    errTest1();
    errTest2();
    delete Duktape.errhnd;

    /*
     *  An error handler follows into a resumed thread.
     */

    print('- plain error handler, follows into resumed thread')
    Duktape.errhnd = function (err) {
        err.foo = 'bar';
        err.bar = 'quux';
        return err;
    };

    thr = new Duktape.Thread(function () {
        // run test inside called thread
        errTest1();
        errTest2();
    });
    Duktape.Thread.resume(thr);

    thr = new Duktape.Thread(function () {
        // throw the error from inside the thread and catch in the resumer
        aiee;
    });
    try {
        Duktape.Thread.resume(thr);
    } catch (e) {
        errPrint(e);
    }

    thr = new Duktape.Thread(function () {
        // throw the error from inside the thread and catch in the resumer
        throw new URIError('fake uri error');
    });
    try {
        Duktape.Thread.resume(thr);
    } catch (e) {
        errPrint(e);
    }

    /*
     *  In addition to Duktape internal errors and explicit Ecmascript
     *  throws, coroutine yield() / resume() errors are processed with
     *  the error handler.
     */

    print('- plain error handler, called in yield/resume when isError is true');

    Duktape.errhnd = function (err) {
        err.foo = 'bar';
        err.bar = 'quux';
        return err;
    };

    thr = new Duktape.Thread(function () {
        try {
            Duktape.Thread.yield();
        } catch (e) {
            print('caught in resume');
            errPrint(e);
        }
    });
    Duktape.Thread.resume(thr);  // until yield()
    Duktape.Thread.resume(thr, new URIError('fake uri error (resume)'), true);  // true=isError

    thr = new Duktape.Thread(function () {
        Duktape.Thread.yield(new URIError('fake uri error (yield)'), true);  // true=isError
    });
    try {
        Duktape.Thread.resume(thr);
    } catch (e) {
        print('caught yield');
        errPrint(e);
    }
}

print('errhandler');

try {
    errHandlerTest();
} catch (e) {
    print(e);
}
