/*
 *  RegExp.prototype.compile() polyfill, described in E6 Annex B:
 *
 *    https://people.mozilla.org/~jorendorff/es6-draft.html#sec-regexp.prototype.compile
 *
 *  See also:
 *
 *    http://mozilla.6506.n7.nabble.com/RegExp-prototype-compile-and-RegExp-instance-properties-td270408.html
 *
 *  This polyfill cannot be implemented in terms of standard E5 because it
 *  needs to reinitialize the internal state of a RegExp instance.  To do
 *  that, we access the Duktape internal properties directly, which is
 *  quite fragile.
 *
 *  Avoid storing a public copy of Duktape or some internal property name in
 *  the global object.  This could subvert user sandboxing.
 */

(function () {
    var propBytecode = Duktape.dec('hex', 'ff62797465636f6465');  // \xFFbytecode

    if (typeof RegExp.prototype.compile !== 'undefined') {
        return;
    }

    Object.defineProperty(RegExp.prototype, 'compile', {
        value: function (pattern, flags) {
            var newBytecode, tmpRegexp;
            if (typeof this !== 'object' || !(propBytecode in this)) {
                throw new TypeError('invalid this binding');
            }

            // FIXME: property attributes prevent this approach from working
            // right now.  The properties we'd need to modify are non-writable
            // and non-configurable:
            //
            //     \xffbytecode
            //     source
            //     global
            //     ignoreCase
            //     multiline
            //
            // The property attributes can be relaxed, or the properties can
            // be made accessors backing to the regexp bytecode, see Ditz
            // issues: 0f2c246cadbb3b3913b75dc7e890ee4e7d336a1a and
            // f8396fbcc36db4610fec5ad5a7d7a8f471d084a4.

            if (typeof pattern === 'object' && (propBytecode in pattern)) {
                this[propBytecode] = pattern[propBytecode];
            } else {
                tmpRegexp = new RegExp(pattern, flags);
                this[propBytecode] = tmpRegexp[propBytecode];
            }

            //this.source
            //this.global
            //this.ignoreCase
            //this.multiline
            this.lastIndex = 0;
            return this;
        }, writable: true, enumerable: false, configurable: true
    });

    throw new Error('RegExp.prototype.compile() polyfill incomplete');
})();
