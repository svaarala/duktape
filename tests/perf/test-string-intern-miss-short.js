if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = Duktape.Buffer(31);
    var i;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    for (i = 0; i < 1e5; i++) {
        // make buffer value unique
        buf[0] = i;
        buf[1] = i >> 8;
        buf[2] = i >> 16;

        buf[3] = 0; void ("" + buf);
        buf[3] = 1; void ("" + buf);
        buf[3] = 2; void ("" + buf);
        buf[3] = 3; void ("" + buf);
        buf[3] = 4; void ("" + buf);
        buf[3] = 5; void ("" + buf);
        buf[3] = 6; void ("" + buf);
        buf[3] = 7; void ("" + buf);
        buf[3] = 8; void ("" + buf);
        buf[3] = 9; void ("" + buf);

        buf[3] = 10; void ("" + buf);
        buf[3] = 11; void ("" + buf);
        buf[3] = 12; void ("" + buf);
        buf[3] = 13; void ("" + buf);
        buf[3] = 14; void ("" + buf);
        buf[3] = 15; void ("" + buf);
        buf[3] = 16; void ("" + buf);
        buf[3] = 17; void ("" + buf);
        buf[3] = 18; void ("" + buf);
        buf[3] = 19; void ("" + buf);

        buf[3] = 20; void ("" + buf);
        buf[3] = 21; void ("" + buf);
        buf[3] = 22; void ("" + buf);
        buf[3] = 23; void ("" + buf);
        buf[3] = 24; void ("" + buf);
        buf[3] = 25; void ("" + buf);
        buf[3] = 26; void ("" + buf);
        buf[3] = 27; void ("" + buf);
        buf[3] = 28; void ("" + buf);
        buf[3] = 29; void ("" + buf);

        buf[3] = 30; void ("" + buf);
        buf[3] = 31; void ("" + buf);
        buf[3] = 32; void ("" + buf);
        buf[3] = 33; void ("" + buf);
        buf[3] = 34; void ("" + buf);
        buf[3] = 35; void ("" + buf);
        buf[3] = 36; void ("" + buf);
        buf[3] = 37; void ("" + buf);
        buf[3] = 38; void ("" + buf);
        buf[3] = 39; void ("" + buf);

        buf[3] = 40; void ("" + buf);
        buf[3] = 41; void ("" + buf);
        buf[3] = 42; void ("" + buf);
        buf[3] = 43; void ("" + buf);
        buf[3] = 44; void ("" + buf);
        buf[3] = 45; void ("" + buf);
        buf[3] = 46; void ("" + buf);
        buf[3] = 47; void ("" + buf);
        buf[3] = 48; void ("" + buf);
        buf[3] = 49; void ("" + buf);

        buf[3] = 50; void ("" + buf);
        buf[3] = 51; void ("" + buf);
        buf[3] = 52; void ("" + buf);
        buf[3] = 53; void ("" + buf);
        buf[3] = 54; void ("" + buf);
        buf[3] = 55; void ("" + buf);
        buf[3] = 56; void ("" + buf);
        buf[3] = 57; void ("" + buf);
        buf[3] = 58; void ("" + buf);
        buf[3] = 59; void ("" + buf);

        buf[3] = 60; void ("" + buf);
        buf[3] = 61; void ("" + buf);
        buf[3] = 62; void ("" + buf);
        buf[3] = 63; void ("" + buf);
        buf[3] = 64; void ("" + buf);
        buf[3] = 65; void ("" + buf);
        buf[3] = 66; void ("" + buf);
        buf[3] = 67; void ("" + buf);
        buf[3] = 68; void ("" + buf);
        buf[3] = 69; void ("" + buf);

        buf[3] = 70; void ("" + buf);
        buf[3] = 71; void ("" + buf);
        buf[3] = 72; void ("" + buf);
        buf[3] = 73; void ("" + buf);
        buf[3] = 74; void ("" + buf);
        buf[3] = 75; void ("" + buf);
        buf[3] = 76; void ("" + buf);
        buf[3] = 77; void ("" + buf);
        buf[3] = 78; void ("" + buf);
        buf[3] = 79; void ("" + buf);

        buf[3] = 80; void ("" + buf);
        buf[3] = 81; void ("" + buf);
        buf[3] = 82; void ("" + buf);
        buf[3] = 83; void ("" + buf);
        buf[3] = 84; void ("" + buf);
        buf[3] = 85; void ("" + buf);
        buf[3] = 86; void ("" + buf);
        buf[3] = 87; void ("" + buf);
        buf[3] = 88; void ("" + buf);
        buf[3] = 89; void ("" + buf);

        buf[3] = 90; void ("" + buf);
        buf[3] = 91; void ("" + buf);
        buf[3] = 92; void ("" + buf);
        buf[3] = 93; void ("" + buf);
        buf[3] = 94; void ("" + buf);
        buf[3] = 95; void ("" + buf);
        buf[3] = 96; void ("" + buf);
        buf[3] = 97; void ("" + buf);
        buf[3] = 98; void ("" + buf);
        buf[3] = 99; void ("" + buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
