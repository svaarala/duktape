/*
 *  Init code for legacy compatibility.
 *
 *  Compatibility properties / wrapper functions here allow Duktape to remain
 *  compatible for user code when core features are changed, without burdening
 *  the main C code with compatibility stuff.
 *
 *  This file is minified with UglifyJS or the closure compiler.  Both will
 *  rename variables, remove comments, and are clever enough to drop any
 *  "if (false) { ... }" blocks altogether, so that's an effective way to
 *  disable currently unneeded code.
 */

(function(G, D) {
    'use strict';

    function def(object, name, value) {
        Object.defineProperty(object, name, {
            value: value,
            writable: true,
            enumerable: false,
            configurable: true
        });
    }

    function defD(name, value) {
        def(D, name, value);
    }

    // Compatibility for 'console.log'.
    if (false) {
        console = {
            log: function() {
                print(Array.prototype.join.call(arguments, ' '));
            }
        };
    }

    // Duktape.line() was removed in Duktape 0.11.0, here's an example
    // replacement user code could use.
    if (false) {
        def(D, 'line', function () {
            'use duk notail';

            /* Tail calls are prevented to ensure calling activation exists.
             * Call stack indices: -1 = Duktape.act, -2 = getCurrentLine, -3 = caller
             */

            return (Duktape.act(-3) || {}).lineNumber;
        });
    }

    // Logger object for C code provided by init code now.
    if (true) {
        def(D.Logger, 'clog', new D.Logger('C'));
    }

    // Tracking table for CommonJS module loading.
    if (true) {
        def(D, 'modLoaded', {});
    }
})(this, Duktape);
