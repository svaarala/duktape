if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    for (i = 0; i < 1e7; i++) {
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
        try {
            throw 123;
        } catch (e) {
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
