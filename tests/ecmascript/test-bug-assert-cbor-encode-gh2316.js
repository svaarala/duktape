// https://github.com/svaarala/duktape/issues/2316

/*---
custom: true
---*/

/*===
===*/

function main() {
    var v0 = [];
    var v4 = Duktape.dec("base64",v0);
    var v5 = CBOR.encode(v4);
}
main();
