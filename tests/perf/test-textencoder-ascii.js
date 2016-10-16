/*
 *  TextEncoder with ASCII input
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var block = 16777216;
    var loops = 50;
    var te = new TextEncoder();
    var str = 'x';
    var i;

    var lorem_str = 'Lorem ipsum dolor sit amet, ea nam oratio partem, id augue lobortis neglegentur quo, ne sed audiam cotidieque. Cu vel causae minimum, sumo putant ea qui, ferri summo complectitur et pro. Consul propriae te eum. Vix eu quaeque assueverit, viderer sensibus no sed. Brute tempor pri et.\n\nUtinam tritani nonumes te duo. Pro ne dico inermis voluptatibus, tantas appellantur vix in. Quo id summo deterruisset, id pro audire viderer assentior. Nam eu deserunt assueverit, sonet erroribus no mei, soleat commodo accusata te vis. Per tota incorrupte suscipiantur ex, his decore audiam ne. Discere quaestio complectitur per ne, cibo audiam at mei, te etiam libris vivendo cum. Vis ne ferri iracundia euripidis, at duo commodo adolescens.';

    str = lorem_str;
    while (str.length < block) {
        str = str + str;
    }
    str = str.substring(0, block);

    var t1 = Date.now();

    for (i = 0; i < loops; i++) {
        void te.encode(str);
    }

    var t2 = Date.now();

    print((block * loops) / (t2 - t1) + ' codepoints / millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
