/*===
foo,foo
===*/

/* This used to have a bug: closure numbers would get incorrectly used. */

function F() {
	this.values = [];
}
F.prototype.f = function() {
	this.values[this.values.length] = 'foo';
}
F.prototype.g = function() {
	this.values[this.values.length] = 'bar';
}

function test() {
	var obj = new F();
	obj.f();
	obj.f();
	print(obj.values);
}

test();
