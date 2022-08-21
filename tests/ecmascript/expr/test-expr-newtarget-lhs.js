/*===
start
ReferenceError
===*/

function test() {
    new.target = 123;
}

// Currently causes a runtime ReferenceError.
// XXX: in Firefox the error is compile time.
try {
    print('start');
    test();
    print('done');
} catch (e) {
    print(e.name);
}
