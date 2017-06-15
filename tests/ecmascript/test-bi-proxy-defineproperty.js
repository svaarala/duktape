/*
 *  Proxy (ES2015) 'defineProperty'.
 */

/*@include util-object.js@*/

/*===
value=123,writable=false,enumerable=false,configurable=true
get=function,set=function,enumerable=false,configurable=true
===*/

function passThroughTest() {
    var T = {};
    var P = new Proxy(T, {});
    var pd;

    Object.defineProperty(P, 'foo', {
        value: 123,
        writable: false,
        enumerable: false,
        configurable: true
    });
    Object.defineProperty(P, 'bar', {
        get: function getter() {},
        set: function setter() {},
        enumerable: false,
        configurable: true
    });

    pd = Object.getOwnPropertyDescriptor(T, 'foo');
    printPropDesc(pd);
    pd = Object.getOwnPropertyDescriptor(T, 'bar');
    printPropDesc(pd);
}

try {
    passThroughTest();
} catch (e) {
    print(e.stack || e);
}
