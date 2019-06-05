/*
 *  A shim for running underscore tests.
 *
 *  See: http://api.qunitjs.com/
 */

var skipErrors = true;

var module = function(name) {
    print('MODULE', name);
};

var assert_count;

var test = function(name, arg1, arg2) {
    var emsg;
    print('***', name);

    assert_count = 0;
    if (typeof(arg1) === 'function') {
        arg1();
    } else {
        // deprecated
        arg2();
        if (assert_count !== arg1) {
            emsg = 'TEST CASE FAILED: assert count mismatch (' + assert_count + ' vs ' + arg1 + ')';
            if (skipErrors) {
                print(emsg);
            } else {
                throw new Error(emsg);
            }
        }
    }
};

var asyncTest = function(name, arg1, arg2) {
    print('***', name, 'SKIPPED (asyncTest unimplemented)');
    return;

    assert_count = 0;
    if (typeof(arg1) === 'function') {
        throw new Error('unimplemented');
    } else {
        // deprecated
        throw new Error('unimplemented');
        if (assert_count !== arg1) {
            emsg = 'TEST CASE FAILED: assert count mismatch (' + assert_count + ' vs ' + arg1 + ')';
            if (skipErrors) {
                print(emsg);
            } else {
                throw new Error(emsg);
            }
        }
    }
}

var handleResult = function(bool, msg) {
    var emsg;

    if (bool) {
        print('SUCCESS', msg);
    } else {
        emsg = 'FAILURE ' + msg;
        if (skipErrors) {
            print(emsg);
        } else {
            throw new Error(emsg);
        }
    }
}

var ok = function(bool, msg) {
    assert_count++;
    handleResult(bool, msg);
};

var equal = function(x, y, msg) {
    assert_count++;
    handleResult(x == y, msg);
};

var notEqual = function(x, y, msg) {
    assert_count++;
    handleResult(x != y, msg);
};

var strictEqual = function(x, y, msg) {
    assert_count++;
    handleResult(x === y, msg);
};

var notStrictEqual = function(x, y, msg) {
    assert_count++;
    handleResult(x !== y, msg);
};

var deepEqual = function(x, y, msg) {
    // FIXME: incorrect as key ordering matters, but close enough maybe?
    assert_count++;
    handleResult(Duktape.enc('jx', x) === Duktape.enc('jx', y), msg);
};

var notDeepEqual = function(x, y, msg) {
    // FIXME: incorrect as key ordering matters, but close enough maybe?
    assert_count++;
    handleResult(Duktape.enc('jx', x) !== Duktape.enc('jx', y), msg);
};

var raises = function(fn, expect_err, msg) {
    var emsg;
    var err_name = expect_err.name;

    assert_count++;
    try {
        fn();
        emsg = 'FAILURE ' + msg + ' -- did not throw ' + err_name + ' as expected';
        if (skipErrors) {
            print(emsg);
        } else {
            throw new Error(emsg);
        }
    } catch (e) {
        if (err_name === e.name) {
            print('SUCCESS', msg);
        } else {
            emsg = 'FAILURE ' + msg + ' -- got unexpected error type: ' + e.name + ' (expected ' + err_name + ')';
            if (skipErrors) {
                print(emsg);
            } else {
                throw new Error(emsg);
            }
        }
    }
}

// Minimal jQuery fakery.  These are needed to transform the $(document).ready(fn);
// notation into a synchronous call, and to provide enough fake data and functions
// to satisfy the tests (while not breaking the test intent).

var doc_images = [ { id: 'chart_image' } ];
var document = {
    images: doc_images
};
var iObject = {};

var $ = function(arg) {
    if (arg === document) {
        return $;
    }

    if (arg === '#map-test') {
        return {
            children: function() {
                return [ { id: 'id1' }, { id: 'id2' } ];
            }
        };
    }

    // This is a bit tricky, the test is more or less bypassed.
    // equal(_.size($('<div>').add('<span>').add('<span>')), 3, 'can compute the size of jQuery objects');

    if (arg === '<div>') {
        var fun_count = 1;
        var fun = function (add_arg) {
            fun_count++;
            if (fun_count == 3) {
                return { length: 3 };
            } else {
                return fun;
            }
        };
        fun.add = fun;
        return fun;
    }

    throw Error('unexpected call to $: ' + arg);
};

$.ready = function(cb) {
    // just call directly
    cb();
};

// this disables some unwanted browser tests
$.browser = {
    msie: true
};

var jQuery = $;

// document.createElement() is used by some tests
document.createElement = function createElement(name) {
    return { name: name };  // Just a dummy return value
};

// the Duktape require() function confuses underscore tests
delete require;

// ... and the 'window' binding is expected
var window = new Function('return this;')();
window.location = {};
window.location.protocol = 'file:';
window.location.search = '?';
window.addEventListener = function nop() {};
window.removeEventListener = function nop() {};
