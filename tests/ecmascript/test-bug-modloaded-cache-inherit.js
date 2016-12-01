/*
 *  In Duktape 1.5.1 Duktape.modLoaded[] inherited from Object.prototype
 *  which caused e.g. require('toString') to return undefined without
 *  trying to load a module.
 */

/*===
TypeError
===*/

// Without a module loader, expected result is TypeError.
try {
    require('toString');
    print('never here');
} catch (e) {
    print(e.name);
}
