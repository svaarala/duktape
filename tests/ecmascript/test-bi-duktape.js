/*
 *  Duktape builtin.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
global
Duktape object wc
Duktape
Duktape.version number none
Duktape.Pointer function wc
Duktape.Thread function wc
Duktape.info function wc
Duktape.act function wc
Duktape.gc function wc
Duktape.fin function wc
Duktape.enc function wc
Duktape.dec function wc
Duktape.compact function wc
Duktape.env string wc
Duktape.modLoaded object wc
Duktape.Pointer.name string c
Duktape.Pointer.length number c
Duktape.Pointer.prototype object none
Duktape.Pointer.prototype.constructor function wc
Duktape.Pointer.prototype.toString function wc
Duktape.Pointer.prototype.valueOf function wc
Duktape.Pointer.prototype.toString.length number c
Duktape.Pointer.prototype.toString.name string c
Duktape.Pointer.prototype.valueOf.length number c
Duktape.Pointer.prototype.valueOf.name string c
Duktape.Thread.name string c
Duktape.Thread.length number c
Duktape.Thread.prototype object none
Duktape.Thread.yield function wc
Duktape.Thread.resume function wc
Duktape.Thread.current function wc
Duktape.Thread.prototype.constructor function wc
Duktape.Thread.yield.length number c
Duktape.Thread.yield.name string c
Duktape.Thread.resume.length number c
Duktape.Thread.resume.name string c
Duktape.Thread.current.length number c
Duktape.Thread.current.name string c
Duktape.info.length number c
Duktape.info.name string c
Duktape.act.length number c
Duktape.act.name string c
Duktape.gc.length number c
Duktape.gc.name string c
Duktape.fin.length number c
Duktape.fin.name string c
Duktape.enc.length number c
Duktape.enc.name string c
Duktape.dec.length number c
Duktape.dec.name string c
Duktape.compact.length number c
Duktape.compact.name string c
===*/

function propsTest() {
    function printraw(obj, name, printname) {
        var t = typeof obj[name];
        var d = Object.getOwnPropertyDescriptor(obj, name) || {};
        var tmp = [];
        tmp.push('writable' in d ? (d.writable ? "w" : "") : '');
        tmp.push('enumerable' in d ? (d.enumerable ? "e" : "") : '');
        tmp.push('configurable' in d ? (d.configurable ? "c" : "") : '');
        tmp = [ tmp.join('') || 'none' ];
        if ('set' in d) { tmp.push('has-set'); }
        if ('get' in d) { tmp.push('has-get'); }
        print(printname, t, tmp.join(' '));
    }

    function printall(obj, printname) {
        var visited = [];

        function rec(obj, printname) {
            visited.push(obj);
            var ownprops = Object.getOwnPropertyNames(obj);  // keep enum order

            // Skip Logger which is no longer a default built-in in Duktape 2.x.
            ownprops = ownprops.filter(function (pname) { return pname !== 'Logger'; });

            ownprops.forEach(function (pname) {
                printraw(obj, pname, printname + '.' + pname);
            });
            ownprops.forEach(function (pname) {
                if ((typeof obj[pname] === 'object' || typeof obj[pname] === 'function') &&
                    visited.indexOf(obj[pname]) < 0) {
                    rec(obj[pname], printname + '.' + pname);
                }
            });
        }
        return rec(obj, printname);
    }

    print('global');
    printraw(this, 'Duktape', 'Duktape');
    // print and alert were removed in Duktape 2.x.

    print('Duktape');
    printall(Duktape, 'Duktape');
}

try {
    propsTest();
} catch (e) {
    print(e.stack || e);
}

/*===
encdec
string
666f6f
string
666f6fe188b4
object
102 111 111
object
102 111 111 4660
string
Zm9v
string
Zm9v4Yi0
object
102 111 111
object
102 111 111 4660
string
Zm9v
object
102 111 111
string
Zm9v
object
102 111 111
string
Zm9v
object
102 111 111
===*/

function printEnc(x) {
    print(typeof x);
    print(x);
}

function printDec(x) {
    print(typeof x);
    x = bufferToStringRaw(x);
    var res = [];
    for (var i = 0; i < x.length; i++) {
        res.push(x.charCodeAt(i));
    }
    print(res.join(' '));
}

function encDecTest() {
    printEnc(Duktape.enc('hex', 'foo'));
    printEnc(Duktape.enc('hex', 'foo\u1234'));

    printDec(Duktape.dec('hex', '666f6f'));
    printDec(Duktape.dec('hex', '666f6fe188b4'));

    printEnc(Duktape.enc('base64', 'foo'));
    printEnc(Duktape.enc('base64', 'foo\u1234'));

    printDec(Duktape.dec('base64', 'Zm9v'));
    printDec(Duktape.dec('base64', 'Zm9v4Yi0'));

    // Plain buffer input
    var pb = createPlainBuffer(3);
    pb[0] = 'f'.charCodeAt(0);
    pb[1] = 'o'.charCodeAt(0);
    pb[2] = 'o'.charCodeAt(0);
    printEnc(Duktape.enc('base64', pb));
    var pb = createPlainBuffer(4);
    pb[0] = 'Z'.charCodeAt(0);
    pb[1] = 'm'.charCodeAt(0);
    pb[2] = '9'.charCodeAt(0);
    pb[3] = 'v'.charCodeAt(0);
    printDec(Duktape.dec('base64', pb));

    // ArrayBuffer input
    var ab = new ArrayBuffer(3);
    var u8 = new Uint8Array(ab);
    u8[0] = 'f'.charCodeAt(0);
    u8[1] = 'o'.charCodeAt(0);
    u8[2] = 'o'.charCodeAt(0);
    printEnc(Duktape.enc('base64', ab));
    var ab = new ArrayBuffer(4);
    var u8 = new Uint8Array(ab);
    u8[0] = 'Z'.charCodeAt(0);
    u8[1] = 'm'.charCodeAt(0);
    u8[2] = '9'.charCodeAt(0);
    u8[3] = 'v'.charCodeAt(0);
    printDec(Duktape.dec('base64', ab));

    // Uint8Array slice input
    var ab = new ArrayBuffer(6);
    var u8 = new Uint8Array(ab);
    u8[0] = '!'.charCodeAt(0);
    u8[1] = '!'.charCodeAt(0);
    u8[2] = 'f'.charCodeAt(0);
    u8[3] = 'o'.charCodeAt(0);
    u8[4] = 'o'.charCodeAt(0);
    u8[5] = '!'.charCodeAt(0);
    printEnc(Duktape.enc('base64', new Uint8Array(ab).subarray(2, 5)));
    var ab = new ArrayBuffer(7);
    var u8 = new Uint8Array(ab);
    u8[0] = '_'.charCodeAt(0);
    u8[1] = '_'.charCodeAt(0);
    u8[2] = 'Z'.charCodeAt(0);
    u8[3] = 'm'.charCodeAt(0);
    u8[4] = '9'.charCodeAt(0);
    u8[5] = 'v'.charCodeAt(0);
    u8[6] = '_'.charCodeAt(0);
    printDec(Duktape.dec('base64', new Uint8Array(ab).subarray(2, 6)));
}

print('encdec');

try {
    encDecTest();
} catch (e) {
    print(e.stack || e);
}

/*===
finalizer
undefined
undefined
function
true
undefined
undefined
===*/

/* Just test that we can set and get a finalizer. */

function finalizerTest() {
    var obj = {};
    function f(v) { print(typeof v); }
    function fin(x) { print('finalizer'); }
    f(Duktape.fin(obj));
    f(Duktape.fin(obj, fin));
    f(Duktape.fin(obj));
    print(Duktape.fin(obj) === fin);
    f(Duktape.fin(obj, undefined));
    f(Duktape.fin(obj));
}

print('finalizer');

try {
    finalizerTest();
} catch (e) {
    print(e.stack || e);
}
