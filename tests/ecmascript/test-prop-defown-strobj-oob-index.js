/*===
- define S["-0"]
allowed
999
- define S[-1]
allowed
888
- define S[3]
allowed
777
===*/

try {
    var S = new String('f\ucafeo');  // Length 3.

    print('- define S["-0"]');
    try {
        Object.defineProperty(S, '-0', { value: 999, writable: true, enumerable: true, configurable: true });
        print('allowed');
    } catch (e) {
        print(e.name);
    }
    print(S['-0']);

    print('- define S[-1]');
    try {
        Object.defineProperty(S, '-1', { value: 888, writable: true, enumerable: true, configurable: true });
        print('allowed');
    } catch (e) {
        print(e.name);
    }
    print(S[-1]);

    print('- define S[3]');
    try {
        Object.defineProperty(S, '3', { value: 777, writable: true, enumerable: true, configurable: true });
        print('allowed');
    } catch (e) {
        print(e.name);
    }
    print(S[3]);
} catch (e) {
    print(e.stack || e);
}
