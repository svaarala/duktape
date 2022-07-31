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

test();
