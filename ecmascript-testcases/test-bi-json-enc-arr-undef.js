/*===
[]
[]
[null]
[
  null
]
[null,null]
[
  null,
  null
]
[null,null,null]
[
  null,
  null,
  null
]
[null,"foo",null,"bar",null]
[
  null,
  "foo",
  null,
  "bar",
  null
]
===*/

function arrayUndefinedTest() {
    print(JSON.stringify([]));
    print(JSON.stringify([], null, 2));
    print(JSON.stringify([undefined]));
    print(JSON.stringify([undefined], null, 2));
    print(JSON.stringify([undefined,undefined]));
    print(JSON.stringify([undefined,undefined], null, 2));
    print(JSON.stringify([undefined,undefined,undefined]));
    print(JSON.stringify([undefined,undefined,undefined], null, 2));

    print(JSON.stringify([undefined, 'foo', undefined, 'bar', undefined]));
    print(JSON.stringify([undefined, 'foo', undefined, 'bar', undefined], null, 2));
}

try {
    arrayUndefinedTest();
} catch (e) {
    print(e.name);
}
