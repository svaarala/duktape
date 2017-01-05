/*@include util-buffer.js@*/

/*===
{"buf":{}}
{
    "buf": {}
}
{buf:||}
{
    buf: ||
}
{"buf":{"0":18,"1":35,"2":52}}
{
    "buf": {
        "0": 18,
        "1": 35,
        "2": 52
    }
}
{buf:|122334|}
{
    buf: |122334|
}
{"buf":{}}
{
    "buf": {}
}
{buf:||}
{
    buf: ||
}
{"buf":{"0":18,"1":35,"2":52}}
{
    "buf": {
        "0": 18,
        "1": 35,
        "2": 52
    }
}
{buf:|122334|}
{
    buf: |122334|
}
===*/

function test() {
    var b;

    function f(v) {
        v = { buf: v };
        print(JSON.stringify(v));
        print(JSON.stringify(v, null, 4));
        print(Duktape.enc('jx', v));
        print(Duktape.enc('jx', v, null, 4));
    }

    b = new Uint8Array(0);
    f(b);

    b = new Uint8Array(3);
    b[0] = 0x12;
    b[1] = 0x23;
    b[2] = 0x34;
    f(b);

    b = createPlainBuffer(0);
    f(b);

    b = createPlainBuffer(3);
    b[0] = 0x12;
    b[1] = 0x23;
    b[2] = 0x34;
    f(b);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
