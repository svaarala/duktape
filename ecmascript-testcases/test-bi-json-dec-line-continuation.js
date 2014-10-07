/*===
"This is a \
line continuation."
This is a line continuation.
SyntaxError
===*/

/* Line continuation is not allowed for JSON strings. */

try {
    var str = '"This is a \\\nline continuation."';
    print(str);
    print(eval(str));
    print(JSON.parse(str));
} catch (e) {
    print(e.name);
}
