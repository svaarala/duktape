/*===
1
2
3
{"value":1,"writable":true,"enumerable":true,"configurable":true}
{"value":2,"writable":true,"enumerable":true,"configurable":true}
{"value":3,"writable":true,"enumerable":true,"configurable":true}
100
100
2
3
{"value":100,"writable":true,"enumerable":true,"configurable":true}
{"value":2,"writable":true,"enumerable":true,"configurable":true}
{"value":3,"writable":true,"enumerable":true,"configurable":true}
200
200
2
3
{"value":100,"writable":false,"enumerable":true,"configurable":true}
{"value":2,"writable":true,"enumerable":true,"configurable":true}
{"value":3,"writable":true,"enumerable":true,"configurable":true}
===*/

function foo(a, b, c) {
    return { A: arguments, E: function (code) { return eval(code); } };
}

var ret = foo(1, 2, 3);
print(ret.E('a'));
print(ret.E('b'));
print(ret.E('c'));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '0')));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '1')));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '2')));
print(ret.E('a = 100'));
print(ret.E('a'));
print(ret.E('b'));
print(ret.E('c'));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '0')));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '1')));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '2')));
Object.defineProperty(ret.A, '0', { writable: false });
print(ret.E('a = 200'));
print(ret.E('a'));
print(ret.E('b'));
print(ret.E('c'));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '0')));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '1')));
print(JSON.stringify(Object.getOwnPropertyDescriptor(ret.A, '2')));
