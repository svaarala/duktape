/*===
123
===*/

var curr = { foo: 123 };
for (var i = 0; i < 1e6; i++) {
    curr = new Proxy(curr, {});
}

var finalProxy = curr;
print(finalProxy.foo);
