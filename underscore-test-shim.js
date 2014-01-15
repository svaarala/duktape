/*
 *  A shim for running underscore tests.
 */

var skipErrors = true;

var module = function(name) {
    print('MODULE', name);
};

var test = function(name, func) {
    print('***', name);
    func();
};

var handleResult = function(bool, msg) {
    if (bool) {
        print('SUCCESS', msg);
    } else {
        if (skipErrors) {
            print('FAILURE', msg);
        } else {
            throw new Error('FAILURE: ' + msg);
        }
    }
}

var ok = function(bool, msg) {
    handleResult(bool, msg);
};

var equal = function(x, y, msg) {
    handleResult(x == y, msg);
};

var strictEqual = function(x, y, msg) {
    handleResult(x === y, msg);
};

var deepEqual = function(x, y, msg) {
    // FIXME: incorrect as key ordering matters, but close enough maybe?
    handleResult(Duktape.jsonxEnc(x) === Duktape.jsonxEnc(y), msg);
};

// These are needed to transform the $(document).ready(fn); notation into
// a synchronous call.

var document = {};

var $ = function(arg) {
    return $;
};
$.ready = function(cb) {
    // just call directly
    cb();
};

