/*===
"\t\u000b\f \u00a0\ufeff\u1680\u180e\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u202f\u205f\u3000\n\r\u2028\u2029123.0"
"\u180e\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u202f\u205f\u3000\n\r\u2028\u2029123.0"
"\t\u000b\f \u00a0\ufeff\u1680\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u202f\u205f\u3000\n\r\u2028\u2029123.0"
"123.0"
===*/

function sanitize(v) {
    return v.replace(/[\u007f-\uffff]/g, function (c) { return '\\u' + ('0000' + c.charCodeAt(0).toString(16)).substr(-4); });
}

var input = "\t\u000b\f \u00a0\ufeff\u1680\u180e\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u202f\u205f\u3000\n\r\u2028\u2029123.0";
print(sanitize(JSON.stringify(input)));
print(sanitize(JSON.stringify(input.trim())));

var input = "\t\u000b\f \u00a0\ufeff\u1680\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u202f\u205f\u3000\n\r\u2028\u2029123.0";
print(sanitize(JSON.stringify(input)));
print(sanitize(JSON.stringify(input.trim())));
