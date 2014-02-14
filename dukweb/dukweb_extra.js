/*
 *  Dukweb glue appended to Emscripten-generated dukweb.js code.
 *
 *  Sets up a 'Duktape' object accessible from web page Javascript.  'Duktape'
 *  provides various glue functions, the most important being Duktape.eval()
 *  which evaluates a string inside Duktape and returns the result.  Currently
 *  Duktape.eval() is limited to JSON compatible results (other values are
 *  coerced in whichever way).
 *
 *  https://github.com/kripken/emscripten/wiki/Interacting-with-code
 */

var Duktape = {};

/*
 *  Utilities
 */

Duktape.log = function() {};
if (typeof console === 'object' && typeof console.log === 'function') {
    Duktape.log = function() {
        console.log.apply(console, arguments);
    }
} else if (typeof print === 'function') {
    Duktape.log = function() {
        print.apply(this, arguments);
    }
}
Duktape.log('Duktape (dukweb) initializing');

Duktape.logOwnProperties = function() {
    Duktape.log('Own properties of Duktape:');
    Object.getOwnPropertyNames(Duktape).forEach(function (k) {
        var v = Duktape[k];
        if (typeof v === 'function') { v = '[omitted]'; }
        Duktape.log('    ' + k + ' = (' + typeof Duktape[k] + ') ' + String(v));
    });
}

Duktape.hexVal = function(x, ndigits) {
    var nybbles = '0123456789abcdef';
    var i;
    var res = ''
    for (i = 0; i < ndigits; i++) {
        res += nybbles[x & 0x0f];
        x >>>= 4;
    }
    return res;
};

Duktape.escapeTable = (function() {
    var i, n;
    var res = {};

    for (i = 0; i < 256; i++) {
        if (i < 0x20 || i > 0x7e || i == 0x27 /*apos*/ || i == 0x22 /*quot*/ ||
            i == 0x5c /*backslash*/) {
            res[String.fromCharCode(i)] = '\\u' + Duktape.hexVal(i, 4);
        } else {
            res[String.fromCharCode(i)] = String.fromCharCode(i);
        }
    }
    return res;
})();

// Probably unnecessary, JSON.stringify() escapes strings nicely.
Duktape.escapeString = function(x) {
    var res = [];
    var i, n = x.length;
    var c;
    var esc = Duktape.escapeTable;
    for (i = 0; i < n; i++) {
        c = x.charAt(i);
        res.push(esc[c] || '\\u' + Duktape.hexVal(String.charCodeAt(i)));
    }
    return res.join('');
}

/*
 *  Raw C function bindings.
 *
 *  The dukweb_eval() binding is a very raw binding which provides an
 *  interface to eval one string, and to get one string output (ToString
 *  coerced result or error).
 */

// FIXME: not sure about the memory use here (leaks?), check
Duktape.dukweb_open = Module.cwrap('dukweb_open', 'void', [ 'void' ]);
Duktape.dukweb_close = Module.cwrap('dukweb_close', 'void', [ 'void' ]);
Duktape.dukweb_eval = Module.cwrap('dukweb_eval', 'string', [ 'string' ]);
Duktape.logOwnProperties();

/*
 *  Duktape.eval: run code inside Duktape, encode output value using JSON.
 */

Duktape.eval = function(code) {
    // Code escape into a Javascript string
    var escapedString = JSON.stringify(String(code));
    //var escapedString = '"' + Duktape.escapeString(code) + '"';

    // The raw eval result is first JSON encoded to make it a string.
    // For unsupported types JSON.stringify() might return e.g. 'undefined'.
    var res = Duktape.dukweb_eval(
        '(function() { var t; try { ' +
        't = eval(' + escapedString + ');' + 
        'return JSON.stringify(t) || "\\"undefined\\"";' +
        '} catch (e) { ' +
        'return "ERROR: " + String(e); ' +
        '} })();'
    );
    Duktape.log('raw eval result: ' + typeof(res) + ': ' + String(res));
    return JSON.parse(res);
};

/*
 *  Initialize Duktape heap automatically (not closed for now), and use
 *  Duktape.eval() to pull in some convenience properties like Duktape
 *  version.
 */

Duktape.dukweb_open();
Duktape.version = Duktape.eval('Duktape.version');
Duktape.env = Duktape.eval('Duktape.env');
Duktape.logOwnProperties();

//console.log('=== ' + Duktape.eval('Duktape.enc("jsonx", { env: Duktape.env, version: Duktape.version })') + ' ===');
