if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    for (i = 0; i < 1e7; i++) {
        // must have an outer try-catch to be able to loop
        // which fuzzes the result a bit

        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
        } catch (e) {
        }
        try {
            try {
                throw 123;
            } finally {
            }
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
