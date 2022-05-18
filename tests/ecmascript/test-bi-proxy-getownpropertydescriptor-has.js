// Object.hasOwnProperty() uses [[GetOwnProperty]] which invokes the
// 'getOwnPropertyDescriptor' trap rather than 'has'.

/*===
handler.getOwnPropertyDescriptor true true string foo
true
handler.getOwnPropertyDescriptor true true string bar
false
handler.getOwnPropertyDescriptor true true string 123
false
handler.getOwnPropertyDescriptor true true string 123
false
===*/

function test() {
    var target = {};
    var handler = {
        has: function (targ, key) {
            print('handler.has', this === handler, targ === target, typeof key, key);
            if (key === 'foo') {
                return true;
            }
        },
        getOwnPropertyDescriptor: function (targ, key) {
            print('handler.getOwnPropertyDescriptor', this === handler, targ === target, typeof key, key);
            if (key === 'foo') {
                return { value: 'bar', writable: true, enumerable: true, configurable: true };
            }
        }
    };
    var P = new Proxy(target, handler);

    print(P.hasOwnProperty('foo'));
    print(P.hasOwnProperty('bar'));
    print(P.hasOwnProperty(123));
    print(P.hasOwnProperty('123'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
