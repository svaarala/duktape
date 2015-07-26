/*---
{
    "custom": true
}
---*/

/*===
Infinity
implementation dependent -Infinity
===*/

function timeClipZeroTest() {
    // TimeClip() may (but is not required) to convert a negative zero
    // to a positive one.  Our current implementation will preserve the
    // sign so test for that behavior (V8 has the same behavior, Rhino
    // does not).

    d = new Date(+0); print(1 / d.getTime());
    d = new Date(-0); print('implementation dependent', 1 / d.getTime());
}

try {
    timeClipZeroTest();
} catch (e) {
    print(e.stack || e);
}
