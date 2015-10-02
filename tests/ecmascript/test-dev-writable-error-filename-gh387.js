/*
 *  Test that error .fileName, .lineNumber, and .stack are all writable.
 *
 *  These properties are not part of E5, but they are present in e.g. V8.
 *  Before Duktape 1.4.0 the properties were not directly writable, but you
 *  could use Object.defineProperty() to overwrite them.  This was changed
 *  in Duktape 1.4.0 to match V8 and Spidermonkey behavior.
 *
 *  Also when Duktape 1.3.0 and prior was compiled without tracebacks,
 *  .fileName and .lineNumber were writable, so the change in Duktape 1.4.0
 *  makes traceback-enabled behavior more consistent.
 *
 *  https://github.com/svaarala/duktape/issues/387
 */

/*===
["message"]
dummy
99999
Foo
	bar
	quux
["message","fileName","lineNumber","stack"]
true true true
false false false
{get:{_func:true},set:{_func:true},enumerable:false,configurable:true}
true true true
true true true
===*/

function test() {
    var err = new Error('aiee');
    var pd;

    // By default an Error instance only has a .message property; the rest
    // of the properties are provided by inherited accessors based on an
    // internal _Tracedata property.
    print(JSON.stringify(Object.getOwnPropertyNames(err)));

    // .fileName is writable
    err.fileName = 'dummy';
    print(err.fileName);

    // .lineNumber is writable
    err.lineNumber = 99999;
    print(err.lineNumber);

    // Note that the stacktrace uses file/line information from the call
    // stack functions and will ignore the .fileName and .lineNumber "own"
    // properties.

    //print(err.stack)

    // .stack is also writable
    err.stack = 'Foo\n\tbar\n\tquux';
    print(err.stack);

    // All of the overwritten values appear as own properties which will
    // shadow the default accessors.
    print(JSON.stringify(Object.getOwnPropertyNames(err)));

    function writabilityTest() {
        err = new Error('zork');
        var origFileName = err.fileName;
        var origLineNumber = err.lineNumber;
        var origStack = err.stack;
        print(err.fileName === origFileName, err.lineNumber === origLineNumber, err.stack === origStack);
        err.fileName = 'dummy';
        err.lineNumber = 99999;
        err.stack = 'Foo\nBar';
        print(err.fileName === origFileName, err.lineNumber === origLineNumber, err.stack === origStack);
    }

    // If you prefer the Duktape 1.3.0 behavior, you can edit the inherited
    // accessors.  For example, if the setter is made a no-op, Duktape 1.3.0
    // behavior is restored.

    writabilityTest();  // writable

    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'fileName');
    print(Duktape.enc('jx', pd));
    Object.defineProperties(Error.prototype, {
        fileName: { set: function nop() {} },
        lineNumber: { set: function nop() {} },
        stack: { set: function nop() {} }
    });

    writabilityTest();  // not writable, Duktape 1.3.0 behavior
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
