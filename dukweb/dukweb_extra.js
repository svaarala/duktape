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

Duktape.initializedPromise = new Promise(function (resolve, reject) {
    Duktape.initializedPromiseFuncs = [ resolve, reject ];
});

/*
 *  Utilities
 */

Duktape.log = function log() {};
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

Duktape.logOwnProperties = function logOwnProperties() {
    Duktape.log('Own properties of Duktape:');
    Object.getOwnPropertyNames(Duktape).forEach(function (k) {
        var v = Duktape[k];
        if (typeof v === 'function') { v = '[omitted]'; }
        Duktape.log('    ' + k + ' = (' + typeof Duktape[k] + ') ' + String(v));
    });
};

Duktape.hexVal = function hexVal(x, ndigits) {
    var nybbles = '0123456789abcdef';
    var i;
    var res = ''
    for (i = 0; i < ndigits; i++) {
        res += nybbles[x & 0x0f];
        x >>>= 4;
    }
    return res;
};

Duktape.escapeTable = (function escapeTableInit() {
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
Duktape.escapeString = function escapeString(x) {
    var res = [];
    var i, n = x.length;
    var c;
    var esc = Duktape.escapeTable;
    for (i = 0; i < n; i++) {
        c = x.charAt(i);
        res.push(esc[c] || '\\u' + Duktape.hexVal(String.charCodeAt(i)));
    }
    return res.join('');
};

/*
 *  Duktape.eval: run code inside Duktape, encode output value using JSON.
 */

// XXX: errors should probably be promoted to work better
Duktape.eval = function eval(code) {
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
 *  Handlers for Duktape's print() and alert() replacements.
 */

// Expect web page to override this, e.g. to append to some buffer.
Duktape.printHandler = function printHandler(msg) {
    log(msg);
};

// Expect web page to override this, e.g. to use browser alert().
Duktape.alertHandler = function alertHandler(msg) {
    log(msg);
};

// Get Promise for Dukweb init completion.
Duktape.getInitializedPromise = function getInitializedPromise() {
    return Duktape.initializedPromise;
};

// Asynchronous init, triggered from C main().
Duktape.initializeRaw = function initializeRaw() {
    console.log('Duktape.initialize() start');

    /*
     *  Raw C function bindings.
     *
     *  The dukweb_eval() binding is a very raw binding which provides an
     *  interface to eval one string, and to get one string output (ToString
     *  coerced result or error).
     */

    Duktape.dukweb_is_open = Module.cwrap('dukweb_is_open', 'number', [ ]);
    Duktape.dukweb_open = Module.cwrap('dukweb_open', null, [ ]);
    Duktape.dukweb_close = Module.cwrap('dukweb_close', null, [ ]);
    Duktape.dukweb_eval = Module.cwrap('dukweb_eval', 'string', [ 'string' ]);
    Duktape.logOwnProperties();

    /*
     *  Initialize Duktape heap automatically (not closed for now), and use
     *  Duktape.eval() to pull in some convenience properties like Duktape
     *  version.
     */

    Duktape.dukweb_open();
    Duktape.version = Duktape.eval('Duktape.version');
    Duktape.env = Duktape.eval('Duktape.env');
    Duktape.logOwnProperties();

    /*
     *  Initialize a 'Dukweb' instance inside Duktape for interfacing with the
     *  browser side.
     */

    // Minimal console object.
    var DUKWEB_CONSOLE_INIT =
        'Dukweb.console = (function () {\n' +
        '    var useProxyWrapper = true;\n' +
        '    var c = {};\n' +
        '    function console_log(args, errName) {\n' +
        '        var msg = Array.prototype.map.call(args, function (v) {\n' +
        '            if (typeof v === "object" && v !== null) { return console.format(v); };\n' +
        '            return v;\n' +
        '        }).join(" ");\n' +
        '        if (errName) {\n' +
        '            var err = new Error(msg);\n' +
        '            err.name = "Trace";\n' +
        '            Dukweb.print(err.stack || err);\n' +
        '        } else {\n' +
        '            Dukweb.print(msg);\n' +
        '        }\n' +
        '    };\n' +
        '    c.format = function format(v) { try { return Duktape.enc("jx", v); } catch (e) { return String(v); } };\n' +
        '    c.assert = function assert(v) {\n' +
        '        if (arguments[0]) { return; }\n' +
        '        console_log(arguments.length > 1 ? Array.prototype.slice.call(arguments, 1) : [ "false == true" ], "AssertionError");\n' +
        '    };\n'+
        '    c.log = function log() { console_log(arguments, null); };\n' +
        '    c.debug = function debug() { console_log(arguments, null); };\n' +
        '    c.trace = function trace() { console_log(arguments, "Trace"); };\n' +
        '    c.info = function info() { console_log(arguments, null); };\n' +
        '    c.warn = function warn() { console_log(arguments, null); };\n' +
        '    c.error = function error() { console_log(arguments, "Error"); };\n' +
        '    c.exception = function exception() { console_log(arguments, "Error"); };\n' +
        '    c.dir = function dir() { console_log(arguments, null); };\n' +
        '    if (typeof Proxy === "function" && useProxyWrapper) {\n' +
        '        var orig = c;\n' +
        '        var dummy = function () {};\n' +
        '        c = new Proxy(orig, {\n' +
        '            get: function (targ, key, recv) {\n' +
        '                var v = targ[key];\n' +
        '                return typeof v === "function" ? v : dummy;\n' +
        '            }\n' +
        '        });\n' +
        '    }\n' +
        '    return c;\n' +
        '})();';

    Duktape.eval('Dukweb = {};');
    Duktape.eval('Dukweb.userAgent = ' + (JSON.stringify(navigator.userAgent.toString()) || '"unknown"') + ';');
    Duktape.eval('Dukweb.emscripten_run_script = this.emscripten_run_script; delete this.emscripten_run_script;');
    Duktape.eval('Dukweb.eval = Dukweb.emscripten_run_script;')  // XXX: better binding
    Duktape.eval('Dukweb.print = function() { Dukweb.eval("Duktape.printHandler(" + JSON.stringify(Array.prototype.join.call(arguments, " ")) + ")") };');
    Duktape.eval('Dukweb.alert = function() { Dukweb.eval("Duktape.alertHandler(" + JSON.stringify(Array.prototype.join.call(arguments, " ")) + ")") };');
    Duktape.eval(DUKWEB_CONSOLE_INIT);
    Duktape.eval('print = Dukweb.print;');
    Duktape.eval('alert = Dukweb.print;');  // intentionally bound to print()
    Duktape.eval('console = Dukweb.console;');

    Duktape.initSuccess = !!Duktape.dukweb_is_open();
    //console.log('=== ' + Duktape.eval('Duktape.enc("jx", { env: Duktape.env, version: Duktape.version })') + ' ===');

    console.log('Duktape.initialize() end');
};

Duktape.initialize = function initialize() {
    try {
        Duktape.initializeRaw();
        Duktape.initializedPromiseFuncs[0](Duktape);
    } catch (e) {
        Duktape.initializedPromiseFuncs[1](e);
    } finally {
        Duktape.initializedPromiseFuncs[1](new Error('internal error'));
    }
};
