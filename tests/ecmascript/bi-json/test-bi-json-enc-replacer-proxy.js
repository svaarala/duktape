/*===
{"foo":123,"bar":234}
{"foo":123,"bar":234}
===*/

print(JSON.stringify({ foo: 123, bar: 234, quux: 345 }, [ 'foo', 'bar' ]));
print(JSON.stringify({ foo: 123, bar: 234, quux: 345 }, new Proxy([ 'foo', 'bar' ], {})));
