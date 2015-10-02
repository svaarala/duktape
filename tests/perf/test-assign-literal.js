/*
 *  Loading a mix of plain literals to register.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var t;

    for (i = 0; i < 1e7; i++) {
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;

        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
        t = true;
        t = false;
        t = void null;
        t = null;
        t = this;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
