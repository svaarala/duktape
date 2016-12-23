if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = (Uint8Array.allocPlain || Duktape.Buffer)(31);
    var i;
    var bufferToString = String.fromBufferRaw || String;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    for (i = 0; i < 1e5; i++) {
        // make buffer value unique
        buf[0] = i;
        buf[1] = i >> 8;
        buf[2] = i >> 16;

        buf[3] = 0; void bufferToString(buf);
        buf[3] = 1; void bufferToString(buf);
        buf[3] = 2; void bufferToString(buf);
        buf[3] = 3; void bufferToString(buf);
        buf[3] = 4; void bufferToString(buf);
        buf[3] = 5; void bufferToString(buf);
        buf[3] = 6; void bufferToString(buf);
        buf[3] = 7; void bufferToString(buf);
        buf[3] = 8; void bufferToString(buf);
        buf[3] = 9; void bufferToString(buf);

        buf[3] = 10; void bufferToString(buf);
        buf[3] = 11; void bufferToString(buf);
        buf[3] = 12; void bufferToString(buf);
        buf[3] = 13; void bufferToString(buf);
        buf[3] = 14; void bufferToString(buf);
        buf[3] = 15; void bufferToString(buf);
        buf[3] = 16; void bufferToString(buf);
        buf[3] = 17; void bufferToString(buf);
        buf[3] = 18; void bufferToString(buf);
        buf[3] = 19; void bufferToString(buf);

        buf[3] = 20; void bufferToString(buf);
        buf[3] = 21; void bufferToString(buf);
        buf[3] = 22; void bufferToString(buf);
        buf[3] = 23; void bufferToString(buf);
        buf[3] = 24; void bufferToString(buf);
        buf[3] = 25; void bufferToString(buf);
        buf[3] = 26; void bufferToString(buf);
        buf[3] = 27; void bufferToString(buf);
        buf[3] = 28; void bufferToString(buf);
        buf[3] = 29; void bufferToString(buf);

        buf[3] = 30; void bufferToString(buf);
        buf[3] = 31; void bufferToString(buf);
        buf[3] = 32; void bufferToString(buf);
        buf[3] = 33; void bufferToString(buf);
        buf[3] = 34; void bufferToString(buf);
        buf[3] = 35; void bufferToString(buf);
        buf[3] = 36; void bufferToString(buf);
        buf[3] = 37; void bufferToString(buf);
        buf[3] = 38; void bufferToString(buf);
        buf[3] = 39; void bufferToString(buf);

        buf[3] = 40; void bufferToString(buf);
        buf[3] = 41; void bufferToString(buf);
        buf[3] = 42; void bufferToString(buf);
        buf[3] = 43; void bufferToString(buf);
        buf[3] = 44; void bufferToString(buf);
        buf[3] = 45; void bufferToString(buf);
        buf[3] = 46; void bufferToString(buf);
        buf[3] = 47; void bufferToString(buf);
        buf[3] = 48; void bufferToString(buf);
        buf[3] = 49; void bufferToString(buf);

        buf[3] = 50; void bufferToString(buf);
        buf[3] = 51; void bufferToString(buf);
        buf[3] = 52; void bufferToString(buf);
        buf[3] = 53; void bufferToString(buf);
        buf[3] = 54; void bufferToString(buf);
        buf[3] = 55; void bufferToString(buf);
        buf[3] = 56; void bufferToString(buf);
        buf[3] = 57; void bufferToString(buf);
        buf[3] = 58; void bufferToString(buf);
        buf[3] = 59; void bufferToString(buf);

        buf[3] = 60; void bufferToString(buf);
        buf[3] = 61; void bufferToString(buf);
        buf[3] = 62; void bufferToString(buf);
        buf[3] = 63; void bufferToString(buf);
        buf[3] = 64; void bufferToString(buf);
        buf[3] = 65; void bufferToString(buf);
        buf[3] = 66; void bufferToString(buf);
        buf[3] = 67; void bufferToString(buf);
        buf[3] = 68; void bufferToString(buf);
        buf[3] = 69; void bufferToString(buf);

        buf[3] = 70; void bufferToString(buf);
        buf[3] = 71; void bufferToString(buf);
        buf[3] = 72; void bufferToString(buf);
        buf[3] = 73; void bufferToString(buf);
        buf[3] = 74; void bufferToString(buf);
        buf[3] = 75; void bufferToString(buf);
        buf[3] = 76; void bufferToString(buf);
        buf[3] = 77; void bufferToString(buf);
        buf[3] = 78; void bufferToString(buf);
        buf[3] = 79; void bufferToString(buf);

        buf[3] = 80; void bufferToString(buf);
        buf[3] = 81; void bufferToString(buf);
        buf[3] = 82; void bufferToString(buf);
        buf[3] = 83; void bufferToString(buf);
        buf[3] = 84; void bufferToString(buf);
        buf[3] = 85; void bufferToString(buf);
        buf[3] = 86; void bufferToString(buf);
        buf[3] = 87; void bufferToString(buf);
        buf[3] = 88; void bufferToString(buf);
        buf[3] = 89; void bufferToString(buf);

        buf[3] = 90; void bufferToString(buf);
        buf[3] = 91; void bufferToString(buf);
        buf[3] = 92; void bufferToString(buf);
        buf[3] = 93; void bufferToString(buf);
        buf[3] = 94; void bufferToString(buf);
        buf[3] = 95; void bufferToString(buf);
        buf[3] = 96; void bufferToString(buf);
        buf[3] = 97; void bufferToString(buf);
        buf[3] = 98; void bufferToString(buf);
        buf[3] = 99; void bufferToString(buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
