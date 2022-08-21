/*===
[object Function]
[object Math]
[object Math]
TypeError
===*/

print(Object.prototype.toString.call(new Proxy(Math.cos,{})));
print(Object.prototype.toString.call(new Proxy(Math,{})));

var P = Proxy.revocable(Math, {});

print(Object.prototype.toString.call(P.proxy));
P.revoke();
try {
    print(Object.prototype.toString.call(P.proxy));
} catch (e) {
    print(e.name);
}
