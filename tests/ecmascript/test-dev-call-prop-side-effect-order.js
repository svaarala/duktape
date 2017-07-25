/*===
getter
foo
bar
quux
TypeError: 'notCallable' not callable (property 'func' of [object Array])
TypeError: [object Error] not callable
===*/

var base = [];
Object.defineProperty(base, 'func', {
    get: function myGetter() { print('getter'); return 'notCallable'; }
});

// Main test: ensure that observable side effect order is correct:
// 1. base.func lookup
// 2. argument evaluation
// 3. call handling -> callability test and error throw
//
// Internally:
// 1. GETPROPC does base.func lookup, notices value is not callable, replaces
//    call target (at idx_func) with an error object to be thrown later.  Error
//    object is tagged "to be thrown" using a hidden Symbol.
// 2. Argument evaluation normally.
// 3. Call handling normally, detects target (error object) is not callable.
//    Error handling path checks for hidden Symbol safely (without side effects)
//    and throws error created in step 1.

try {
    base.func(print('foo'), print('bar'), print('quux'));
} catch (e) {
    print(e);
}

// Call handling must not confuse errors created by GETPROPC and user code.
// If user adds that specific hidden Symbol (reserved to Duktape) to a call
// target (error or any other object) it will be thrown as is instead of a
// normal call error being thrown.  This is weird by memory safe.
try {
    new RangeError('aiee')();
} catch (e) {
    print(e);
}
