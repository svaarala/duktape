/*===
- Make dense array shorter.
true false false 10 1,2,3,4,5,6,7,8,9,10
true false false 4 1,2,3,4
- Make dense array shorter when index 7 is not deletable.
true false false 10 1,2,3,4,5,6,7,8,9,10
TypeError
false false false 8 1,2,3,4,5,6,7,8
- Make dense array longer.
true false false 4 1,2,3,4
true false false 6 1,2,3,4,,
true false false 8 1,2,3,4,,,,
- Make dense array longer when length is write protected.
true false false 4 1,2,3,4
true false false 6 1,2,3,4,,
TypeError
false false false 6 1,2,3,4,,
- Make dense array longer and write protect.
true false false 4 1,2,3,4
false false false 6 1,2,3,4,,
- Make dense array shorter and write protect.
true false false 4 1,2,3,4
false false false 2 1,2
- Make dense array same size and write protect.
true false false 4 1,2,3,4
false false false 4 1,2,3,4
- Make abandoned array shorter.
true false false 10 1,2,3,4,5,6,7,8,9,10
true false false 4 1,2,3,4
- Make abandoned array shorter when index 7 is not deletable.
true false false 10 1,2,3,4,5,6,7,8,9,10
TypeError
false false false 8 1,2,3,4,5,6,7,8
- Make abandoned array longer.
true false false 4 1,2,3,4
true false false 6 1,2,3,4,,
TypeError
false false false 6 1,2,3,4,,
- Make abandoned array longer when length is write protected.
true false false 4 1,2,3,4
true false false 6 1,2,3,4,,
TypeError
false false false 6 1,2,3,4,,
- Make abandoned array longer and write protect.
true false false 4 1,2,3,4
false false false 6 1,2,3,4,,
- Make abandoned array shorter and write protect.
true false false 4 1,2,3,4
false false false 2 1,2
- Make abandoned array same size and write protect.
true false false 4 1,2,3,4
false false false 4 1,2,3,4
===*/

function test() {
    var arr;

    function dump() {
        var pd = Object.getOwnPropertyDescriptor(arr, 'length');
        print(pd.writable, pd.enumerable, pd.configurable, pd.value, String(arr));
    }

    print('- Make dense array shorter.');
    arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    dump();
    Object.defineProperty(arr, 'length', { value: 4 });
    dump();

    print('- Make dense array shorter when index 7 is not deletable.');
    arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    dump();
    try {
        Object.defineProperty(arr, '7', { configurable: false });
        Object.defineProperty(arr, 'length', { value: 4, writable: false });
        print('never here');
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make dense array longer.');
    arr = [ 1, 2, 3, 4 ];
    dump();
    arr.length = 6;
    dump();
    Object.defineProperty(arr, 'length', { value: 8 });
    dump();

    print('- Make dense array longer when length is write protected.');
    arr = [ 1, 2, 3, 4 ];
    dump();
    arr.length = 6;
    dump();
    try {
        Object.defineProperty(arr, 'length', { writable: false });
        Object.defineProperty(arr, 'length', { value: 8 });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make dense array longer and write protect.');
    arr = [ 1, 2, 3, 4 ];
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 6, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make dense array shorter and write protect.');
    arr = [ 1, 2, 3, 4 ];
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 2, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make dense array same size and write protect.');
    arr = [ 1, 2, 3, 4 ];
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 4, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();

    // Same tests for abandoned arrays.

    print('- Make abandoned array shorter.');
    arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    arr[1e6] = 'foo';
    arr.length = 10;
    dump();
    Object.defineProperty(arr, 'length', { value: 4 });
    dump();

    print('- Make abandoned array shorter when index 7 is not deletable.');
    arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    arr[1e6] = 'foo';
    arr.length = 10;
    dump();
    try {
        Object.defineProperty(arr, '7', { configurable: false });
        Object.defineProperty(arr, 'length', { value: 4, writable: false });
        print('never here');
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make abandoned array longer.');
    arr = [ 1, 2, 3, 4 ];
    arr[1e6] = 'foo';
    arr.length = 4;
    dump();
    arr.length = 6;
    dump();
    try {
        Object.defineProperty(arr, 'length', { writable: false });
        Object.defineProperty(arr, 'length', { value: 8 });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make abandoned array longer when length is write protected.');
    arr = [ 1, 2, 3, 4 ];
    arr[1e6] = 'foo';
    arr.length = 4;
    dump();
    arr.length = 6;
    dump();
    try {
        Object.defineProperty(arr, 'length', { writable: false });
        Object.defineProperty(arr, 'length', { value: 8 });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make abandoned array longer and write protect.');
    arr = [ 1, 2, 3, 4 ];
    arr[1e6] = 'foo';
    arr.length = 4;
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 6, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make abandoned array shorter and write protect.');
    arr = [ 1, 2, 3, 4 ];
    arr[1e6] = 'foo';
    arr.length = 4;
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 2, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();

    print('- Make abandoned array same size and write protect.');
    arr = [ 1, 2, 3, 4 ];
    arr[1e6] = 'foo';
    arr.length = 4;
    dump();
    try {
        Object.defineProperty(arr, 'length', { value: 4, writable: false });
    } catch (e) {
        print(e.name);
    }
    dump();
}

test();
