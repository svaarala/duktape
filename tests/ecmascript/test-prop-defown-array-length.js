/*===
- test 1
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
8 foo,bar,quux,baz,quuux,,, {"value":8,"writable":true,"enumerable":false,"configurable":false}
2 foo,bar {"value":2,"writable":false,"enumerable":false,"configurable":false}
- test 2
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
TypeError
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
8 foo,bar,quux,baz,quuux,,, {"value":8,"writable":true,"enumerable":false,"configurable":false}
TypeError
5 foo,bar,quux,baz,quuux {"value":5,"writable":false,"enumerable":false,"configurable":false}
- test 3
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
TypeError
4 foo,bar,quux,baz {"value":4,"writable":true,"enumerable":false,"configurable":false}
8 foo,bar,quux,baz,,,, {"value":8,"writable":true,"enumerable":false,"configurable":false}
TypeError
4 foo,bar,quux,baz {"value":4,"writable":false,"enumerable":false,"configurable":false}
- test 4
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
5 foo,bar,quux,baz,quuux {"value":5,"writable":true,"enumerable":false,"configurable":false}
TypeError
5 foo,bar,quux,baz,quuux {"value":5,"writable":false,"enumerable":false,"configurable":false}
===*/

function test() {
    var arr;

    function dump() {
        print(arr.length, String(arr), JSON.stringify(Object.getOwnPropertyDescriptor(arr, 'length')));
    }

    print('- test 1');

    arr = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    dump();
    Object.defineProperty(arr, 'length', { value: 8 });
    dump();
    Object.defineProperty(arr, 'length', { value: 2, writable: false });
    dump();

    print('- test 2');
    arr = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    dump();
    Object.defineProperty(arr, 4, { configurable: false });
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 2 });
    } catch (e) {
        print(e.name);
    }
    dump();
    Object.defineProperty(arr, 'length', { value: 8 });
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 1, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- test 3');
    arr = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    dump();
    Object.defineProperty(arr, 3, { configurable: false });
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 2 });
    } catch (e) {
        print(e.name);
    }
    dump();
    Object.defineProperty(arr, 'length', { value: 8 });
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 1, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- test 4');
    arr = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    dump();
    Object.defineProperty(arr, 4, { configurable: false });
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 1, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
