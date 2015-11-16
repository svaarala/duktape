/*
 *  Testcases for error handler behavior from Ecmascript code point of view.
 *  Checks both Duktape.errCreate and Duktape.errThrow.
 */

/*===
errCreate
- no errCreate
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain errCreate (sets foo and bar)
error: ReferenceError: identifier 'aiee' undefined, foo: 1, bar: 2
error: URIError: fake uri error, foo: 1, bar: 2
- errCreate gets only error instances
errCreate: object true foo
errCreate: object true bar
errCreate: object true quux
errCreate: object true quux
errCreate: object true baz
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
- errCreate throws an error
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: ReferenceError: identifier 'zork' undefined, foo: undefined, bar: undefined
- non-callable errCreate
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: 123 not callable, foo: undefined, bar: undefined
- "undefined" (but set) errCreate
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: undefined not callable, foo: undefined, bar: undefined
- delete errCreate property
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- errCreate as an accessor property is ignored
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- recursive errCreate
error: RangeError: test error, foo: undefined, bar: undefined
error: ReferenceError: identifier 'aiee' undefined, foo: 1, bar: 2
error: RangeError: test error, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: 1, bar: 2
===*/

function errCreateTest() {
    delete Duktape.errThrow;
    delete Duktape.errCreate;

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
     *  Normal, default case: no errCreate
     */

    print('- no errCreate');
    errTest1();
    errTest2();

    /*
     *  Basic errCreate.
     */

    print('- plain errCreate (sets foo and bar)');
    Duktape.errCreate = function (err) {
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
     *  Error gets errCreate processed twice: (1) when it is created
     *  inside Constructor2, and (2) when Constructor2 returns and
     *  the final constructed value gets checked.
     */

    print('- errCreate gets only error instances');
    Duktape.errCreate = function (err) {
        if (err === null) { print('errCreate:', null); }
        else if (typeof err === 'object') { print('errCreate:', typeof err, err instanceof Error, err.message); }
        else { print('errCreate:', typeof err); }
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
     *  If an errCreate causes an error, that error won't be augmented.
     *
     *  There is some inconsistent behavior here now.  If the original error
     *  is thrown by Duktape itself (referencing 'aiee') the second error
     *  causes the error to be replaced with a DoubleError.  However, if the
     *  original error is thrown by Ecmascript code (throw X) the error from
     *  errCreate will replace the original error as is (here it will be
     *  a ReferenceError caused by referencing 'zork').
     */

    print('- errCreate throws an error');
    Duktape.errCreate = function (err) {
        err.foo = 1;
        zork;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  If errCreate is set but is not callable, original error is
     *  replaced with another error - either DoubleError or TypeError.
     *
     *  The same inconsistency appears here as well: if original error
     *  is thrown by Duktape internally, the final result is a DoubleError,
     *  otherwise a TypeError.
     */

    print('- non-callable errCreate');
    Duktape.errCreate = 123;
    errTest1();
    errTest2();

    /*
     *  Setting to undefined/null does *not* remove the errCreate, but
     *  will still cause DoubleError/TypeError.
     */

    print('- "undefined" (but set) errCreate');
    Duktape.errCreate = undefined;
    errTest1();
    errTest2();

    /*
     *  The proper way to remove an errCreate is to delete the property.
     */

    print('- delete errCreate property');
    delete Duktape.errCreate;
    errTest1();
    errTest2();

    /*
     *  An accessor errCreate is ignored.
     */

    print('- errCreate as an accessor property is ignored');
    Object.defineProperty(Duktape, 'errCreate', {
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
    delete Duktape.errCreate;

    /*
     *  If an error is created within an errCreate, it won't get augmented
     *  with errCreate.
     */

    print('- recursive errCreate');
    Duktape.errCreate = function (err) {
        err.foo = 1;
        err.bar = 2;
        var test = new RangeError('test error');  // won't be augmented
        errPrint(test);
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  Unlike errThrow, errCreate does not have any interaction with coroutines,
     *  so no yield/resume tests here.
     */
}

print
print('errCreate');

try {
    errCreateTest();
} catch (e) {
    print(e);
}

/*===
errThrow
- no errThrow
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain errThrow (sets foo and bar)
error: ReferenceError: identifier 'aiee' undefined, foo: 1, bar: 2
error: URIError: fake uri error, foo: 1, bar: 2
- errThrow gets all value types
errThrow: undefined
catch: undefined
errThrow: null
catch: null
errThrow: boolean
catch: boolean
errThrow: number
catch: number
errThrow: string
catch: string
errThrow: object false undefined
catch: object
errThrow: object false undefined
catch: object
errThrow: function
catch: function
errThrow: buffer
catch: buffer
errThrow: pointer
catch: pointer
errThrow: object false undefined
catch: object
errThrow: object false undefined
catch: object
errThrow: object true foo
catch: object
errThrow: object true bar
catch: object
errThrow: object false undefined
catch: object
errThrow: object true quux
catch: object
errThrow: object false undefined
catch: object
errThrow: object true baz
catch: object
- errThrow throws an error
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: ReferenceError: identifier 'zork' undefined, foo: undefined, bar: undefined
- non-callable errThrow
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: 123 not callable, foo: undefined, bar: undefined
- "undefined" (but set) errThrow
error: DoubleError: error in error handling, foo: undefined, bar: undefined
error: TypeError: undefined not callable, foo: undefined, bar: undefined
- delete errThrow property
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- errThrow as an accessor property is ignored
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
error: URIError: fake uri error, foo: undefined, bar: undefined
- plain errThrow, follows into resumed thread
error: ReferenceError: identifier 'aiee' undefined, foo: bar, bar: quux
error: URIError: fake uri error, foo: bar, bar: quux
error: ReferenceError: identifier 'aiee' undefined, foo: bar, bar: quux
error: URIError: fake uri error, foo: bar, bar: quux
- plain errThrow, called in yield/resume when isError is true
caught in resume
error: URIError: fake uri error (resume), foo: bar, bar: quux
caught yield
error: URIError: fake uri error (yield), foo: bar, bar: quux
===*/

function errThrowTest() {
    var thr;

    delete Duktape.errThrow;
    delete Duktape.errCreate;

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
     *  Normal, default case: no errThrow
     */

    print('- no errThrow');
    errTest1();
    errTest2();

    /*
     *  Basic errThrow.
     */

    print('- plain errThrow (sets foo and bar)');
    Duktape.errThrow = function (err) {
        if (!(err instanceof Error)) { return err; }
        err.foo = 1;
        err.bar = 2;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  An errThrow handler gets whatever values are thrown and must deal
     *  with them properly.
     */

    print('- errThrow gets all value types');
    Duktape.errThrow = function (err) {
        if (err === null) { print('errThrow:', null); }
        else if (typeof err === 'object') { print('errThrow:', typeof err, err instanceof Error, err.message); }
        else { print('errThrow:', typeof err); }
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
     *  If an errThrow causes an error, that error won't be augmented.
     *
     *  There is some inconsistent behavior here now.  If the original error
     *  is thrown by Duktape itself (referencing 'aiee') the second error
     *  causes the error to be replaced with a DoubleError.  However, if the
     *  original error is thrown by Ecmascript code (throw X) the error from
     *  errThrow will replace the original error as is (here it will be
     *  a ReferenceError caused by referencing 'zork').
     */

    print('- errThrow throws an error');
    Duktape.errThrow = function (err) {
        if (!(err instanceof Error)) { return err; }
        err.foo = 1;
        zork;
        return err;
    };
    errTest1();
    errTest2();

    /*
     *  If errThrow is set but is not callable, original error is
     *  replaced with another error - either DoubleError or TypeError.
     *
     *  The same inconsistency appears here as well: if original error
     *  is thrown by Duktape internally, the final result is a DoubleError,
     *  otherwise a TypeError.
     */

    print('- non-callable errThrow');
    Duktape.errThrow = 123;
    errTest1();
    errTest2();

    /*
     *  Setting to undefined/null does *not* remove the errThrow, but
     *  will still cause DoubleError/TypeError.
     */

    print('- "undefined" (but set) errThrow');
    Duktape.errThrow = undefined;
    errTest1();
    errTest2();

    /*
     *  The proper way to remove an errThrow is to delete the property.
     */

    print('- delete errThrow property');
    delete Duktape.errThrow;
    errTest1();
    errTest2();

    /*
     *  An accessor errThrow is ignored.
     */

    print('- errThrow as an accessor property is ignored');
    Object.defineProperty(Duktape, 'errThrow', {
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
    delete Duktape.errThrow;

    /*
     *  An errThrow follows into a resumed thread.
     */

    print('- plain errThrow, follows into resumed thread')
    Duktape.errThrow = function (err) {
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
     *  the errThrow.
     */

    print('- plain errThrow, called in yield/resume when isError is true');

    Duktape.errThrow = function (err) {
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

print('errThrow');

try {
    errThrowTest();
} catch (e) {
    print(e);
}

/*===
errCreate + errThrow
enter errCreate: Error: initial error undefined undefined
error: URIError: fake error (in errCreate), foo: undefined, bar: undefined
error: ReferenceError: identifier 'zork' undefined, foo: undefined, bar: undefined
e1 tracedata existence matches: true
e2 tracedata existence matches: true
exit errCreate: Error: initial error added-by-errCreate undefined
enter errThrow Error: initial error added-by-errCreate undefined
error: URIError: fake error (in errThrow), foo: undefined, bar: undefined
error: ReferenceError: identifier 'aiee' undefined, foo: undefined, bar: undefined
e1 tracedata existence matches: true
e2 tracedata existence matches: true
exit errThrow: Error: initial error added-by-errCreate added-by-errThrow
in catch
error: Error: initial error, foo: added-by-errCreate, bar: added-by-errThrow
===*/

/* When errCreate is running, errors created and thrown inside the handler
 * will not trigger further errCreate/errThrow calls.  Similarly, when
 * errThrow is running, recursive errCreate/errThrow calls are not made.
 *
 * The built-in error augmentation (tracedata) still happens.
 */

function errCreateAndErrThrowTest() {
    delete Duktape.errThrow;
    delete Duktape.errCreate;

    function errPrint(err) {
        print('error: ' + String(err) + ', foo: ' + String(err.foo) +
              ', bar: ' + String(err.bar));
        //print(err.stack);
    }

    Duktape.errThrow = function (err) {
        print('enter errThrow', err, err.foo, err.bar);
        err.bar = 'added-by-errThrow';

        var e1 = new URIError('fake error (in errThrow)');
        try {
            aiee;
        } catch (e) {
            e2 = e;
        }
        errPrint(e1);
        errPrint(e2);
        print('e1 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e1));
        print('e2 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e2));

        print('exit errThrow:', err, err.foo, err.bar);
        return err;
    }
    Duktape.errCreate = function (err) {
        print('enter errCreate:', err, err.foo, err.bar);
        err.foo = 'added-by-errCreate';

        var e1 = new URIError('fake error (in errCreate)');
        try {
            zork;
        } catch (e) {
            e2 = e;
        }
        errPrint(e1);
        errPrint(e2);
        print('e1 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e1));
        print('e2 tracedata existence matches:', ('tracedata' in err === 'tracedata' in e2));

        print('exit errCreate:', err, err.foo, err.bar);
        return err;
    }

    try {
        throw new Error('initial error');
    } catch (e) {
        print('in catch');
        errPrint(e);
    }
}

print('errCreate + errThrow');

try {
    errCreateAndErrThrowTest();
} catch (e) {
    print(e);
}
