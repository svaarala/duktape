/*
 *  TextDecoder with ASCII input
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var block = 16777216;
    var loops = 50;
    var te = new TextDecoder();
    var tmp = new Uint8Array(1024);
    var buf = new Uint8Array(block);
    var i;

    var lorem_str = 'Lorem ipsum dolor sit amet, ea nam oratio partem, id augue lobortis neglegentur quo, ne sed audiam cotidieque. Cu vel causae minimum, sumo putant ea qui, ferri summo complectitur et pro. Consul propriae te eum. Vix eu quaeque assueverit, viderer sensibus no sed. Brute tempor pri et.\n\nUtinam tritani nonumes te duo. Pro ne dico inermis voluptatibus, tantas appellantur vix in. Quo id summo deterruisset, id pro audire viderer assentior. Nam eu deserunt assueverit, sonet erroribus no mei, soleat commodo accusata te vis. Per tota incorrupte suscipiantur ex, his decore audiam ne. Discere quaestio complectitur per ne, cibo audiam at mei, te etiam libris vivendo cum. Vis ne ferri iracundia euripidis, at duo commodo adolescens.';
    var lorem = new TextEncoder().encode(lorem_str);

    for (i = 0; i < tmp.length; i++) {
        tmp[i] = lorem[i % lorem.length];
    }
    //print(Duktape.enc('jx', tmp));
    for (i = 0; i < buf.length; i += 1024) {
        buf.set(tmp, i);
    }

    var t1 = Date.now();

    for (i = 0; i < loops; i++) {
        void te.decode(buf);
    }

    var t2 = Date.now();

    print((block * loops) / (t2 - t1) + ' bytes / millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
