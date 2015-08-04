/*===
constructor as a function
string 
string undefined
string null
string true
string false
string -Infinity
string -123
string 0
string 0
string 123
string Infinity
string NaN
string 1,2
string [object Object]
string noval
===*/

print('constructor as a function');

function constructorAsFunctionTest() {
    function pv(x, noval) {
        print(typeof x, (noval ? 'noval' : x));
    }

    // No argument is handled different from undefined
    pv(String());

    pv(String(undefined));
    pv(String(null));
    pv(String(true));
    pv(String(false));
    pv(String(Number.NEGATIVE_INFINITY));
    pv(String(-123.0));
    pv(String(-0.0));
    pv(String(0.0));
    pv(String(123.0));
    pv(String(Number.POSITIVE_INFINITY));
    pv(String(Number.NaN));
    pv(String([1,2]));
    pv(String({ foo: 1, bar: 2 }));

    // the ToString() conversion of a function is implementation
    // specific so just check the type
    pv(String(function(){}), true);
}

try {
    constructorAsFunctionTest();
} catch (e) {
    print(e);
}

/*===
constructor
object  [object String] true
object undefined [object String] true
object null [object String] true
object true [object String] true
object false [object String] true
object -Infinity [object String] true
object -123 [object String] true
object 0 [object String] true
object 0 [object String] true
object 123 [object String] true
object Infinity [object String] true
object NaN [object String] true
object 1,2 [object String] true
object [object Object] [object String] true
object noval [object String] true
===*/

print('constructor');

function constructorTest() {
    function pv(x, noval) {
        print(typeof x, (noval ? 'noval' : x), Object.prototype.toString.call(x), Object.isExtensible(x));
    }

    // No argument is handled different from undefined
    pv(new String());

    pv(new String(undefined));
    pv(new String(null));
    pv(new String(true));
    pv(new String(false));
    pv(new String(Number.NEGATIVE_INFINITY));
    pv(new String(-123.0));
    pv(new String(-0.0));
    pv(new String(0.0));
    pv(new String(123.0));
    pv(new String(Number.POSITIVE_INFINITY));
    pv(new String(Number.NaN));
    pv(new String([1,2]));
    pv(new String({ foo: 1, bar: 2 }));

    // the ToString() conversion of a function is implementation
    // specific so just check the type
    pv(new String(function(){}), true);
}

try {
    constructorTest();
} catch (e) {
    print(e);
}
