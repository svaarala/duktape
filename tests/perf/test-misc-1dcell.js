if (typeof print !== 'function') { print = console.log; }

function evolve(input, rule) {
    var res = [];
    var i;
    var bits;

    for (i = 0; i < input.length; i++) {
        bits = 0;
        if (i - 1 >= 0) {
            bits += input[i - 1] * 4;
        }
        if (i - 1 < input.length) {
            bits += input[i + 1];
        }
        if (i - 2 >= 0) {
            bits += input[i - 2] * 8;
        }
        if (i + 2 < input.length) {
            bits += input[i + 2] * 16;
        }
        bits += input[i] * 2;

        if ((1 << bits) & rule) {
            res[i] = 0;
        } else {
            res[i] = 1;
        }
    }

    res[(Math.random() * input.length) >>> 0] = (Math.random() > 0.5 ? 1 : 0);

    return res;
}

function mapchar(v) {
    return v ? ' ' : '#';
}

function dump(input) {
    var line = '|' + input.map(mapchar).join('') + '|';
    //print(line);
}

function test() {
    var input;
    var rulenum;
    var round;

    //rulenum = (Math.random() * 0x100000000) >>> 0;
    rulenum = 0xdeadbeef;

    input = [];
    while (input.length < 512) {
        input.push(1);
    }
    input[256] = 0;

    for (round = 0; round < 1e4; round++) {
        dump(input);
        input = evolve(input, rulenum);
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
