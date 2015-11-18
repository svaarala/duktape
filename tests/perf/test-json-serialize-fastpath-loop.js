/*
 *  Loop detection in fast path.
 */

function mkloop(depth) {
    var objects = [];
    var i;

    for (i = 0; i < depth; i++) {
        objects.push({ myDepth: i });
    }

    // Link objects linearly.
    for (i = 0; i < depth - 1; i++) {
        objects[i].ref = objects[i + 1];
    }

    // No loop here, because a loop causes a fallback to slow path.

    return objects[0];
}

function test() {
    var i, j, k;
    var obj;

    // Limit to depth 64, size of visiting[] at the moment.
    for (i = 1; i <= 64; i++) {
        for (j = 0; j < i; j++) {
            obj = mkloop(i, j);

            for (k = 0; k < 100; k++) {
                try {
                    void JSON.stringify(obj, null, 4);
                } catch (e) {
                }
            }
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
