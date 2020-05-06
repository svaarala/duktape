// Repro case GH-2278, to be executed with duk-low.  Triggers a corner case in
// out-of-memory nester error handling.  The test is fragile because it must
// trigger an out-of-memory for an allocation not protected against compaction
// (e.g. allocation of an error object itself, not its property table).  The
// test may no longer trigger the error if alloc pool sizes or Duktape alloc
// patterns are changed even slightly.

/*===
===*/

function test() {
    var obj = {};
    var count = 0;
    while (count < 1e6) {
        // Limit loops, intended for duk-low only which hits the issue at around ~1270 loops.
        //print(count++);
        count++;
        obj = { ref1: obj, ref2: obj };
    }
}
try {
    test();
} catch (e) {
    print(e);
}
