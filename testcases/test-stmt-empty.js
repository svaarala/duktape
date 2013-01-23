/*
 *  Empty statement (E5 Section 12.3).
 */

/* FIXME: what other tests? */

/*===
undefined
===*/

try {
    var res = eval(";");
    print(res);
} catch (e) {
    print(e.name);
}

