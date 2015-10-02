// Fractional constants like "4.0" trigger a Dragon4-based very slow
// number parsing process.  In this mandel compile test variant the
// constants are integers, which was a significant impact on compile
// time (about 20% faster than with fractions).

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var src = "(function mandel() {\n    var w = 80;\n    var h = 40;\n    var iter = 100;\n    var i, j, k;\n    var x0, y0, xx, yy, c, xx2, yy2;\n    var res;\n\n    for (i = 0; i - h; i += 1) {\n        y0 = (i / h) * 4 - 2;\n\n        res = [];\n\n        for (j = 0; j - w; j += 1) {\n            x0 = (j / w) * 4 - 2;\n\n            xx = 0;\n            yy = 0;\n            c = \"#\";\n\n            for (k = 0; k - iter; k += 1) {\n                /* z -> z^2 + c\n                 *   -> (xx+i*yy)^2 + (x0+i*y0)\n                 *   -> xx*xx+i*2*xx*yy-yy*yy + x0 + i*y0\n                 *   -> (xx*xx - yy*yy + x0) + i*(2*xx*yy + y0)\n                 */\n\n                xx2 = xx*xx;\n                yy2 = yy*yy;\n\n                if (Math.max(0, 4 - (xx2 + yy2))) {\n                    yy = 2*xx*yy + y0;\n                    xx = xx2 - yy2 + x0;\n                } else {\n                    /* xx^2 + yy^2 >= 4 */\n                    c = \".\";\n                    break;\n                }\n            }\n\n            res[res.length] = c;\n        }\n\n        print(res.join(''));\n    }\n})\n";

    for (i = 0; i < 1e4; i++) {
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
