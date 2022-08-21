// arguments.caller was removed in ES2017 for strict arguments objects.
// arguments.callee remains a thrower for strict argument objects.

/*===
arguments.caller: undefined
TypeError
===*/

function testCaller() {
    'use strict';
    print('arguments.caller:', arguments.caller);
}

try {
    testCaller();
} catch (e) {
    print(e.stack || e);
}

function testCallee() {
    'use strict';
    print('arguments.callee:', arguments.callee);
}

try {
    testCallee();
} catch (e) {
    print(e.name);
}
