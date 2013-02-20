print=console.log

/*===
identity replacer
replacer: [object Object] string  object [object Object]
replacer: [object Object] string foo number 1
replacer: [object Object] string bar string bar
{"foo":1,"bar":"bar"}
replace top level with foo
"foo"
replace non-empty primitive values
{"foo":"foo","bar":"foo","quux":{"key1":"foo","key2":"foo"},"quuux":["foo","foo","foo"]}
===*/

function replacerTest1() {
    var obj = {
        foo: 1,
        bar: 'bar'
    };

    function repl(k, v) {
        print('replacer:', this, typeof k, k, typeof v, v);
        return v;
    }

    // Replacer will be called first for the specification mandated
    // holder object and an empty string key.
    print(JSON.stringify(obj, repl));
}

function replacerTest2() {
    var obj = {
        foo: 1,
        bar: 'bar'
    };

    function repl(k, v) {
        return 'foo';
    }

    // When the replacer is called for the top-level holder, return
    // 'foo', thus serializing only that string.
    print(JSON.stringify(obj, repl));
}

function replacerTest3() {
    var obj = {
        foo: 1,
        bar: 'bar',
        quux: {
            key1: 'val1', key2: 'val2'
        },
        quuux: [
            'arr1', 'arr2', 'arr3'
        ]
    };

    function repl(k, v) {
        if (k.length > 0 && typeof v !== 'object') {
            return 'foo';
        } else {
            return v;
        }
    }

    // Replace every non-object value with a non-empty key with 'foo'.
    //
    // Note that this also affects the array serialization.
    // Rhino seems to skip replacer for the array.

    print(JSON.stringify(obj, repl));
}

try {
    print('identity replacer');
    replacerTest1();

    print('replace top level with foo');
    replacerTest2();

    print('replace non-empty primitive values')
    replacerTest3();
} catch (e) {
    print(e.name);
}
