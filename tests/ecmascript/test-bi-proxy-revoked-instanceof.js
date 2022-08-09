/*===
false
undefined
TypeError
===*/

var target = function test() {};
var P = Proxy.revocable(target, {});
try {
    print({} instanceof P.proxy);
} catch (e) {
    print(e.name);
    //print(e.stack);
}
print(P.revoke());
try {
    print({} instanceof P.proxy);
} catch (e) {
    print(e.name);
    //print(e.stack);
}
