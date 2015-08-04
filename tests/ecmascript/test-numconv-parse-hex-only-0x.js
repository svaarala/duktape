/*===
SyntaxError
===*/

try {
    // Hex prefix without digits -> SyntaxError.  One could also interpret
    // this as an octal number (0) followed by an 'x'.
    //
    // In fact, with the interpretation that one should parse the longest
    // valid prefix, this might be the correct interpretation if one
    // supports octal numbers.  This doesn't really matter because a valid
    // number (0) followed by 'x' would be a SyntaxError anyway.

    print(eval('0x'));
} catch(e) {
    print(e.name);
}
