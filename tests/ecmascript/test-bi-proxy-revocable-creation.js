/*===
[object Object]
true
[object Object]
true true true
[object Function]
true true true
undefined undefined undefined undefined
undefined undefined undefined undefined
===*/

var target = { foo: 123 };

var P = Proxy.revocable(target, {});
print(Object.prototype.toString.call(P));
print(Object.getPrototypeOf(P) === Object.prototype);

print(Object.prototype.toString.call(P.proxy));
var pd = Object.getOwnPropertyDescriptor(P, 'proxy');
print(pd.writable, pd.enumerable, pd.writable);

print(Object.prototype.toString.call(P.revoke));
var pd = Object.getOwnPropertyDescriptor(P, 'revoke');
print(pd.writable, pd.enumerable, pd.writable);

Object.getOwnPropertyNames(P).forEach(function (k) {
    if (k === 'proxy' || k === 'revoke') {
        return;
    }
    print('extra key:', k);
});
Object.getOwnPropertySymbols(P).forEach(function (s) {
    print('extra symbol:', String(s));
});

var pd = Object.getOwnPropertyDescriptor(P.revoke, 'name') || {};
print(pd.value, pd.writable, pd.enumerable, pd.writable);

var pd = Object.getOwnPropertyDescriptor(P.revoke, 'length') || {};
print(pd.value, pd.writable, pd.enumerable, pd.writable);
