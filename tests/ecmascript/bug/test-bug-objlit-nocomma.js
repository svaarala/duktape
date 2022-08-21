/*===
SyntaxError
===*/

try {
    var obj = eval("({ 'foo': 1 'bar': 2 })");
    print(obj.foo, obj.bar);
} catch (e) {
    print(e.name);
}
