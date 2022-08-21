// https://github.com/svaarala/duktape/issues/2203

/*===
fundefinedo
===*/

Object.defineProperty(Array.prototype, 0, { set: function () { } });
print(String('foo'.replace(/(o)|(o)/)));
