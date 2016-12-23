/*@include util-buffer.js@*/

/*===
json test
{"0":97,"1":98,"2":99,"3":100,"4":101,"5":102,"6":103,"7":104,"8":105,"9":106,"10":107,"11":108,"12":109,"13":110,"14":111,"15":112}
|6162636465666768696a6b6c6d6e6f70|
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
{bufferLength:16,plain:true,data:"abcdefghijklmnop"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
{"0":97,"1":98,"2":99,"3":100,"4":101,"5":102,"6":103,"7":104,"8":105,"9":106,"10":107,"11":108,"12":109,"13":110,"14":111,"15":112}
|6162636465666768696a6b6c6d6e6f70|
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
{bufferLength:16,plain:true,data:"abcdefghijklmnop"}
{"bufferLength":16,"plain":true,"data":"abcdefghijklmnop"}
===*/

function jsonTest() {
    var pb = createPlainBuffer('abcdefghijklmnop');
    function id(k,v) { return v; }

    function doTest(repl) {
        // JSON, JX, and JC
        print(JSON.stringify(pb, repl));
        print(Duktape.enc('jx', pb, repl));
        print(Duktape.enc('jc', pb, repl));

        // .toJSON() works
        Uint8Array.prototype.toJSON = function (k) {
            'use strict';  // must be strict to avoid object coercion for 'this'
            return {
                bufferLength: this.length,
                plain: isPlainBuffer(this),
                data: bufferToStringRaw(this)
            };
        };
        print(JSON.stringify(pb, repl));
        print(Duktape.enc('jx', pb, repl));
        print(Duktape.enc('jc', pb, repl));

        delete Uint8Array.prototype.toJSON;
    }

    doTest();
    doTest(id);  // force slow path
}

try {
    print('json test');
    jsonTest();
} catch (e) {
    print(e.stack || e);
}
