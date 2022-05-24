/*===
true
undefined
TypeError
done
===*/

var target = [];
var P = Proxy.revocable(target, {});
try {
    print(Array.isArray(P.proxy));
} catch (e) {
    print(e.name);
    //print(e.stack);
}
print(P.revoke());
try {
    print(Array.isArray(P.proxy));
} catch (e) {
    print(e.name);
    //print(e.stack);
}
print('done');
