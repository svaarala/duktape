/*
 *  __duk__ builtin.
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
__duk__
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
        print(typeof __duk__[name]);
    }
    function fbuf(name) {
        print(typeof __duk__.Buffer[name]);
    }
    function fbufp(name) {
        print(typeof __duk__.Buffer.prototype[name]);
    }
    function fptr(name) {
        print(typeof __duk__.Pointer[name]);
    }
    function fptrp(name) {
        print(typeof __duk__.Pointer.prototype[name]);
    }
    function fthr(name) {
        print(typeof __duk__.Thread[name]);
    }
    function fthrp(name) {
        print(typeof __duk__.Thread.prototype[name]);
    }

    print('global');
    fglob('__duk__');
    fglob('print');
    fglob('alert');

    print('__duk__');
    fduk('build');
    fduk('version');
    fduk('setFinalizer');
    fduk('getFinalizer');
    fduk('enc');
    fduk('dec');
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
    printEnc(__duk__.enc('hex', 'foo'));
    printEnc(__duk__.enc('hex', 'foo\u1234'));

    printDec(__duk__.dec('hex', '666f6f'));
    printDec(__duk__.dec('hex', '666f6fe188b4'));

    printEnc(__duk__.enc('base64', 'foo'));
    printEnc(__duk__.enc('base64', 'foo\u1234'));

    printDec(__duk__.dec('base64', 'Zm9v'));
    printDec(__duk__.dec('base64', 'Zm9v4Yi0'));
}

print('encdec');

try {
    encDecTest();
} catch (e) {
    print(e);
}
