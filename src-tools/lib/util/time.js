const getNowMillis = (function () {
    if (typeof performance === 'object' && performance !== null &&
        typeof performance.now === 'function') {
        return performance.now;
    } else {
        return Date.now;
    }
})();

exports.getNowMillis = getNowMillis;
