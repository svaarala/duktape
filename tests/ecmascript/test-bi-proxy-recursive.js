/*===
recursive proxies
success
success
===*/

/* Duktape 3.x allows Proxy as both handler and target (Duktape 2.x did not
 * allow either).
 */

function proxyHandlerTest() {
    var target = { foo: 123 };
    var handler = new Proxy({}, {});
    var proxy = new Proxy(target, handler);
    print('success');
}

function proxyTargetTest() {
    var target = new Proxy({}, {});
    var handler = {};
    var proxy = new Proxy(target, handler);
    print('success');
}

print('recursive proxies');

try {
    proxyHandlerTest();
} catch (e) {
    print(e.name);
}

try {
    proxyTargetTest();
} catch (e) {
    print(e.name);
}
