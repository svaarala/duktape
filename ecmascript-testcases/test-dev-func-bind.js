var func;

/*===

1 2
1 2 3 4
1 2 3
===*/

/* Basic cases */
print.bind(null)();
print.bind(null,1,2)();
print.bind(null,1,2)(3,4);
print.bind(null,1).bind(null,2).bind(null,3)();

/*===
2
2
1
0
0
===*/

/* XXX: length handling of bound function, improve */

function f1(x,y) {
}

print(f1.length);
func = f1.bind(null);
print(func.length);
func = f1.bind(null, 1);
print(func.length);
func = f1.bind(null, 1, 2);
print(func.length);
func = f1.bind(null, 1, 2, 3);
print(func.length);
