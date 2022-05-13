/*===
- redefine S["0"] as "f"
allowed
- redefine S[0] as "f"
allowed
- redefine S["0"] as "g"
TypeError
- redefine S[0] as "g"
TypeError
- redefine S["-0"] as "g"
allowed
- redefine S[-0] as "g"
TypeError
===*/

try {
    var S = new String('foo');

    print('- redefine S["0"] as "f"');
    try {
        Object.defineProperty(S, '0', { value: 'f' });
        print('allowed');
    } catch (e) {
        print(e.name);
    }

    print('- redefine S[0] as "f"');
    try {
        Object.defineProperty(S, 0, { value: 'f' });
        print('allowed');
    } catch (e) {
        print(e.name);
    }

    print('- redefine S["0"] as "g"');
    try {
        Object.defineProperty(S, '0', { value: 'g' });
        print('allowed');
    } catch (e) {
        print(e.name);
    }

    print('- redefine S[0] as "g"');
    try {
        Object.defineProperty(S, 0, { value: 'g' });
        print('allowed');
    } catch (e) {
        print(e.name);
    }

    // '-0' is a CanonicalNumericIndexString, but is considered not a match
    // for string indices in StringGetOwnProperty() which is used by
    // String object [[DefineOwnProperty]].
    print('- redefine S["-0"] as "g"');
    try {
        Object.defineProperty(S, '-0', { value: 'g' });
        print('allowed');
    } catch (e) {
        print(e.name);
    }

    // Here the -0 ToPropertyKey() coerces to '0' and *is* thus a valid index.
    print('- redefine S[-0] as "g"');
    try {
        Object.defineProperty(S, -0, { value: 'g' });
        print('allowed');
    } catch (e) {
        print(e.name);
    }
} catch (e) {
    print(e.stack || e);
}
