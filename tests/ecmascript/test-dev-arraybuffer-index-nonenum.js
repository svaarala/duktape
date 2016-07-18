/*
 *  ArrayBuffer virtual index properties are non-standard and to avoid being
 *  serialized they're non-enumerable.  Typed array and Node.js buffer indices
 *  are enumerable.
 */

/*===
-1 undefined
0 {value:0,writable:true,enumerable:false,configurable:false}
1 {value:0,writable:true,enumerable:false,configurable:false}
2 {value:0,writable:true,enumerable:false,configurable:false}
3 {value:0,writable:true,enumerable:false,configurable:false}
4 undefined
0
1
2
3
-1 undefined
0 {value:65,writable:true,enumerable:true,configurable:false}
1 {value:66,writable:true,enumerable:true,configurable:false}
2 {value:67,writable:true,enumerable:true,configurable:false}
3 {value:68,writable:true,enumerable:true,configurable:false}
4 undefined
0
1
2
3
-1 undefined
0 {value:0,writable:true,enumerable:true,configurable:false}
1 {value:0,writable:true,enumerable:true,configurable:false}
2 {value:0,writable:true,enumerable:true,configurable:false}
3 {value:0,writable:true,enumerable:true,configurable:false}
4 undefined
===*/

function test(buf) {
    var i, k;

    for (k in buf) {
        print(k);
    }
    for (i = -1; i <= 4; i++) {
        print(i, Duktape.enc('jx', Object.getOwnPropertyDescriptor(buf, i)));
    }
}

try {
    test(new ArrayBuffer(4));
    test(new Buffer('ABCD'));
    test(new Uint8Array(4));
} catch (e) {
    print(e.stack || e);
}
