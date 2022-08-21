/*===
5
===*/

// U+180E is no longer white space in Unicode, so trim() should not remove it.
print('\u180efoo\u180e'.trim().length);
