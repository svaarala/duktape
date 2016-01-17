var t;

/*===
a{abc}
a{1b}
a{2,b}
===*/

// Any non-valid character cancels quantifier parsing

t = /a{.*}/.exec("aa{abc}");
print(t[0]);
t = /a{1.}/.exec("aa{1b}");
print(t[0]);
t = /a{2,.}/.exec("aa{2,b}");
print(t[0]);

/*===
a{abc}
===*/

// Closing brace is allowed
t = /a\{.*}/.exec("aa{abc}");
print(t[0]);

/*===
a{1}
a{1,2}
===*/

// Valid quantifier but for the closing brace
t = /a{1\}/.exec("aa{1}");
print(t[0]);
t = /a{1,2\}/.exec("aa{1,2}");
print(t[0]);

/*===
{1111111111111111111111111
===*/

// Do not fail on digits before , or }
t = /{1111111111111111111111111/.exec('{1111111111111111111111111');
print(t[0]);

/*===
a{}
a{,}
a{1,2,3}
===*/

//On parsing failure, treat as a brace

t = /a{}/.exec('a{}');
print(t[0]);

t = /a{,}/.exec('a{,}');
print(t[0]);

t = /a{1,2,3}/.exec('a{1,2,3}');
print(t[0]);


/*===
SyntaxError
===*/

// Current implementation does not allow all types of error

// Too many numbers
try {
    eval("/{1111111111111111111111111}/.exec('foo');");
    print("no exception");
} catch (e) {
    print(e.name);
}
