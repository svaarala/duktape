// Somewhat bizarrely, you can't delete typed array indices but they still
// report as configurable.

/*===
true true true
===*/

function test() {
    var u8 = new Uint8Array(3);
    var pd = Object.getOwnPropertyDescriptor(u8, '1');
    print(pd.writable, pd.enumerable, pd.configurable);
}

test();
