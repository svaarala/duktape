/*---
{
    "skip": true
}
---*/

try {
    decodeURIComponent('%e1%a9%01');  // invalid utf-8
} catch (e) {
    print(e.stack || e);
}
