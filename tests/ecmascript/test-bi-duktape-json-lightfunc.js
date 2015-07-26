/*
 *  JSON, JX, and JC serialization of lightfuncs.
 */

/*---
{
    "custom": true
}
---*/

/*===
undefined undefined
string [1,2,3,null,4,5,6]
string {"foo":123}
string {_func:true}
string [1,2,3,{_func:true},4,5,6]
string {foo:123,bar:{_func:true}}
string {"_func":true}
string [1,2,3,{"_func":true},4,5,6]
string {"foo":123,"bar":{"_func":true}}
===*/

function test() {
    // Note: rely on Math.cos being a lightfunc
    var lf = Math.cos;

    function json(x) {
        var res = JSON.stringify(x);
        print(typeof res, res);
    }

    function jx(x) {
        var res = Duktape.enc('jx', x);
        print(typeof res, res);
    }

    function jc(x) {
        var res = Duktape.enc('jc', x);
        print(typeof res, res);
    }

    json(lf);
    json([ 1, 2, 3, lf, 4, 5, 6 ]);
    json({ foo: 123, bar: lf });

    jx(lf);
    jx([ 1, 2, 3, lf, 4, 5, 6 ]);
    jx({ foo: 123, bar: lf });

    jc(lf);
    jc([ 1, 2, 3, lf, 4, 5, 6 ]);
    jc({ foo: 123, bar: lf });
}

try {
    test();
} catch (e) {
    print(e);
}
