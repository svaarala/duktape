/*===
TypeError
TypeError
TypeError
done
===*/

try {
    undefined[123] = 123;
} catch (e) {
    print(e.name);
}

try {
    null[123] = 123;
} catch (e) {
    print(e.name);
}

try {
    // Invalid base is detected before key coercion.
    null[{
        valueOf: function () { print('valueOf'); return 123; },
        toString: function () { print('toString'); return '123'; }
    }] = 123;
} catch (e) {
    print(e.name);
}

print('done');
