/*
 *  Init code executed for new global environments.
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

    // Logger object for C code provided by init code now.
    if (true) {
        def(D.Logger, 'clog', new D.Logger('C'));
    }

    // Tracking table for CommonJS module loading.
    if (true) {
        def(D, 'modLoaded', {});
    }
})(this, Duktape);
