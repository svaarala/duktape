/*
 *  Basic regression harness for refcount leaks
 *
 *  Execute this test manually:
 *
 *      # Execute main test
 *      $ valgrind ./duk --no-heap-destroy test-dev-refcount-leak-basic.js
 *
 *      # Set LOOP_COUNT to 1 manually and re-run the test
 *      $ valgrind ./duk --no-heap-destroy test-dev-refcount-leak-basic.js
 *
 *  Inspect the final "still allocated" to ensure that in botj cases we arrive at
 *  the same baseline allocation at the end.  There's a large loop count in this
 *  test to magnify any leaks.  Using LOOP_COUNT 0 is OK but there may be small
 *  differences due to the value stack and/or call stack having a different size
 *  (which manifests as a final leak difference of ~100-1000 bytes).
 *
 *  You can also compare to "hello world"; it won't be an exact match if
 *  mark-and-sweep is disabled, but close:
 *
 *      # Compare to 'hello world'
 *      $ valgrind ./duk --no-heap-destroy test-dev-hello-world.js
 *
 *  The test function tries to exercise all places in code where refcounts
 *  are increased/decreased.  It's not 100%, but together with other tests
 *  should provide a reasonable regression harness.  The basic coverage here
 *  is based on:
 *
 *      $ cd src/; grep DECREF UPDREF *.c > /tmp/worklist
 *
 *  Run the test with and without fastint support because that affects the
 *  code paths executed.  It's best to test with mark-and-sweep disabled so
 *  that garbage with broken refcounts won't get collected.
 *
 *  NOTE! To avoid circular references, all functions are made cycle free by
 *  forcing .prototype to null.  If this is not done, and mark-and-sweep is
 *  disabled, all functions will be leaks.
 *
 *  NOTE! Leaving function instances (even cycle free ones) in the function
 *  registers when the function exits leads to a circular reference: the
 *  environment will be closed which creates an environment object referring
 *  to the function instance, and the function instance refers back to the
 *  environment.  The testcases nullify locals on exit to avoid this issue
 *  (not a bug but affects refcount testing).
 */

/*---
{
    "custom": true
}
---*/

/*===
exiting
===*/

this.T = {};
this.T.LOOP_COUNT = 10000;  // <== EDIT MANUALLY between 10000 and 1

this.T.executorTest = function executorTest() {
    /*
     *  duk_js_executor.c
     */

    var x, y, z;

    // Catch variable
    try { throw new Error('aiee'); } catch (e) { void e; }

    // Finally catch register slots
    do { try { break; } finally { void 0; } } while (0);
    do { try { continue; } finally { void 0; } } while (0);
    do { try { try { throw new Error('aiee'); } finally { void 0; } } catch (e) { void e; } } while (0);
    do { try { try { return 'aiee'; } finally { throw new Error('cancel return'); } } catch (e) { void e; } } while (0);

    // Return value handling
    x = function () { return 123; }; x.prototype = null; z = {}; z = x();
    x = function () { return 123; }; x.prototype = null;
    y = function () { return x(); }; y.prototype = null;  // tail call
    z = {}; z = y();
    z = {}; z = Math.min(123, 234);

    // Resume and yield
    x = function () { Duktape.Thread.yield(new Date()); }; x.prototype = null;
    y = new Duktape.Thread(x);
    z = {}; z = Duktape.Thread.resume(y);

    // With binding
    with({ foo: 'bar' }) { void foo; }

    // Addition fastint/number fast path; slow path
    z = {}; x = 10; y = 20; z = x + y;
    z = {}; x = 10.1; y = 20.1; z = x + y;
    z = {}; x = ''; y = 20; z = x + y;

    // Binary arithematic fastint/number fast path; slow path
    z = {}; x = 10; y = 20; z = x - y;
    z = {}; x = 10; y = 20; z = x * y;
    z = {}; x = 10; y = 20; z = x / y;
    z = {}; x = 10; y = 20; z = x % y;
    z = {}; x = 10.1; y = 20.1; z = x - y;
    z = {}; x = 10.1; y = 20.1; z = x * y;
    z = {}; x = 10.1; y = 20.1; z = x / y;
    z = {}; x = 10.1; y = 20.1; z = x % y;
    z = {}; x = '100'; y = 20.1; z = x - y;
    z = {}; x = '100'; y = 20.1; z = x * y;
    z = {}; x = '100'; y = 20.1; z = x / y;
    z = {}; x = '100'; y = 20.1; z = x % y;

    // Bitwise binary ops
    z = {}; x = 10; y = 20; z = x & y;
    z = {}; x = 10; y = 20; z = x | y;
    z = {}; x = 10; y = 20; z = x ^ y;
    z = {}; x = 10; y = 20; z = x << y;
    z = {}; x = 10; y = 20; z = x >> y;
    z = {}; x = 10; y = 20; z = x >>> y;
    z = {}; x = 10.1; y = 20.1; z = x & y;
    z = {}; x = 10.1; y = 20.1; z = x | y;
    z = {}; x = 10.1; y = 20.1; z = x ^ y;
    z = {}; x = 10.1; y = 20.1; z = x << y;
    z = {}; x = 10.1; y = 20.1; z = x >> y;
    z = {}; x = 10.1; y = 20.1; z = x >>> y;
    z = {}; x = '10.1'; y = 20.1; z = x & y;
    z = {}; x = '10.1'; y = 20.1; z = x | y;
    z = {}; x = '10.1'; y = 20.1; z = x ^ y;
    z = {}; x = '10.1'; y = 20.1; z = x << y;
    z = {}; x = '10.1'; y = 20.1; z = x >> y;
    z = {}; x = '10.1'; y = 20.1; z = x >>> y;

    // Unary arithmetic
    z = {}; x = 10; z = +x;
    z = {}; x = 10; z = -x;
    z = {}; x = 10.1; z = +x;
    z = {}; x = 10.1; z = -x;
    z = {}; x = '100'; z = +x;
    z = {}; x = '100'; z = -x;

    // Bitwise unary op(s)
    z = {}; x = 10; z = ~x;
    z = {}; x = 10.1; z = ~x;
    z = {}; x = '100'; z = ~x;

    // Pre/post inc/dec
    x = {}; z = x++;
    x = {}; z = x--;
    x = {}; z = ++x;
    x = {}; z = --x;

    // Misc opcode exercises
    x = 123.1; z = {}; z = x;  // LDCONST, LDREG
    x = 123; z = {}; z = x;    // LDINT, LDREG
    z = {}; z = this;          // LDTHIS
    z = {}; z = void 0;        // LDUNDEF
    z = {}; z = null;          // LDNULL
    z = {}; z = true;          // LDTRUE
    z = {}; z = false;         // LDFALSE
    z = []; z = {};            // NEWOBJ
    z = {}; z = [];            // NEWARR
    z = [1,2,3,,,,];           // SETALEN

    // Without this any functions left behind will be copied into the
    // final environment record.  The functions will then be in a circular
    // reference with their environment and uncollectable.  This is not an
    // actual leak; break the loop here for measurement.

    x = y = z = null;
};
this.T.executorTest.prototype = null;  // break circular ref

this.T.objectMiscTest = function objectMiscTest() {
    var x, y, z;
    var setter, getter;

    // duk_hobject_props.c: array write shallow fast path
    x = [ {}, {} ];
    x[0] = {};
    x[1] = 123;

    // duk_hobject_props.c: make array shorter, fast path array part
    x = [ {}, {}, {}, {} ];
    x.length = 2;

    // duk_hobject_props.c: make array shorter, abandoned array
    x = [ {}, {}, {}, {} ];
    x[1000] = {};
    x.length = 2;

    // duk_hobject_props.c: overwrite a data property (own or inherited)
    x = { prop: {} }; y = { inherit: {} };
    Object.setPrototypeOf(x, y);
    x.prop = {};
    x.inherit = {};

    // duk_hobject_misc.c: duk_hobject_set_prototype()
    x = {};
    Object.setPrototypeOf(x, {});
    Object.setPrototypeOf(x, {});

    // duk_hobject_props.c: duk__realloc_props() abandon array
    a = [{}, {}, {}];
    a[1000] = {};

    // duk_hobject_props.c: duk_hobject_delprop_raw() delete array elem
    x = [ {}, {} ];
    delete x[0];
    delete x[1];

    // duk_hobject_props.c: duk_hobject_delprop_raw() delete array elem, abandoned array
    x = [ {}, {} ];
    x[1000] = {};
    delete x[0];
    delete x[1];

    // duk_hobject_props.c: duk_hobject_delprop_raw() delete accessor
    x = {};
    setter = function setter() {}; setter.prototype = null;
    getter = function getter() {}; getter.prototype = null;

    Object.defineProperty(x, 'prop', {
        set: setter, get: getter, configurable: true
    });
    delete x.prop;

    // duk_hobject_props.c: duk_hobject_delprop_raw() delete normal
    x = {};
    x.prop = {};
    delete x.prop;

    // duk_hobject_props.c: duk_hobject_define_property_helper,
    // overwrite data property
    x = {};
    x.prop = {};
    Object.defineProperty(x, 'prop', { value: {} });

    // duk_hobject_props.c: duk_hobject_define_property_helper,
    // overwrite accessor property
    x = {};
    setter = function () {}; setter.prototype = null;
    getter = function () {}; getter.prototype = null;
    Object.defineProperty(x, 'prop', {
        set: setter, get: getter, configurable: true
    });
    setter = function () {}; setter.prototype = null;
    getter = function () {}; getter.prototype = null;
    Object.defineProperty(x, 'prop', {
        set: setter, get: getter, configurable: true
    });

    // duk_hobject_props.c: duk_hobject_define_property_helper,
    // convert data property to accessor
    x = {};
    x.prop = {};
    setter = function () {}; setter.prototype = null;
    getter = function () {}; getter.prototype = null;
    Object.defineProperty(x, 'prop', {
        set: setter, get: getter, configurable: true
    });

    // duk_hobject_props.c: duk_hobject_define_property_helper,
    // convert accessor to data property
    x = {};
    setter = function () {}; setter.prototype = null;
    getter = function () {}; getter.prototype = null;
    Object.defineProperty(x, 'prop', {
        set: setter, get: getter, configurable: true
    });
    Object.defineProperty(x, 'prop', { value: {} });

    // Constructor calls
    x = function () {}; x.prototype = { foo: 123 };  // constructor
    y = new x();

    // Avoid closure circular ref
    x = y = z = setter = getter = null;
};
this.T.objectMiscTest.prototype = null;  // break circular ref

this.T.miscTest = function miscTest() {
    var x, y, z;

    // duk_hobject_misc.c: duk_hobject_set_prototype()
    x = {};
    Object.setPrototypeOf(x, {});
    Object.setPrototypeOf(x, {});

    // DECLVAR over a data property
    x = {}; eval('var x = {};');
    with ({ foo: {} }) { eval('var foo = {};'); }

    // DECLVAR over an accessor property
    x = {};
    setter = function () {}; setter.prototype = null;
    getter = function () {}; getter.prototype = null;
    Object.defineProperty(x, 'foo', { set: setter, get: getter, configurable: true });
    with (x) { eval('var foo = {};'); }

    // PUTVAR (slow path) to a data property
    x = {}; eval('x = {};');
    with ({ foo: {} }) { eval('foo = {};'); }

    // PUTVAR (slow path) to an accessor property
    x = {};
    setter = function () {}; setter.prototype = null;
    getter = function () {}; getter.prototype = null;
    Object.defineProperty(x, 'foo', { set: setter, get: getter, configurable: true });
    with (x) { eval('foo = {};'); }

    // TypedArray .slice() prototype updref
    x = new Uint8Array(16);
    y = x.subarray(4, 7);

    // RegExp creation
    x = /foo/g;

    // Avoid closure circular ref
    x = y = z = setter = getter = null;
};
this.T.miscTest.prototype = null;  // break circular ref

this.T.test = function test() {
    for (var i = 0; i < T.LOOP_COUNT; i++) {
        T.executorTest();
        T.objectMiscTest();
        T.miscTest();

        /* XXX: missing coverage:
         *
         *   - duk_api_heap.c: duk_set_global_object()
         *   - duk_debugger.c: duk_debug_remove_breakpoint()
         *   - duk_hthread_stacks.c: duk_hthread_callstack_unwind, .caller unwind
         *   - duk_api_bytecode.c: prototype updref
         *   - duk_api_stack.c: duk_push_buffer_object()
         *   - duk_regexp_compiler.c: regexp prototype (refcount never reaches zero
         *     because there's a built-in reference, so can't detect leaks)
         *   - duk_hthread_builtins.c: built-in property references in general
         *     (refcount never reaches zero so can't detect leaks)
         *
         * XXX: missing explicit coverage (but have indirect coverage most likely):
         *
         *   - duk_api_stack.c: duk_replace()
         *   - duk_api_stack.c: duk_remove()
         *   - duk_api_stack.c: integer coercions
         *   - duk_api_stack.c: duk_set_top(), duk_pop(), duk_pop_n()
         *   - duk_api_stack.c: duk_copy(), duk_to_undefined(), duk_to_null(),
         *                      duk_to_boolean(), duk_to_number()
         *   - duk_hthread_stacks.c: duk_hthread_callstack_unwind, decrefs for
         *                           lex_env, var_env, func
         *   - duk_js_call.c: thr->heap->lj.value1/2 decrefs
         *   - duk_js_call.c: tailcall 'this' binding copy
         *   - duk_js_compiler.c: varmap cleanup
         *   - duk_hobject_props.c: duk_hobject_define_property_internal()
         *   - duk_hobject_props.c: duk_hobject_define_property_internal_arridx()
         */
    }
};
this.T.test.prototype = null;  // break circular ref

try {
    T.test();

    // Delete bindings and compact global object.  Important to delete
    // and not just set to null.
    delete T;

    // Compact the global object to match test-dev-hello-world.js;
    // built-ins including global object are compacted on creation
    // so the result should be the same.
    Duktape.compact(this);

    // Global object should now be in its original state.

    // *DO NOT* run mark-and-sweep here: if you do, garbage with broken
    // refcounts will be collected which we don't want!  It's best to
    // test with mark-and-sweep disabled.
    //print(Object.getOwnPropertyNames(this));
    print('exiting');
} catch (e) {
    print(e.stack || e);
}
