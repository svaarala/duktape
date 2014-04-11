/*
 *  Duktape builtin.
 */

/*---
{
    "custom": true
}
---*/

/*===
global
Duktape object true false true false false
print function true false true false false
alert function true false true false false
Duktape
env string true false true false false
version number false false false false false
fin function true false true false false
enc function true false true false false
dec function true false true false false
info function true false true false false
gc function true false true false false
Buffer function true false true false false
Pointer function true false true false false
Thread function true false true false false
Buffer
prototype object false false false false false
toString function true false true false false
valueOf function true false true false false
Pointer
prototype object false false false false false
toString function true false true false false
valueOf function true false true false false
Thread
prototype object false false false false false
resume function true false true false false
yield function true false true false false
current function true false true false false
===*/

function propsTest() {
    function printraw(obj, name) {
        var t = typeof obj[name];
        var d = Object.getOwnPropertyDescriptor(obj, name) || {};
        print(name, t, d.writable, d.enumerable, d.configurable, 'set' in d, 'get' in d );
    }

    function fglob(name) {
        printraw(this, name);
    }
    function fduk(name) {
        printraw(Duktape, name);
    }
    function fbuf(name) {
        printraw(Duktape.Buffer, name);
    }
    function fbufp(name) {
        printraw(Duktape.Buffer.prototype, name);
    }
    function fptr(name) {
        printraw(Duktape.Pointer, name);
    }
    function fptrp(name) {
        printraw(Duktape.Pointer.prototype, name);
    }
    function fthr(name) {
        printraw(Duktape.Thread, name);
    }
    function fthrp(name) {
        printraw(Duktape.Thread.prototype, name);
    }

    print('global');
    fglob('Duktape');
    fglob('print');
    fglob('alert');

    print('Duktape');
    fduk('env');
    fduk('version');
    fduk('fin');
    fduk('enc');
    fduk('dec');
    fduk('info');
    fduk('gc');
    fduk('Buffer');
    fduk('Pointer');
    fduk('Thread');

    print('Buffer');
    fbuf('prototype');
    fbufp('toString');
    fbufp('valueOf');

    print('Pointer');
    fptr('prototype');
    fptrp('toString');
    fptrp('valueOf');

    print('Thread');
    fthr('prototype');
    fthr('resume');
    fthr('yield');
    fthr('current');
}

try {
    propsTest();
} catch (e) {
    print(e);
}

/*===
encdec
string
666f6f
string
666f6fe188b4
buffer
102 111 111
buffer
102 111 111 4660
string
Zm9v
string
Zm9v4Yi0
buffer
102 111 111
buffer
102 111 111 4660
===*/

function printEnc(x) {
    print(typeof x);
    print(x);
}

function printDec(x) {
    print(typeof x);
    x = String(x);
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
}

print('encdec');

try {
    encDecTest();
} catch (e) {
    print(e);
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
    print(e);
}
