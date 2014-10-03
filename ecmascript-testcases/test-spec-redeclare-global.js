/*
 *  E5.1 updated the algorithm in Section 10.5, declaration binding instantiation.
 *  Step 5.e is a new step (compared to E5) and restores compatibility to an E3
 *  idiom.  See E5.1 Annex F.
 *
 *  This testcase tests for the the updated E5.1 behavior for eval() calls.
 *  Eval creates configurable bindings.
 *
 *  Tests for Program execution context are in individual separate files,
 *  because function declarations are "hoisted" to program entry.  Thus,
 *  controlling test execution is a bit tricky.  Program code creates
 *  non-configurable bindings.
 *
 *  V8 does not pass these tests.
 */

/*
 *  Notes:
 *
 *    - The new behavior only applies if a function is being re-declared in the
 *      global environment record.  This is the case with:
 *
 *      + Program code
 *      + Indirect eval
 *
 *    - An existing property of the same name exists in the global object.
 *      Such a name can be:
 *
 *      + Another function
 *      + Any other value (like a constant)
 *
 *    - The existing property may have various attribute combinations, and may
 *      even be an accessor (set/get).
 *
 *    - The purpose of step 5.e is to ensure that the resulting property is
 *      a plain data property, with [[Writable]] and [[Enumerable]] true;
 *      [[Configurable]] final value depends on execution context.
 *
 *      + If configurable, the property is converted to a plain data property
 *        and [[Writable]] and [[Enumerable]] are set to true.  [[Configurable]]
 *        is set based on whether configurable bindings are enabled.
 *
 *      + If not configurable, the property must be a plain data property and
 *        have [[Writable]] and [[Enumerable]] set.  If not, a TypeError occurs.
 *
 *    - Note that re-declaring a global function automatically makes the property
 *      enumerable even if it earlier wasn't.
 */

function printProps(name) {
    var indirectEval = eval;
    var global = indirectEval('this;');  // indirect eval -> this = global object
    var desc = Object.getOwnPropertyDescriptor(global, name);
    if (desc.get || desc.set) {
        // Note: typeof will invoke getter here
        print(name + ': ' +
              'typeof=' + typeof global[name] + ', ' +
              'enumerable=' + desc.enumerable + ', ' +
              'configurable=' + desc.configurable);
    } else {
        print(name + ': ' +
              'typeof=' + typeof global[name] + ', ' +
              'writable=' + desc.writable + ', ' +
              'enumerable=' + desc.enumerable + ', ' +
              'configurable=' + desc.configurable);
    }
}

function defineProp(name, attrs) {
    var indirectEval = eval;
    var global = indirectEval('this;');  // indirect eval -> this = global object
    Object.defineProperty(global, name, attrs);
}

/*===
RegExp: typeof=function, writable=true, enumerable=false, configurable=true
123
RegExp: typeof=function, writable=true, enumerable=true, configurable=true
===*/

/* Existing configurable data property.
 *
 * Note that the property should become enumerable (but does not in current
 * V8).
 */

printProps('RegExp');

try {
    eval("function RegExp() { return 123; }");
    print(RegExp());
} catch (e) {
    print(e.name);
}

printProps('RegExp');

/*===
NaN: typeof=number, writable=false, enumerable=false, configurable=false
TypeError
NaN: typeof=number, writable=false, enumerable=false, configurable=false
===*/

/* Existing non-configurable data property. */

printProps('NaN');

try {
    // non-configurable and does not match requirements of step 5.e.iv -> TypeError */
    eval("function NaN() { return 234; }");
    print('never here');
} catch (e) {
    print(e.name);
}

printProps('NaN');

/*===
getter
configurableAccessor: typeof=undefined, enumerable=false, configurable=true
345
configurableAccessor: typeof=function, writable=true, enumerable=true, configurable=true
===*/

/* Existing configurable accessor.
 *
 * Need to create ourselves.
 */

defineProp('configurableAccessor', {
    get: function() { print('getter'); },
    set: function(x) { print('setter'); },
    enumerable: false,
    configurable: true
})

// Note: printProps() will invoke getter here (but not below)

printProps('configurableAccessor');

try {
    eval("function configurableAccessor() { return 345; }");
    print(configurableAccessor());
} catch (e) {
    print(e.name);
}

printProps('configurableAccessor');

/*===
getter
nonconfigurableAccessor: typeof=undefined, enumerable=false, configurable=false
TypeError
getter
nonconfigurableAccessor: typeof=undefined, enumerable=false, configurable=false
===*/

/* Existing non-configurable accessor. */

defineProp('nonconfigurableAccessor', {
    get: function() { print('getter'); },
    set: function(x) { print('setter'); },
    enumerable: false,
    configurable: false,
})

// Note: printProps() will invoke getter here and again below

printProps('nonconfigurableAccessor');

try {
    eval("function nonconfigurableAccessor() { return 456; }");
} catch (e) {
    print(e.name);
}

printProps('nonconfigurableAccessor');
