/*===
{"65":"C","123":"B","321":"A"}
===*/

function test() {
    var obj = { 321: 'A', '65': 'C', 123: 'B' };
    print(JSON.stringify(obj));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
