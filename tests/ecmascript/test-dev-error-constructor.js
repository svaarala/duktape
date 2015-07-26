/*
 *  Simple cases for Error constructor
 */

var e;

/*===
string

string

===*/

e = new Error(undefined);
print(typeof e.message);  /* should be inherited from prototype, empty string */
print(e.message);

// same behavior if called as function
e = Error(undefined);
print(typeof e.message);
print(e.message);

/*===
foo
foo
===*/

e = new Error('foo');
print(e.message);

e = Error('foo');
print(e.message);

/*===
string null
string null
string 123
string 123
===*/

/* ToString() coercion */

e = new Error(null);
print(typeof e.message, e.message);

e = Error(null);
print(typeof e.message, e.message);

e = new Error(123);
print(typeof e.message, e.message);

e = Error(123);
print(typeof e.message, e.message);
