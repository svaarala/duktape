/*
 *  Loop detection in slow path.
 */

// identity replacer to forcibly avoid fast path
function id(k, v) {
    return v;
}

function mkloop(depth, loopDepth) {
    var objects = [];
    var i;

    for (i = 0; i < depth; i++) {
        objects.push({ myDepth: i });
    }

    // Link objects linearly.
    for (i = 0; i < depth - 1; i++) {
        objects[i].ref = objects[i + 1];
    }

    // Add loop link to specified object.
    objects[depth - 1].loopRef = objects[loopDepth];

    return objects[0];
}

function test() {
    var i, j, j_step, k;
    var obj;

    for (i = 1; i < 100;) {
        j_step = (i <= 40 ? 1 : i >>> 2);

        for (j = 0; j < i; j += j_step) {
            obj = mkloop(i, j);

            for (k = 0; k < 100; k++) {
                try {
                    void JSON.stringify(obj, id, 4);
                } catch (e) {
                }
            }
        }

        if (i <= 40) {
            i++;
        } else {
            i += 11;
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
