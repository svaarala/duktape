/*
 *  Decoding non-BMP, valid UTF-8 strings.
 */

/*===
"foo"
"A\U0010fedcB"
done
===*/

function test(hexInput) {
    try {
        print(Duktape.enc('jx', CBOR.decode(Duktape.dec('hex', hexInput))));
    } catch (e) {
        print(e.name);
    }
}

// Valid CBOR text string.
test('63666f6f');

// Codepoint U+10FEDC.  Input is valid UTF-8 which should decode to a
// surrogate pair for ECMAScript compatibility.
//
// Currently decodes to Duktape specific non-BMP codepoint as is.
test('6641f48fbb9c42');

print('done');
