/*---
custom: true
---*/

/*===
obj created
before
alter prototype chain
call gc()
return void 0 (no trap)
root finalized
123
after
null
done
===*/

var obj;

function getter() {
    print('alter prototype chain');
    Object.setPrototypeOf(obj, null);
    obj = null;

    print('call gc()');
    if (typeof Duktape === 'object' && Duktape !== null) { Duktape.gc(); Duktape.gc(); }

    print('return void 0 (no trap)');
    return void 0;
}

function rootFinalizer(root) {
    print('root finalized');
}

function create() {
    var root = { 100: 123 };
    var handler = {};
    var proxy = new Proxy(root, handler);
    var obj = Object.create(proxy);
    Object.defineProperty(handler, 'get', { get: getter });
    if (typeof Duktape === 'object' && Duktape !== null) { Duktape.fin(root, rootFinalizer); }

    root = null;
    handler = null;
    proxy = null;
    if (typeof Duktape === 'object' && Duktape !== null) { Duktape.gc(); Duktape.gc(); }

    print('obj created');
    return obj;
}

function test() {
    obj = create();
    if (typeof Duktape === 'object' && Duktape !== null) { Duktape.gc(); Duktape.gc(); }

    // Now root, proxy, handler are only reachable via 'obj'.

    print('before');
    print(obj[100]);
    print('after');
    print(obj);
}

test();

print('done');
