/*===
foo
foo
fo
fo
f
f
===*/

/* Undefined captures were not skipped correctly. */

try {
    print('foo'.replace(/(o)|(o)/, '$1'));
    print('foo'.replace(/(o)|(o)/, '$01'));

    // capture 2 not found, matching string should be replaced with empty;
    // with the bug, the first one would print "f2o".
    print('foo'.replace(/(o)|(o)/, '$2'));
    print('foo'.replace(/(o)|(o)/, '$02'));
    print('foo'.replace(/(o)|(o)/g, '$2'));
    print('foo'.replace(/(o)|(o)/g, '$02'));
} catch (e) {
    print(e);
}
