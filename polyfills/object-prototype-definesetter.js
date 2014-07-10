if (typeof Object.prototype.__defineSetter__ === 'undefined') {
    Object.prototype.__defineSetter__ = function (n, f) {
        Object.defineProperty(this, n, { enumerable: true, configurable: true, set: f });
    }
}
