/*
 *  Handling of padding in base-64 decoder.
 *
 *  There's a lot of variance in base-64 decoders in dealing with padding.
 */

/*===
"" ""
"==" ""
" " ""
"=" ""
"===" ""
" = " ""
"==" ""
"====" ""
" = = " ""
"===" ""
"=====" ""
" = = = " ""
"====" ""
"======" ""
" = = = = " ""
"=====" ""
"=======" ""
" = = = = = " ""
"Z" TypeError
"==Z" TypeError
" Z " TypeError
"Z=" TypeError
"==Z=" TypeError
" Z = " TypeError
"Z==" TypeError
"==Z==" TypeError
" Z = = " TypeError
"Z===" TypeError
"==Z===" TypeError
" Z = = = " TypeError
"Z====" TypeError
"==Z====" TypeError
" Z = = = = " TypeError
"Z=====" TypeError
"==Z=====" TypeError
" Z = = = = = " TypeError
"Zm9vZ" TypeError
"==Zm9vZ" TypeError
" Z m 9 v Z " TypeError
"Zm9vZ=" TypeError
"==Zm9vZ=" TypeError
" Z m 9 v Z = " TypeError
"Zm9vZ==" TypeError
"==Zm9vZ==" TypeError
" Z m 9 v Z = = " TypeError
"Zm9vZ===" TypeError
"==Zm9vZ===" TypeError
" Z m 9 v Z = = = " TypeError
"Zm9vZ====" TypeError
"==Zm9vZ====" TypeError
" Z m 9 v Z = = = = " TypeError
"Zm9vZ=====" TypeError
"==Zm9vZ=====" TypeError
" Z m 9 v Z = = = = = " TypeError
"Zm" "f"
"==Zm" "f"
" Z m " "f"
"Zm=" "f"
"==Zm=" "f"
" Z m = " "f"
"Zm==" "f"
"==Zm==" "f"
" Z m = = " "f"
"Zm===" "f"
"==Zm===" "f"
" Z m = = = " "f"
"Zm====" "f"
"==Zm====" "f"
" Z m = = = = " "f"
"Zm=====" "f"
"==Zm=====" "f"
" Z m = = = = = " "f"
"Zm9vZm" "foof"
"==Zm9vZm" "foof"
" Z m 9 v Z m " "foof"
"Zm9vZm=" "foof"
"==Zm9vZm=" "foof"
" Z m 9 v Z m = " "foof"
"Zm9vZm==" "foof"
"==Zm9vZm==" "foof"
" Z m 9 v Z m = = " "foof"
"Zm9vZm===" "foof"
"==Zm9vZm===" "foof"
" Z m 9 v Z m = = = " "foof"
"Zm9vZm====" "foof"
"==Zm9vZm====" "foof"
" Z m 9 v Z m = = = = " "foof"
"Zm9vZm=====" "foof"
"==Zm9vZm=====" "foof"
" Z m 9 v Z m = = = = = " "foof"
"Zm9" "fo"
"==Zm9" "fo"
" Z m 9 " "fo"
"Zm9=" "fo"
"==Zm9=" "fo"
" Z m 9 = " "fo"
"Zm9==" "fo"
"==Zm9==" "fo"
" Z m 9 = = " "fo"
"Zm9===" "fo"
"==Zm9===" "fo"
" Z m 9 = = = " "fo"
"Zm9====" "fo"
"==Zm9====" "fo"
" Z m 9 = = = = " "fo"
"Zm9=====" "fo"
"==Zm9=====" "fo"
" Z m 9 = = = = = " "fo"
"Zm9vZm9" "foofo"
"==Zm9vZm9" "foofo"
" Z m 9 v Z m 9 " "foofo"
"Zm9vZm9=" "foofo"
"==Zm9vZm9=" "foofo"
" Z m 9 v Z m 9 = " "foofo"
"Zm9vZm9==" "foofo"
"==Zm9vZm9==" "foofo"
" Z m 9 v Z m 9 = = " "foofo"
"Zm9vZm9===" "foofo"
"==Zm9vZm9===" "foofo"
" Z m 9 v Z m 9 = = = " "foofo"
"Zm9vZm9====" "foofo"
"==Zm9vZm9====" "foofo"
" Z m 9 v Z m 9 = = = = " "foofo"
"Zm9vZm9=====" "foofo"
"==Zm9vZm9=====" "foofo"
" Z m 9 v Z m 9 = = = = = " "foofo"
"Zm9v" "foo"
"==Zm9v" "foo"
" Z m 9 v " "foo"
"Zm9v=" "foo"
"==Zm9v=" "foo"
" Z m 9 v = " "foo"
"Zm9v==" "foo"
"==Zm9v==" "foo"
" Z m 9 v = = " "foo"
"Zm9v===" "foo"
"==Zm9v===" "foo"
" Z m 9 v = = = " "foo"
"Zm9v====" "foo"
"==Zm9v====" "foo"
" Z m 9 v = = = = " "foo"
"Zm9v=====" "foo"
"==Zm9v=====" "foo"
" Z m 9 v = = = = = " "foo"
"Zm9vZm9v" "foofoo"
"==Zm9vZm9v" "foofoo"
" Z m 9 v Z m 9 v " "foofoo"
"Zm9vZm9v=" "foofoo"
"==Zm9vZm9v=" "foofoo"
" Z m 9 v Z m 9 v = " "foofoo"
"Zm9vZm9v==" "foofoo"
"==Zm9vZm9v==" "foofoo"
" Z m 9 v Z m 9 v = = " "foofoo"
"Zm9vZm9v===" "foofoo"
"==Zm9vZm9v===" "foofoo"
" Z m 9 v Z m 9 v = = = " "foofoo"
"Zm9vZm9v====" "foofoo"
"==Zm9vZm9v====" "foofoo"
" Z m 9 v Z m 9 v = = = = " "foofoo"
"Zm9vZm9v=====" "foofoo"
"==Zm9vZm9v=====" "foofoo"
" Z m 9 v Z m 9 v = = = = = " "foofoo"
"Zm=Zm9v" "ffoo"
"==Zm=Zm9v" "ffoo"
" Z m = Z m 9 v " "ffoo"
"Zm==Zm9v" "ffoo"
"==Zm==Zm9v" "ffoo"
" Z m = = Z m 9 v " "ffoo"
"Zm===Zm9v" "ffoo"
"==Zm===Zm9v" "ffoo"
" Z m = = = Z m 9 v " "ffoo"
"Zm9=Zm9v" "fofoo"
"==Zm9=Zm9v" "fofoo"
" Z m 9 = Z m 9 v " "fofoo"
"Zm9==Zm9v" "fofoo"
"==Zm9==Zm9v" "fofoo"
" Z m 9 = = Z m 9 v " "fofoo"
"Zm==Zm" "ff"
"==Zm==Zm" "ff"
" Z m = = Z m " "ff"
"Zm===Zm" "ff"
"==Zm===Zm" "ff"
" Z m = = = Z m " "ff"
"=Zm==Zm9v====Zm====Zm9" "ffooffo"
"===Zm==Zm9v====Zm====Zm9" "ffooffo"
" = Z m = = Z m 9 v = = = = Z m = = = = Z m 9 " "ffooffo"
===*/

/*@include util-base.js@*/

function injectWhitespace(x) {
    return ' ' + x.replace(/./g, function (v) { return v + ' '; });
}

function test() {
    [
        // Empty input -> empty output.
        '',
        '=',
        '==',
        '===',
        '====',
        '=====',

        // A single character in the group is not defined in the specification
        // (even with padding).  Node.js decodes it to an empty string.
        'Z',
        'Z=',
        'Z==',
        'Z===',
        'Z====',
        'Z=====',
        'Zm9vZ',
        'Zm9vZ=',
        'Zm9vZ==',
        'Zm9vZ===',
        'Zm9vZ====',
        'Zm9vZ=====',

        // Two characters in the group are accepted.  Partial and missing
        // padding is accepted in Duktape 2.3 (matches Node.js).
        'Zm',
        'Zm=',
        'Zm==',
        'Zm===',
        'Zm====',
        'Zm=====',
        'Zm9vZm',
        'Zm9vZm=',
        'Zm9vZm==',
        'Zm9vZm===',
        'Zm9vZm====',
        'Zm9vZm=====',

        // Same for three characters.
        'Zm9',
        'Zm9=',
        'Zm9==',
        'Zm9===',
        'Zm9====',
        'Zm9=====',
        'Zm9vZm9',
        'Zm9vZm9=',
        'Zm9vZm9==',
        'Zm9vZm9===',
        'Zm9vZm9====',
        'Zm9vZm9=====',

        // Four characters is a full group.
        'Zm9v',
        'Zm9v=',
        'Zm9v==',
        'Zm9v===',
        'Zm9v====',
        'Zm9v=====',
        'Zm9vZm9v',
        'Zm9vZm9v=',
        'Zm9vZm9v==',
        'Zm9vZm9v===',
        'Zm9vZm9v====',
        'Zm9vZm9v=====',

        // Concatenation with various padding lengths.
        'Zm=Zm9v',
        'Zm==Zm9v',
        'Zm===Zm9v',
        'Zm9=Zm9v',
        'Zm9==Zm9v',
        'Zm==Zm',
        'Zm===Zm',
        '=Zm==Zm9v====Zm====Zm9'
    ].forEach(function (v) {
        var input;

        // Try input as is, and a few automated variations.
        input = v;
        try {
            print(Test.valueToString(input), Test.valueToString(new TextDecoder().decode(Duktape.dec('base64', input))));
        } catch (e) {
            print(Test.valueToString(input), e.name);
        }

        // Prepended padding is accepted: the rationale is that since we
        // accept concatenated base-64 documents and an empty document is
        // accepted when encoded e.g. as '==', prepended padding is just
        // an empty document prepended to the input.
        input = '==' + v;
        try {
            print(Test.valueToString(input), Test.valueToString(new TextDecoder().decode(Duktape.dec('base64', input))));
        } catch (e) {
            print(Test.valueToString(input), e.name);
        }

        // Injected whitespace drops us out of the internal fast path.
        input = injectWhitespace(v);
        try {
            print(Test.valueToString(input), Test.valueToString(new TextDecoder().decode(Duktape.dec('base64', input))));
        } catch (e) {
            print(Test.valueToString(input), e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
