if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    for (i = 0; i < 1e7; i++) {
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
        } catch (e) {
        }
        try {
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
