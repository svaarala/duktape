/*
 *  A naive implementation never finishes matching: '(x*)*' against the input 'y',
 *  because eventually the inner quantifier would match the empty string, which
 *  would then match forever.  Other empty quantifier cases are also tested.
 *
 *  Note that Python rejects such regexps:
 *
 *    >>> re.compile(r'(x*)*')
 *    Traceback (most recent call last):
 *      File "<stdin>", line 1, in <module>
 *      File "/usr/lib/python2.6/re.py", line 190, in compile
 *        return _compile(pattern, flags)
 *      File "/usr/lib/python2.6/re.py", line 245, in _compile
 *        raise error, v # invalid expression
 *    sre_constants.error: nothing to repeat
 *
 */

var t;

/*===
xxx xxx
 string
 string
 string
 string
===*/

/* greedy matching, (x*) will match 'xxx', outer quantifier will repeat once */
t = /(x*)*/.exec('xxx');
print(t[0], t[1]);

/* FIXME: check behavior below against E5 specification, both Rhino and Smjs fail */

/* here x* should match zero times, leaving (x*) capture the empty string */
t = /(x*)*/.exec('y');
print(t[0], typeof t[0]);
print(t[1], typeof t[1]);

/* same as above */
t = /(x*)*/.exec('');
print(t[0], typeof t[0]);
print(t[1], typeof t[1]);

/*===
xyz string
xyz string
null object
xyz string
xyz string
null object
===*/

t = /(?:(?=x)){1000}xyz/.exec('xyz');
print(t[0], typeof t[0]);

t = /(?:(?=x)){1000}xyz/.exec('xyyxyz');
print(t[0], typeof t[0]);

t = /(?:(?=x)){1000}xyz/.exec('xyy');
print(t, typeof t);

t = /(?:(?=x))+xyz/.exec('xyz');
print(t[0], typeof t[0]);

t = /(?:(?=x))+xyz/.exec('xyyxyz');
print(t[0], typeof t[0]);

t = /(?:(?=x))+xyz/.exec('xy');
print(t, typeof t);
