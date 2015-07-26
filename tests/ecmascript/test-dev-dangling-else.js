/*===
d
c
b
a
===*/

function f(a,b,c) {
	if (a) {
		print('a');
	} else if (b) {
		print('b');
	} else if (c) {
		print('c');
	} else {
		print('d');
	}
}

f(false, false, false);
f(false, false, true);
f(false, true, false);
f(true, false, false);
