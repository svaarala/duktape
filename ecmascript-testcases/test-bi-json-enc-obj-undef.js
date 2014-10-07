/*===
{}
{}
{"bar":2}
{}
===*/

function objectUndefinedTest() {
    var obj;

    obj = {}
    print(JSON.stringify(obj));

    obj = { foo: undefined };
    print(JSON.stringify(obj));

    obj = { foo: undefined, bar: 2, quux: undefined };
    print(JSON.stringify(obj));

    // all values become undefined after the replacer
    obj = { foo: 1, bar: 2, quux: 3, baz: 4 };
    print(JSON.stringify(obj, function repl(k,v) { if (k !== '') { return; } else { return v;} }));

}

try {
    objectUndefinedTest();
} catch (e) {
    print(e.name);
}
