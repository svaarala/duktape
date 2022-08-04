/*---
custom: true
---*/

/*===
2
56319 57052
"\udbff\udedc"
===*/

function test() {
    var v = Duktape.dec('jx', '"\\U0010fedc"');
    print(v.length);  // 2 surrogates
    print(v.charCodeAt(0), v.charCodeAt(1));

    // Changed in Duktape 3.x (WTF-8): codepoints in [U+10000,U+10FFFF]
    // encode as a standard ES surrogate pair (but get combined into a
    // single WTF-8 codepoint internally).
    print(Duktape.enc('jc', v));
}

test();
