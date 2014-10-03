/* FIXME: bound function cases */

/*
 *  Parsing tests for 'new' expressions.  These are rather detailed because
 *  the grammar for the following productions is a bit awkward:
 *
 *    - LeftHandSideExpression
 *    - CallExpression
 *    - NewExpression
 *    - MemberExpression
 */

var t;

/*===
Basic1
Basic1
Basic2 arg1 arg2
Basic3 arg1 arg2
Basic3 arg1 arg2
Basic4 arg1 arg2
===*/

/* Basic cases where constructor is from a MemberExpression (but not another 'new') */

function Basic1() {
}
Basic1.prototype = { 'name': 'Basic1' };

function Basic2(x,y) {
    this.x = x;
    this.y = y;
}
Basic2.prototype = { 'name': 'Basic2' };

basic3 = {
    'constructor': function(x,y) {
                      this.x = x;
                      this.y = y;
                   }
};
basic3.constructor.prototype = { 'name': 'Basic3' };

try {
    // without parenthesis (arguments)
    eval("t = new Basic1; print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    // with no arguments (semantically same as above)
    eval("t = new Basic1(); print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    // with arguments
    eval("t = new Basic2('arg1', 'arg2'); print(t.name, t.x, t.y);");
} catch (e) {
    print(e.name);
}

try {
    // constructor from a property access (MemberExpression)
    eval("t = new basic3.constructor ('arg1', 'arg2'); print(t.name, t.x, t.y);");
} catch (e) {
    print(e.name);
}

try {
    // constructor from a property access (MemberExpression), bracket syntax
    eval("t = new basic3['constructor'] ('arg1', 'arg2'); print(t.name, t.x, t.y);");
} catch (e) {
    print(e.name);
}

try {
    // constructor from a function expression (from MemberExpression)
    // Note: here the 'name' property is not inherited from prototype
    eval("t = new function(x,y) {this.x=x;this.y=y;this.name='Basic4';} ('arg1', 'arg2'); print(t.name, t.x, t.y);");
} catch (e) {
    print(e.name);
}

/*===
TypeError
getTarget/f
===*/

/* The target for 'new' may be any MemberExpression, but it cannot be a
 * CallExpression.  Thus:
 *
 *   new foo() ()
 *
 * Will be parsed as ( new foo() ) ().  For the intended effect, parenthesis
 * are needed:
 *
 *   new (foo()) ()
 */

function getTarget() {
    var f = function() {
    }
    f.prototype = { 'name': 'getTarget/f' };
    return f;
}

try {
    // this will evaluate "new getTarget ()" and then attempt to call the
    // instance, which is not callable -> TypeError
    eval("t = new getTarget() (); print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    // this will first evaluate "getTarget()" to get the constructor,
    // and then call the constructor
    eval("t = new (getTarget()) (); print(t.name);");
} catch (e) {
    print(e.name);
}

/*===
NewNew2 arg1
===*/

/* The 'new' operator may occur multiple times, although this is
 * quite unusual.  It binds more loosely than MemberExpressions
 * and acts right associative.
 *
 * Example below:
 *
 *      new new Foo()
 *  === new (new Foo())
 *
 * This is derived as (using parenthesis for illustration):
 *
 *  NewExpression => new NewExpression
 *                => new (MemberExpression)
 *                => new (new MemberExpression Arguments)
 *                => new (new Foo ())
 */

function NewNew1(x) {
    function f() {
        // snatch 'x' from closure
        this.x = x;
    }
    f.prototype = { 'name': 'NewNew2' };
    return f;
}
NewNew1.prototype = { 'name': 'NewNew1' };

try {
    eval("t = new new NewNew1 ('arg1'); print(t.name, t.x);");
} catch (e) {
    print(e.name);
}

/*===
Bal1A called, this.name=Bal1A
Bal1A
Bal1A called, this.name=Bal1A
Bal1A
Bal1B called, this.name=Bal1B
Bal1B/f called, this.Number.POSITIVE_INFINITY=Infinity
Bal1B/f return value
Bal1C called, this.name=Bal1C
Bal1C/f called, this.Number.POSITIVE_INFINITY=Infinity
Bal1C/f/g called, this.Number.POSITIVE_INFINITY=Infinity
Bal1C/f/g return value
===*/

/*
 *  Parenthesis "balance tests".
 *
 *  new Foo                      basic case
 *  new Foo ()                   parens associated with 'new', not a func call
 *  new Foo () ()                (new Foo ()) ()
 *  new Foo () () ()             ((new Foo ()) ()) ()
 */

function Bal1A() {
    // printing out this.name indicates whether we're being
    // called as a constructor or as a function
    print("Bal1A called, this.name=" + this.name);
}
Bal1A.prototype = { 'name': 'Bal1A' };

function Bal1B() {
    print("Bal1B called, this.name=" + this.name);

    function f() {
        // f() is called as a normal function, this is bound to the
        // global object; printing out a specific value accessible
        // through the global object proves the point.

        print("Bal1B/f called, this.Number.POSITIVE_INFINITY=" + this.Number.POSITIVE_INFINITY);
        return "Bal1B/f return value";
    }
    f.name = "Bal1B/f";  // not inherited

    // Returned value replaces default constructed object; this return
    // value does not have Bal1B.prototype as its internal prototype.
    return f;
}
Bal1B.prototype = { 'name': 'Bal1B' };  // has no effect

function Bal1C() {
    print("Bal1C called, this.name=" + this.name);

    function f() {
        print("Bal1C/f called, this.Number.POSITIVE_INFINITY=" + this.Number.POSITIVE_INFINITY);
        function g() {
            print("Bal1C/f/g called, this.Number.POSITIVE_INFINITY=" + this.Number.POSITIVE_INFINITY);
            return "Bal1C/f/g return value";
        }
        return g;
    }
    f.name = "Bal1C/f";  // not inherited
    return f;
}
Bal1C.prototype = { 'name': 'Bal1C' };  // has no effect

try {
    eval("var t = new Bal1A; print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    eval("var t = new Bal1A (); print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    eval("var t = new Bal1B () (); print(t);");     // here 't' is return value from Bal1B/f
} catch (e) {
    print(e.name);
}

try {
    eval("var t = new Bal1C () () (); print(t);");
} catch (e) {
    print(e.name);
}

/*===
Bal2A called, this.name=Bal2A
Bal2A/f called, this.name=Bal2A/f
Bal2A/f
Bal2B called, this.name=Bal2B
Bal2B/f called, this.name=Bal2B/f
Bal2B/f/g called, this.name=Bal2B/f/g
Bal2B/f/g
Bal2B called, this.name=Bal2B
Bal2B/f called, this.name=Bal2B/f
Bal2B/f/g called, this.name=Bal2B/f/g
Bal2B/f/g
Bal2B called, this.name=Bal2B
Bal2B/f called, this.name=Bal2B/f
Bal2B/f/g called, this.name=Bal2B/f/g
Bal2B/f/g
Bal2B called, this.name=Bal2B
Bal2B/f called, this.name=Bal2B/f
Bal2B/f/g called, this.name=Bal2B/f/g
Bal2B/f/g
Bal2C called, this.name=Bal2C
Bal2C/f called, this.name=Bal2C/f
Bal2C/f/g called, this.name=Bal2C/f/g
Bal2C/f/g/h called, this.Number.POSITIVE_INFINITY=Infinity
Bal2C/f/g/h return value
===*/

/*
 *  Parenthesis and 'new' "balance tests".
 *
 *  new new Foo                  new (new Foo)
 *  new new new Foo              new (new (new Foo))
 *  new new new Foo ()           new (new (new Foo ()))
 *  new new new Foo () ()        new (new (new Foo ()) ())
 *  new new new Foo () () ()     new (new (new Foo ()) ()) ()         up to this point, parens associated with 'new' calls
 *  new new new Foo () () () ()  (new (new (new Foo ()) ()) ()) ()    last is a func call
 */

function Bal2A() {
    print('Bal2A called, this.name=' + this.name);

    function f() {
        print("Bal2A/f called, this.name=" + this.name);
    }
    f.prototype = { 'name': 'Bal2A/f' };

    // return value (instance) of inner ("new Bal2A") is used as the
    // constructor for the outer "new" call.
    return f;
}
Bal2A.prototype = { 'name': 'Bal2A' };

function Bal2B() {
    print('Bal2B called, this.name=' + this.name);

    function f() {
        print('Bal2B/f called, this.name=' + this.name);

        function g() {
            print('Bal2B/f/g called, this.name=' + this.name);
        }
        g.prototype = { 'name': 'Bal2B/f/g' };

        return g;
    }
    f.prototype = { 'name': 'Bal2B/f' };

    return f;
}
Bal2B.prototype = { 'name': 'Bal2B' };

function Bal2C() {
    print('Bal2C called, this.name=' + this.name);

    function f() {
        print('Bal2C/f called, this.name=' + this.name);

        function g() {
            print('Bal2C/f/g called, this.name=' + this.name);

            function h() {
                print('Bal2C/f/g/h called, this.Number.POSITIVE_INFINITY=' + this.Number.POSITIVE_INFINITY);
                return 'Bal2C/f/g/h return value';
            }

            return h;
        }
        g.prototype = { 'name': 'Bal2C/f/g' };

        return g;
    }
    f.prototype = { 'name': 'Bal2C/f' };

    return f;
}
Bal2C.prototype = { 'name': 'Bal2C' };

try {
    eval("t = new new Bal2A; print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    eval("t = new new new Bal2B; print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    eval("t = new new new Bal2B (); print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    eval("t = new new new Bal2B () (); print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    eval("t = new new new Bal2B () () (); print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    eval("t = new new new Bal2C () () () (); print(t);");
} catch (e) {
    print(e.name);
}

/*===
Bal3A called, this.name=Bal3A, x=1, y=2, z=3
Bal3A/f called, this.name=Bal3A/f, x=4, y=5
z is from outer scope: z=3
Bal3A/f/g called, this.Number.POSITIVE_INFINITY=Infinity, a=6, b=7
the following are from outer scope: x=4, y=5, z=3
Bal3A/f/g return value
===*/

/* Parenthesis association test with argument values:
 *
 * new new Foo (1,2,3) (4,5) (6,7)    last parens are a call
 */

function Bal3A(x,y,z) {
    print("Bal3A called, this.name=" + this.name +
          ", x=" + x + ", y=" + y + ", z=" + z);

    function f(x,y) {
        print("Bal3A/f called, this.name=" + this.name +
              ", x=" + x + ", y=" + y);
        print("z is from outer scope: z=" + z);

        function g(a,b) {
            print("Bal3A/f/g called, this.Number.POSITIVE_INFINITY=" + this.Number.POSITIVE_INFINITY +
                  ", a=" + a + ", b=" + b);
            print("the following are from outer scope: " +
                  "x=" + x + ", y=" + y + ", z=" + z);

            return "Bal3A/f/g return value";
        }

        // replace created object with function g
        return g;
    }
    f.prototype = { 'name': 'Bal3A/f' };

    return f;
}
Bal3A.prototype = { 'name': 'Bal3A' };

try {
    eval("t = new new Bal3A (1,2,3) (4,5) (6,7); print(t);");
} catch (e) {
    print(e.name);
}

/*===
Misc1 called, this.name=Misc1
foo getter called
Misc1/f called, this.name=Misc1/f
Misc1/f
Misc1 called, this.name=Misc1
foo getter called
Misc1/f called, this.name=Misc1/f
Misc1/f
Misc2 called, this.name=Misc2
foo getter called
Misc2/f called, this.name=Misc2/f, x=1, y=2
Misc2/f
===*/

/*
 *  Consider the following:
 *
 *    new new Foo () . foo
 *
 *  If we start from LeftHandSideExpression and choose CallExpression,
 *  it is not possible to create any 'new' expression without arguments,
 *  like the outer 'new' above.  More specifically:
 *
 *    LeftHandSideExpression => CallExpression
 *                           => CallExpression '.' IdentifierName
 *                           => MemberExpression '.' IdentifierName
 *
 *    At this point there is no way to expand MemberExpression and create
 *    two 'new' tokens and only one set of arguments.
 *
 *  If we start from LeftHandSideExpression and choose NewExpression
 *  (brackets for emphasis):
 *
 *    LeftHandSideExpression => NewExpression
 *                           => 'new' NewExpression
 *                           => 'new' MemberExpression
 *                           => 'new' [ MemberExpression '.' IdentifierName ]
 *                           => 'new' [ [ 'new' MemberExpression Arguments ] '.' IdentifierName ]
 *                           => 'new' [ [ 'new' 'Foo' '(' ')' ] '.' 'foo' ]
 *
 *  So, in other words, we get:
 *
 *    new new Foo () . foo == new ((new Foo ()) . foo)
 *
 *  And:
 *
 *    new new Foo () . foo () == new ((new Foo ()) . foo) ()
 *
 *  where the last parens are related to the outer 'new'.
 */

function Misc1() {
    print('Misc1 called, this.name=' + this.name);

    function f() {
        print('Misc1/f called, this.name=' + this.name);
    }
    f.prototype = { 'name': 'Misc1/f' };

    // return an object whose 'foo' property is read by the caller
    // and used as the next constructor
    return { get foo() { print ('foo getter called'); return f } };
}
Misc1.prototype = { 'name': 'Misc1' };

function Misc2() {
    print('Misc2 called, this.name=' + this.name);

    function f(x,y) {
        print('Misc2/f called, this.name=' + this.name +
              ', x=' + x + ', y=' + y);
    }
    f.prototype = { 'name': 'Misc2/f' };

    // return an object whose 'foo' property is read by the caller
    // and used as the next constructor
    return { get foo() { print ('foo getter called'); return f } };
}
Misc2.prototype = { 'name': 'Misc2' };

try {
    eval("t = new new Misc1 () . foo; print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    // exactly same, explicit parens
    eval("t = new ((new Misc1 ()) . foo); print(t.name);");
} catch (e) {
    print(e.name);
}

try {
    // add arguments to outer 'new'
    eval("t = new new Misc2 () . foo (1,2); print(t.name);");
} catch (e) {
    print(e.name);
}

/*===
1 2 3 4
1 2 undefined undefined
true
===*/

/* If a constructor returns a replacement value, the internal prototype
 * of that object will not be set by the 'new' call.
 */
function Cons1() {
    this.foo = 1;
    this.bar = 2;
}
Cons1.prototype = { quux: 3, baz: 4 };

function Cons2() {
    return { foo: 1, bar: 2 };
}
Cons2.prototype = { quux: 3, baz: 4 };

try {
    t = new Cons1();
    print(t.foo, t.bar, t.quux, t.baz);

    // the internal prototype of 't' will be Object.prototype here
    t = new Cons2();
    print(t.foo, t.bar, t.quux, t.baz);
    print(Object.getPrototypeOf(t) === Object.prototype);
} catch (e) {
    print(e.name);
}
