/* Even if trap result is usable as is, a new Array is always created
 * using CreateListFromArrayLike() + CreateArrayFromList().
 */

/*===
ownKeys trap, arguments length: 1
true
true
false
["foo","bar"]
===*/

var target = { foo: 123, bar: 234, quux: 345 };
var handler = {};
var retval = [ 'foo', 'bar' ];
handler.ownKeys = function ownKeysTrap(targ) {
    print('ownKeys trap, arguments length:', arguments.length);
    print(this === handler);
    print(targ === target);
    return retval;
}
var P = new Proxy(target, handler);
var res = Reflect.ownKeys(P);
print(res === retval);
print(JSON.stringify(res));
