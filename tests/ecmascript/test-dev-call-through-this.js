/*===
g this.name: myThis
f this.name: myThis
===*/

/* Ensure that calling through 'this' preserves this binding. */

function f() {
    print('f this.name:', this.name);
}

function g() {
    print('g this.name:', this.name);
    this.f();
}

var myThis = {
    name: 'myThis',
    f: f
};

g.call(myThis);
