/*===
foobarfoobar
foobarfoobar
foobarbarfooffoobarfoobar
===*/
[
    // Fast path: first pair (Zm9vYmFy) has two clean groups.
    // Second pair has a clean group + a non-clean group.
    'Zm9vYmFyZm9v Y m F y',

     // Fast path: first pair has a non-clean group followed
     // by a clean group (which is ignored because it will be
     // decoded from an incorrect assumed offset).
     'Zm9 vYmFy Z m 9 v Y m F y',

     // Clean groups, unclean group, some clean groups.
     'Zm9vYmFyYmFyZm9v Zm== Zm9vYmFyZm9vYmFy',
].forEach(function (v) {
    try {
        print(new TextDecoder().decode(Duktape.dec('base64', v)));
    } catch (e) {
        print(e.name);
    }
});
