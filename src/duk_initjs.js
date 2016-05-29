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

    // XXX: at the moment there's no init stuff; remove?
})(this, Duktape);
