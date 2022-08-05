/*===
ownKeys called true
enum: string quux
enum: string bar
enum: string foo
done
===*/

function test() {
    var O1 = { foo: 123 };
    var O2 = new Proxy(O1, {
        ownKeys: function (target) {
            print('ownKeys called', target === O1);
            return [ 'foo' ];
        }
    });
    var O3 = { bar: 234 };
    Object.setPrototypeOf(O3, O2);
    var O4 = { quux: 345 };
    Object.setPrototypeOf(O4, O3);

    for (var k in O4) {
        print('enum:', typeof k, String(k));
    }
    print('done');
}

test();
