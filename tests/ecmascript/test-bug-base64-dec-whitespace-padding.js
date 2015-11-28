/*
 *  Duktape 1.3.0 had a bug where whitespace inside padding bytes was not
 *  tolerated.
 */

/*===
foo
foo
fo
fo
f
f
f
TypeError
===*/

function test() {
    // Whitespace is accepted at any point.
    print(String(Duktape.dec('base64', 'Zm9v')));
    print(String(Duktape.dec('base64', ' Z m 9 v ')));

    // Works also for padding.
    print(String(Duktape.dec('base64', 'Zm9=')));
    print(String(Duktape.dec('base64', ' Z m 9 =')));

    // Whitespace between padding bytes was not accepted by Duktape 1.3.0.
    try {
        print(String(Duktape.dec('base64', 'Zm==')));
    } catch (e) {
        print(e);
    }
    try {
        print(String(Duktape.dec('base64', 'Zm==')));
    } catch (e) {
        print(e);
    }
    try {
        print(String(Duktape.dec('base64', ' Z m = = ')));
    } catch (e) {
        print(e);
    }

    // Mixed padding not allowed.
    try {
        print(String(Duktape.dec('base64', 'Z=m=')));
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
