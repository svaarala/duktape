// https://github.com/svaarala/duktape/issues/2202

/*===
A
B
{}
===*/

print('A');
Object.defineProperty(Array.prototype, 0, { set: function () { } })
print('B');
print(String(JSON.stringify({ }, [ 0, 0])));
