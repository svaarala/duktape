/*===
"This is a \
line continuation."
This is a line continuation.
===*/

/* Line continuation inside a string. */

var str;

str = '"This is a \\\nline continuation."';
print(str);
print(eval(str));
