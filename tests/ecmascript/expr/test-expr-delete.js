/*
 *  'delete' operator (E5 Section 11.4.1).
 */

/*===
boolean true
number 3
boolean true
boolean true
boolean true
SyntaxError
SyntaxError
false
boolean true
true
boolean false
TypeError
boolean false
SyntaxError
undefined
boolean true
SyntaxError
===*/

function evalTest(code) {
    try {
        var t = eval(code);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
        //print(e);
    }
}

// delete for anything but a Reference is 'true'
evalTest('delete (1 + 2);');
evalTest('delete 1 + 2;');  // 'delete 1' evaluates to true, true + 2 = 3!
evalTest('delete function () {};');

// unresolvable reference in non-strict code is 'true'
// unresolvable reference in strict code is a SyntaxError
evalTest('delete noSuchRef;');
evalTest('(function () { return delete noSuchRef; })()');
evalTest('"use strict"; delete noSuchRef;');
evalTest('(function () { "use strict"; return delete noSuchRef; })()');

// property reference, configurable properties can be deleted
evalTest('var obj = { foo: 123 }; var rc = delete obj.foo; print("foo" in obj); rc;');

// property reference, non-configurable properties cannot be deleted
// non-strict mode: false, strict mode: TypeError
evalTest('var obj={}; Object.defineProperty(obj, "foo", { value: 123, writable: true, enumerable: true, configurable: false }); var rc = delete obj.foo; print("foo" in obj); rc;');
evalTest('"use strict"; var obj={}; Object.defineProperty(obj, "foo", { value: 123, writable: true, enumerable: true, configurable: false }); var rc = delete obj.foo; print("foo" in obj); rc;');

// identifier reference, ordinary bindings cannot be deleted because
// they're not deletable;
// non-strict mode: false, strict mode: SyntaxError
evalTest('(function () { var x; return delete x; })()');
evalTest('(function () { "use strict"; var x; return delete x; })()');

// identifier reference, eval-established bindings can be deleted in non-strict mode
// non-strict mode: true, allowed
// strict mode: SyntaxError (even if binding is deletable!)
evalTest('(function () { eval("var x = 123;"); var rc = delete x; print(typeof x); return rc; })()');
evalTest('(function () { "use strict"; eval("var x = 123;"); var rc = delete x; print(typeof x); return rc; })()');
