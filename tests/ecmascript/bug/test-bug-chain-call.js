/*===
3
===*/

var tmp = String.fromCharCode(3.4);
print(tmp.charCodeAt(0));

/*===
3
===*/

print(String.fromCharCode(3.4).charCodeAt(0));

/*===
3
===*/

/* This was broken at some point, and printed "3.4" */
function f(x) { return String.fromCharCode(x).charCodeAt(0); }
print(f(3.4));
