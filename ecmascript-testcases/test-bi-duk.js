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
object
function
function
Duktape
string
number
function
function
function
function
function
function
function
function
function
function
function
function
function
function
Buffer
object
function
function
Pointer
object
function
function
Thread
object
function
function
function
===*/

function propsTest() {
    function fglob(name) {
        print(typeof this[name]);
    }
    function fduk(name) {
        print(typeof Duktape[name]);
    }
    function fbuf(name) {
        print(typeof Duktape.Buffer[name]);
    }
    function fbufp(name) {
        print(typeof Duktape.Buffer.prototype[name]);
    }
    function fptr(name) {
        print(typeof Duktape.Pointer[name]);
    }
    function fptrp(name) {
        print(typeof Duktape.Pointer.prototype[name]);
    }
    function fthr(name) {
        print(typeof Duktape.Thread[name]);
    }
    function fthrp(name) {
        print(typeof Duktape.Thread.prototype[name]);
    }

    print('global');
    fglob('Duktape');
    fglob('print');
    fglob('alert');

    print('Duktape');
    fduk('env');
    fduk('version');
    fduk('setFin');
    fduk('getFin');
    fduk('enc');
    fduk('dec');
    fduk('jxEnc');
    fduk('jxDec');
    fduk('jcEnc');
    fduk('jcDec');
    fduk('addr');
    fduk('refc');
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
