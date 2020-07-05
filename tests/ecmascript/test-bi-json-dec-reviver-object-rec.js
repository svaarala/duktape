// When we revive object key .foo, the key .bar is always replaced with a
// new object.  When we walk that object, the same happens, without limit.
// Note that the .bar key must be present when the Walk() algorithm first
// enumerates the target object; new keys are not considered.

/*===
RangeError
done
===*/

function revive(key, value) {
    this.bar = { foo: 123, bar: 234 };
    return value;
}

try {
    print(JSON.parse('{ "foo": 123, "bar": 234 }', revive));
} catch (e) {
    print(e.name);
}
print('done');
