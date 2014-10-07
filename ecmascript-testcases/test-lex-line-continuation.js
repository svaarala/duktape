/*===
"This is a \
line continuation."
This is a line continuation.
===*/

/* Line continuation inside a string. */

var str;

try {
    str = '"This is a \\\nline continuation."';
    print(str);
    print(eval(str));
} catch (e) {
    print(e.name);
}
