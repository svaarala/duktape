/*
 *  Making a constructor call to a bound constructor function differs from
 *  ordinary bound function handling slightly.  The "this" binding value
 *  provided by the bound functions is essentially ignored when making a
 *  constructor call.  That is, the final function gets as its 'this'
 *  binding the fresh Ecmascript object as usual for constructors.
 */

function func1(v1, v2, v3) {
    print('func1 this', typeof this, this);
    print('func1 args', v1, v2, v3);
    if (typeof this === 'object') {
        this.value1 = v1;
        this.value2 = v2;
        this.value3 = v3;
    }
}

var func2 = func1.bind('func2_this', 'foo');

var func3 = func2.bind('func3_this', 'bar');

/*===
normal function call to func3
func1 this object func2_this
func1 args foo bar quux
===*/

// Calling func3 as a normal function causes 'func2_this' to be the effective
// this binding.
try {
    print('normal function call to func3');
    func3('quux');
} catch (e) {
    print(e);
}

/*===
constructor call to func3
func1 this object [object Object]
func1 args foo bar quux
res.value1 foo
res.value2 bar
res.value3 quux
===*/

// Constructor call causes the fresh Ecmascript object (created in E5.1
// Section 13.2.2 step 1) to the effective this binding.
try {
    print('constructor call to func3');
    var res = new func3('quux');
    print('res.value1', res.value1);
    print('res.value2', res.value2);
    print('res.value3', res.value3);
} catch (e) {
    print(e);
}
