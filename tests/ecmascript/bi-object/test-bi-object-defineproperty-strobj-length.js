/*===
OK
OK
TypeError
TypeError
TypeError
OK
TypeError
OK
TypeError
OK
TypeError
TypeError
===*/

function test() {
    var obj = new String('foo');

    try {
        Object.defineProperty(obj, 'length', {});
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { value: 3 });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { value: 4 });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { value: '3' });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { writable: true });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { writable: false });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { enumerable: true });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { enumerable: false });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { configurable: true });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { configurable: false });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { get: Math.cos });
        print('OK');
    } catch (e) {
        print(e.name);
    }

    try {
        Object.defineProperty(obj, 'length', { set: Math.sin });
        print('OK');
    } catch (e) {
        print(e.name);
    }
}

test();
