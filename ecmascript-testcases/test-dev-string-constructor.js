var obj;

function prS(x) {
    print(typeof x, x);
}

function prO(x) {
    print(typeof x, x.toString());
}

/*===
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
string 
string foo
string obj.toString()
string 1,2,3
===*/

/* Called as a function, returns string (not object).
 *
 * Note that implementation MUST distinguish between an argument "not given",
 * vs. "given as undefined".  This is currently handled by using varargs.
 */

// empty string result
prS(String());
prS(String(undefined));

prS(String(null));
prS(String(true));
prS(String(false));

prS(String(Number.NEGATIVE_INFINITY));
prS(String(-123));
prS(String(-0));
prS(String(+0));
prS(String(+123));
prS(String(Number.POSITIVE_INFINITY));

prS(String(''));
prS(String('foo'));

obj = {
    toString: function() { return 'obj.toString()' }
}
prS(String(obj));

prS(String([1,2,3]));

/*===
object 
object undefined
object null
object true
object false
object -Infinity
object -123
object 0
object 0
object 123
object Infinity
object 
object foo
object obj.toString()
object 1,2,3
===*/

/* Called as a constructor, returns String object */

// empty string result
prS(new String());
prS(new String(undefined));

prS(new String(null));
prS(new String(true));
prS(new String(false));

prS(new String(Number.NEGATIVE_INFINITY));
prS(new String(-123));
prS(new String(-0));
prS(new String(+0));
prS(new String(+123));
prS(new String(Number.POSITIVE_INFINITY));

prS(new String(''));
prS(new String('foo'));

obj = {
    toString: function() { return 'obj.toString()' }
}
prS(new String(obj));

prS(new String([1,2,3]));
