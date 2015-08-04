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
Duktape object wc
print function wc
alert function wc
Duktape
Duktape.version number none
Duktape.Buffer function wc
Duktape.Pointer function wc
Duktape.Thread function wc
Duktape.Logger function wc
Duktape.info function wc
Duktape.act function wc
Duktape.gc function wc
Duktape.fin function wc
Duktape.enc function wc
Duktape.dec function wc
Duktape.compact function wc
Duktape.env string wc
Duktape.modLoaded object wc
Duktape.Buffer.name string none
Duktape.Buffer.length number none
Duktape.Buffer.prototype object none
Duktape.Buffer.prototype.constructor function wc
Duktape.Buffer.prototype.toString function wc
Duktape.Buffer.prototype.valueOf function wc
Duktape.Buffer.prototype.toString.length number none
Duktape.Buffer.prototype.toString.name string none
Duktape.Buffer.prototype.valueOf.length number none
Duktape.Buffer.prototype.valueOf.name string none
Duktape.Pointer.name string none
Duktape.Pointer.length number none
Duktape.Pointer.prototype object none
Duktape.Pointer.prototype.constructor function wc
Duktape.Pointer.prototype.toString function wc
Duktape.Pointer.prototype.valueOf function wc
Duktape.Pointer.prototype.toString.length number none
Duktape.Pointer.prototype.toString.name string none
Duktape.Pointer.prototype.valueOf.length number none
Duktape.Pointer.prototype.valueOf.name string none
Duktape.Thread.name string none
Duktape.Thread.length number none
Duktape.Thread.prototype object none
Duktape.Thread.yield function wc
Duktape.Thread.resume function wc
Duktape.Thread.current function wc
Duktape.Thread.prototype.constructor function wc
Duktape.Thread.yield.length number none
Duktape.Thread.yield.name string none
Duktape.Thread.resume.length number none
Duktape.Thread.resume.name string none
Duktape.Thread.current.length number none
Duktape.Thread.current.name string none
Duktape.Logger.name string none
Duktape.Logger.length number none
Duktape.Logger.prototype object none
Duktape.Logger.clog object wc
Duktape.Logger.prototype.constructor function wc
Duktape.Logger.prototype.l number w
Duktape.Logger.prototype.n string w
Duktape.Logger.prototype.fmt function wc
Duktape.Logger.prototype.raw function wc
Duktape.Logger.prototype.trace function wc
Duktape.Logger.prototype.debug function wc
Duktape.Logger.prototype.info function wc
Duktape.Logger.prototype.warn function wc
Duktape.Logger.prototype.error function wc
Duktape.Logger.prototype.fatal function wc
Duktape.Logger.prototype.fmt.length number none
Duktape.Logger.prototype.fmt.name string none
Duktape.Logger.prototype.raw.length number none
Duktape.Logger.prototype.raw.name string none
Duktape.Logger.prototype.trace.length number none
Duktape.Logger.prototype.trace.name string none
Duktape.Logger.prototype.debug.length number none
Duktape.Logger.prototype.debug.name string none
Duktape.Logger.prototype.info.length number none
Duktape.Logger.prototype.info.name string none
Duktape.Logger.prototype.warn.length number none
Duktape.Logger.prototype.warn.name string none
Duktape.Logger.prototype.error.length number none
Duktape.Logger.prototype.error.name string none
Duktape.Logger.prototype.fatal.length number none
Duktape.Logger.prototype.fatal.name string none
Duktape.Logger.clog.n string wec
Duktape.info.length number none
Duktape.info.name string none
Duktape.act.length number none
Duktape.act.name string none
Duktape.gc.length number none
Duktape.gc.name string none
Duktape.fin.length number none
Duktape.fin.name string none
Duktape.enc.length number none
Duktape.enc.name string none
Duktape.dec.length number none
Duktape.dec.name string none
Duktape.compact.length number none
Duktape.compact.name string none
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
    printraw(this, 'print', 'print');
    printraw(this, 'alert', 'alert');

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
