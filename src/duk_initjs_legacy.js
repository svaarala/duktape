/*
 *  Init code for legacy compatibility.
 *
 *  Compatibility properties / wrapper functions here allow Duktape to remain
 *  compatible for user code when core features are changed, without burdening
 *  the main C code with compatibility stuff.
 */

(function(D) {
    function def(name, value) {
        Object.defineProperty(D, name, {
            value: value,
            writable: true,
            enumerable: false,
            configurable: true
        });
    }
    def('build', '');                 // removed in Duktape 0.9.0
    def('setFinalizer', D.setFin);    // renamed in Duktape 0.9.0
    def('getFinalizer', D.getFin);    // renamed in Duktape 0.9.0
})(Duktape);
