/*===
toString + parse test
-123456789000 -123456789000 -123456789000 -123456789000
-1234567000 -1234567000 -1234567000 -1234567000
-12345000 -12345000 -12345000 -12345000
-123000 -123000 -123000 -123000
0 0 0 0
0 0 0 0
123456789000 123456789000 123456789000 123456789000
1234567000 1234567000 1234567000 1234567000
12345000 12345000 12345000 12345000
123000 123000 123000 123000
===*/

/* Test the conversion property of E5.1 Section 15.9.4.2 (paragraph 2). */

print('toString + parse test');

function toStringParseTest() {
    // all values have milliseconds as 0
    var values = [
        -123456789e3, -1234567e3, -12345e3, -123e3, -0, +0,
        +123456789e3, +1234567e3, +12345e3, +123e3
    ];
    var i, x;

    for (i = 0; i < values.length; i++) {
        x = new Date(values[i]);

        // E5.1 Section 15.9.4.2, 15.9.5.2
        print(x.valueOf(),
              Date.parse(x.toString()),
              Date.parse(x.toUTCString()),
              Date.parse(x.toISOString()));
    }

    // XXX: add more comprehensive cases
}

try {
    toStringParseTest();
} catch (e) {
    print(e.name);
}
