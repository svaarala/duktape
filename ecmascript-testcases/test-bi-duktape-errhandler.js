/*
 *  Testcases for error handler behavior from Ecmascript code point of view.
 *  Checks both Duktape.errcreate and Duktape.errthrow.
 */

/*===
errcreate
- no errcreate
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain errcreate (sets foo and bar)
error: ReferenceError: identifier 'aiee' undefined, foo: 1, bar: 2
error: URIError: fake uri error, foo: 1, bar: 2
- errcreate gets only error instances
errcreate: object true foo
errcreate: object true bar
errcreate: object true quux
errcreate: object true quux
errcreate: object true baz
catch: undefined
catch: null
catch: boolean
catch: number
catch: string
catch: object
catch: object
catch: function
catch: buffer
catch: pointer
catch: object
catch: object
catch: object
catch: object
catch: object
catch: object
catch: object
catch: object
- errcreate throws an error
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: ReferenceError: identifier 'zork' undefined, foo: undefined, bar: undefined
- non-callable errcreate
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: call target not callable, foo: undefined, bar: undefined
- "undefined" (but set) errcreate
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: call target not callable, foo: undefined, bar: undefined
- delete errcreate property
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- errcreate as an accessor property is ignored
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- recursive errcreate
error: RangeError: test error, foo: undefined, bar: undefined
error: ReferenceError: identifier 'aiee' undefined, foo: 1, bar: 2
error: RangeError: test error, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: 1, bar: 2
===*/

function errCreateTest() {
    delete Duktape.errthrow;
    delete Duktape.errcreate;

    function errPrint(err) {
        print('error: ' + String(err) + ', foo: ' + String(err.foo) +
              ', bar: ' + String(err.bar));
        //print(err.stack);
    }

    // Error created from Duktape internals
    function errTest1() {
        // No way to create an error from Duktape internals without
        // throwing it.
        try {
            aiee;
        } catch (e) {
            errPrint(e);
        }
    }

    // Error created from Ecmascript
    function errTest2() {
        var e = new URIError('fake uri error');
        errPrint(e);
    }

    // Note: error created from C code with the Duktape C API is tested
    // with API test cases.

    /*
     *  Normal, default case: no errcreate
     */

    print('- no errcreate');
    errTest1();
    errTest2();

    /*
     *  Basic errcreate.
     */

    print('- plain errcreate (sets foo and bar)');
    Duktape.errcreate = function (err) {
        err.foo = 1;
        err.bar = 2;
        return err;  // NOTE: the error must be returned; if you don't, 'undefined' will replace the error
    };
    errTest1();
    errTest2();

    /*
     *  Errcreate callback only gets called with Error instances,
     *  because only they are augmented by Duktape.
     *
     *  Errcreate does get called even if a constructor replaces the
     *  default constructed value for a constructor which creates
     *  Error instances.
     *
     *  Constructor2 test also illustrates a corner case: the 'quux'
     *  Error gets errcreate processed twice: (1) when it is created
     *  inside Constructor2, and (2) when Constructor2 returns and
     *  the final constructed value gets checked.
     */

    print('- errcreate gets only error instances');
    Duktape.errcreate = function (err) {
        if (err === null) { print('errcreate:', null); }
        else if (typeof err === 'object') { print('errcreate:', typeof err, err instanceof Error, err.message); }
        else { print('errcreate:', typeof err); }
        return err;
    };
    function Constructor1() {
        return 123;  // attempt to replace with a non-object, ignored
    }
    function Constructor2() {
        return new Error('quux');  // replace normal object with an error
    }
    function Constructor3() {
        return {};  // replace Error instance with a normal object
    }
    Constructor3.prototype = Error.prototype;
    function Constructor4() {
        this.message = 'baz';
        return 123;  // keep constructed error
    }
    Constructor4.prototype = Error.prototype;

    [ undefined, null, true, 123, 'foo', [ 'foo', 'bar' ], { foo:1, bar:2 },
      function () {}, Duktape.Buffer('foo'), Duktape.Pointer('dummy'),
      new Object(), new Array(), new Error('foo'), Error('bar'),
      new Constructor1(), new Constructor2(),
      new Constructor3(), new Constructor4() ].forEach(function (v) {
        try {
            throw v;
        } catch (err) {
            if (err === null) { print('catch:', null); }
            else { print('catch:', typeof err); }
        }
    });

    /*
     *  If an errcreate causes an error, that error won't be augmented.
     *
     *  There is some inconsistent behavior here now.  If the original error
     *  is thrown by Duktape itself (referencing 'aiee') the second error
     *  causes the error to be replaced with a DoubleError.  However, if the
     *  original error is thrown by Ecmascript code (throw X) the error from
     *  errcreate will replace the original error as is (here it will be
     *  a ReferenceError caused by referencing 'zork').
     */

    print('- errcreate throws an error');
    Duktape.errcreate = function (err) {
        err.foo = 1;
        zork;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  If errcreate is set but is not callable, original error is
     *  replaced with another error - either DoubleError or TypeError.
     *
     *  The same inconsistency appears here as well: if original error
     *  is thrown by Duktape internally, the final result is a DoubleError,
     *  otherwise a TypeError.
     */

    print('- non-callable errcreate');
    Duktape.errcreate = 123;
    errTest1();
    errTest2();

    /*
     *  Setting to undefined/null does *not* remove the errcreate, but
     *  will still cause DoubleError/TypeError.
     */

    print('- "undefined" (but set) errcreate');
    Duktape.errcreate = undefined;
    errTest1();
    errTest2();

    /*
     *  The proper way to remove an errcreate is to delete the property.
     */

    print('- delete errcreate property');
    delete Duktape.errcreate;
    errTest1();
    errTest2();

    /*
     *  An accessor errcreate is ignored.
     */

    print('- errcreate as an accessor property is ignored');
    Object.defineProperty(Duktape, 'errcreate', {
        get: function () {
            return function(err) {
                err.foo = 'called';
                return err;
            }
        },
        set: function () {
            throw new Error('setter called');
        },
        enumerable: true,
        configurable: true
    });
    errTest1();
    errTest2();
    delete Duktape.errcreate;

    /*
     *  If an error is created within an errcreate, it won't get augmented
     *  with errcreate.
     */

    print('- recursive errcreate');
    Duktape.errcreate = function (err) {
        err.foo = 1;
        err.bar = 2;
        var test = new RangeError('test error');  // won't be augmented
        errPrint(test);
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  Unlike errthrow, errcreate does not have any interaction with coroutines,
     *  so no yield/resume tests here.
     */
}

print
print('errcreate');

try {
    errCreateTest();
} catch (e) {
    print(e);
}

/*===
errthrow
- no errthrow
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain errthrow (sets foo and bar)
error: ReferenceError: identifier 'aiee' undefined, foo: 1, bar: 2
error: URIError: fake uri error, foo: 1, bar: 2
- errthrow gets all value types
errthrow: undefined
catch: undefined
errthrow: null
catch: null
errthrow: boolean
catch: boolean
errthrow: number
catch: number
errthrow: string
catch: string
errthrow: object false undefined
catch: object
errthrow: object false undefined
catch: object
errthrow: function
catch: function
errthrow: buffer
catch: buffer
errthrow: pointer
catch: pointer
errthrow: object false undefined
catch: object
errthrow: object false undefined
catch: object
errthrow: object true foo
catch: object
errthrow: object true bar
catch: object
errthrow: object false undefined
catch: object
errthrow: object true quux
catch: object
errthrow: object false undefined
catch: object
errthrow: object true baz
catch: object
- errthrow throws an error
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: ReferenceError: identifier 'zork' undefined, foo: undefined, bar: undefined
- non-callable errthrow
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: call target not callable, foo: undefined, bar: undefined
- "undefined" (but set) errthrow
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: call target not callable, foo: undefined, bar: undefined
- delete errthrow property
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- errthrow as an accessor property is ignored
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain errthrow, follows into resumed thread
error: ReferenceError: identifier 'aiee' undefined, foo: bar, bar: quux
error: URIError: fake uri error, foo: bar, bar: quux
error: ReferenceError: identifier 'aiee' undefined, foo: bar, bar: quux
error: URIError: fake uri error, foo: bar, bar: quux
- plain errthrow, called in yield/resume when isError is true
caught in resume
error: URIError: fake uri error (resume), foo: bar, bar: quux
caught yield
error: URIError: fake uri error (yield), foo: bar, bar: quux
===*/

function errThrowTest() {
    var thr;

    delete Duktape.errthrow;
    delete Duktape.errcreate;

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
     *  Normal, default case: no errthrow
     */

    print('- no errthrow');
    errTest1();
    errTest2();

    /*
     *  Basic errthrow.
     */

    print('- plain errthrow (sets foo and bar)');
    Duktape.errthrow = function (err) {
        if (!(err instanceof Error)) { return err; }
        err.foo = 1;
        err.bar = 2;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  An errthrow handler gets whatever values are thrown and must deal
     *  with them properly.
     */

    print('- errthrow gets all value types');
    Duktape.errthrow = function (err) {
        if (err === null) { print('errthrow:', null); }
        else if (typeof err === 'object') { print('errthrow:', typeof err, err instanceof Error, err.message); }
        else { print('errthrow:', typeof err); }
        return err;
    };
    function Constructor1() {
        return 123;  // attempt to replace with a non-object, ignored
    }
    function Constructor2() {
        return new Error('quux');  // replace normal object with an error
    }
    function Constructor3() {
        return {};  // replace Error instance with a normal object
    }
    Constructor3.prototype = Error.prototype;
    function Constructor4() {
        this.message = 'baz';
        return 123;  // keep constructed error
    }
    Constructor4.prototype = Error.prototype;

    [ undefined, null, true, 123, 'foo', [ 'foo', 'bar' ], { foo:1, bar:2 },
      function () {}, Duktape.Buffer('foo'), Duktape.Pointer('dummy'),
      new Object(), new Array(), new Error('foo'), Error('bar'),
      new Constructor1(), new Constructor2(),
      new Constructor3(), new Constructor4() ].forEach(function (v) {
        try {
            throw v;
        } catch (err) {
            if (err === null) { print('catch:', null); }
            else { print('catch:', typeof err); }
        }
    });

    /*
     *  If an errthrow causes an error, that error won't be augmented.
     *
     *  There is some inconsistent behavior here now.  If the original error
     *  is thrown by Duktape itself (referencing 'aiee') the second error
     *  causes the error to be replaced with a DoubleError.  However, if the
     *  original error is thrown by Ecmascript code (throw X) the error from
     *  errthrow will replace the original error as is (here it will be
     *  a ReferenceError caused by referencing 'zork').
     */

    print('- errthrow throws an error');
    Duktape.errthrow = function (err) {
        if (!(err instanceof Error)) { return err; }
        err.foo = 1;
        zork;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  If errthrow is set but is not callable, original error is
     *  replaced with another error - either DoubleError or TypeError.
     *
     *  The same inconsistency appears here as well: if original error
     *  is thrown by Duktape internally, the final result is a DoubleError,
     *  otherwise a TypeError.
     */

    print('- non-callable errthrow');
    Duktape.errthrow = 123;
    errTest1();
    errTest2();

    /*
     *  Setting to undefined/null does *not* remove the errthrow, but
     *  will still cause DoubleError/TypeError.
     */

    print('- "undefined" (but set) errthrow');
    Duktape.errthrow = undefined;
    errTest1();
    errTest2();

    /*
     *  The proper way to remove an errthrow is to delete the property.
     */

    print('- delete errthrow property');
    delete Duktape.errthrow;
    errTest1();
    errTest2();

    /*
     *  An accessor errthrow is ignored.
     */

    print('- errthrow as an accessor property is ignored');
    Object.defineProperty(Duktape, 'errthrow', {
        get: function () {
            return function(err) {
                if (!(err instanceof Error)) { return err; }
                err.foo = 'called';
                return err;
            }
        },
        set: function () { throw new Error('setter called'); },
        enumerable: true,
        configurable: true
    });
    errTest1();
    errTest2();
    delete Duktape.errthrow;

    /*
     *  An errthrow follows into a resumed thread.
     */

    print('- plain errthrow, follows into resumed thread')
    Duktape.errthrow = function (err) {
        if (!(err instanceof Error)) { return err; }
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
     *  the errthrow.
     */

    print('- plain errthrow, called in yield/resume when isError is true');

    Duktape.errthrow = function (err) {
        if (!(err instanceof Error)) { return err; }
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

print('errthrow');

try {
    errThrowTest();
} catch (e) {
    print(e);
}

/*===
errcreate + errthrow
enter errcreate: Error: initial error undefined undefined
error: URIError: fake error (in errcreate), foo: undefined, bar: undefined
error: ReferenceError: identifier 'zork' undefined, foo: undefined, bar: undefined
e1 tracedata existence matches: true
e2 tracedata existence matches: true
exit errcreate: Error: initial error added-by-errcreate undefined
enter errthrow Error: initial error added-by-errcreate undefined
error: URIError: fake error (in errthrow), foo: undefined, bar: undefined
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
e1 tracedata existence matches: true
e2 tracedata existence matches: true
exit errthrow: Error: initial error added-by-errcreate added-by-errthrow
in catch
error: Error: initial error, foo: added-by-errcreate, bar: added-by-errthrow
===*/

/* When errcreate is running, errors created and thrown inside the handler
 * will not trigger further errcreate/errthrow calls.  Similarly, when
 * errthrow is running, recursive errcreate/errthrow calls are not made.
 *
 * The built-in error augmentation (tracedata) still happens.
 */

function errCreateAndErrThrowTest() {
    delete Duktape.errthrow;
    delete Duktape.errcreate;

    function errPrint(err) {
        print('error: ' + String(err) + ', foo: ' + String(err.foo) +
              ', bar: ' + String(err.bar));
        //print(err.stack);
    }

    Duktape.errthrow = function (err) {
        print('enter errthrow', err, err.foo, err.bar);
        err.bar = 'added-by-errthrow';

        var e1 = new URIError('fake error (in errthrow)');
        try {
            aiee;
        } catch (e) {
            e2 = e;
        }
        errPrint(e1);
        errPrint(e2);
        print('e1 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e1));
        print('e2 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e2));

        print('exit errthrow:', err, err.foo, err.bar);
        return err;
    }
    Duktape.errcreate = function (err) {
        print('enter errcreate:', err, err.foo, err.bar);
        err.foo = 'added-by-errcreate';

        var e1 = new URIError('fake error (in errcreate)');
        try {
            zork;
        } catch (e) {
            e2 = e;
        }
        errPrint(e1);
        errPrint(e2);
        print('e1 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e1));
        print('e2 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e2));

        print('exit errcreate:', err, err.foo, err.bar);
        return err;
    }

    try {
        throw new Error('initial error');
    } catch (e) {
        print('in catch');
        errPrint(e);
    }
}

print('errcreate + errthrow');

try {
    errCreateAndErrThrowTest();
} catch (e) {
    print(e);
}
