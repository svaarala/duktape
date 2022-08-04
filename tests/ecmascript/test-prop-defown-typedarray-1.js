/*===
OK
TypeError
===*/

function test() {
    var u8 = new Uint8Array(3);

    // Typed array indices cannot be deleted but they are still nominally
    // configurable.  So attempt to set configurable=true works, but
    // configurable=false fails.

    try {
        Object.defineProperty(u8, '1', { configurable: true });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(u8, '1', { configurable: false });
        print('OK');
    } catch (e) {
        print(e.name);
    }
}

test();
