// Test how a revoked Proxy is summarized in error messages by
// duk_api_readable.c.

/*---
custom: true
---*/

/*===
TypeError cannot write property [object Object] of null
TypeError cannot write property [object RevokedProxy] of null
TypeError cannot write property [object Function] of null
TypeError cannot write property [object RevokedProxy] of null
===*/

var p = Proxy.revocable(Math, {});
try {
    null[p.proxy] = 123;
} catch (e) {
    print(e.name, e.message);
}
p.revoke();
try {
    null[p.proxy] = 123;
} catch (e) {
    print(e.name, e.message);
}

var p = Proxy.revocable(Math.cos, {});
try {
    null[p.proxy] = 123;
} catch (e) {
    print(e.name, e.message);
}
p.revoke();
try {
    null[p.proxy] = 123;
} catch (e) {
    print(e.name, e.message);
}
