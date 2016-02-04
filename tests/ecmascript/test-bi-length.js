/*
 *  Check .length values for builtins.
 */

/*===
7
1
===*/

// XXX: just a spot check for now

function test() {
    // .length of a top level function
    print(Date.length);

    // .length of a member function (different handling in init data)
    print(Date.prototype.setYear.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
