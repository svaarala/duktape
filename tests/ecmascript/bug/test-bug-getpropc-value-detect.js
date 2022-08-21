/*
 *  Unreleased bug between 2.1.0 and 2.2.0 release where GETPROPC special
 *  handling caused e.g. new String()() to unintentionally work.  Caught
 *  with test262.
 */

/*===
TypeError
TypeError
TypeError
TypeError
TypeError
===*/

try {
    new Boolean(true)();
} catch (e) {
    print(e.name);
}

try {
    new Number(1)();
} catch (e) {
    print(e.name);
}

try {
    new String('1')();
} catch (e) {
    print(e.name);
}

try {
    new String()();
} catch (e) {
    print(e.name);
}

try {
    new this();  // global object
} catch (e) {
    print(e.name);
}
