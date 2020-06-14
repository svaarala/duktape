// https://github.com/svaarala/duktape/issues/2207

/*===
{"foo":0,"nonEnumerable":0}
done
===*/

Object.defineProperty(Array.prototype, 0, { set: function () { } });

function jsonStringifyOwnKeysProxyTest () {
  target = { foo: 0, nonEnumerable: 0 };
  proxy = new Proxy(target, { $: function () { }, ownKeys: function () { return [ 'foo', 'nonEnumerable' ] } });
  print(String(JSON.stringify(proxy)));
}

jsonStringifyOwnKeysProxyTest();
print('done');
