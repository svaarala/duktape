/*
 *  https://github.com/svaarala/duktape/issues/2327
 */

/*---
{
    "custom": true
}
---*/

/*===
RangeError
done
===*/

function main() {
    var v4 = [13.37,13.37,13.37,13.37,13.37];
    var v6 = [1337,1337];
    v6[1] = v6;
    var v7 = {e:"COVraKWiLf",constructor:v4,c:v6,d:13.37};
    var v8 = {d:"COVraKWiLf",e:v4,c:v4,toString:v7,valueOf:13.37,b:1337};
    var v9 = new Proxy(v8,v8);
    var v10 = CBOR.encode(v9);
}
try {
    main();
} catch (e) {
    print(e.name);
}
print('done');
