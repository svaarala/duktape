if (typeof Object.prototype.__defineGetter__ === 'undefined') {
    Object.prototype.__defineGetter__ = function (n, f) {
        Object.defineProperty(this, n, { enumerable: true, configurable: true, get: f });
    };
}
