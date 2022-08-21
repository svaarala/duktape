/*
 *  Long chain of references going to refcount zero.
 *
 *  In a basic refcount implementation this causes recursive C invocations
 *  of the handler for zero reference count.
 */

/*===
0
1000
2000
3000
4000
5000
6000
7000
8000
9000
10000
11000
12000
13000
14000
15000
16000
17000
18000
19000
20000
21000
22000
23000
24000
still here
===*/

var x;
var t;
var i;

for (i = 0; i < 25000; i++) {
    if ((i % 1000) == 0) {
        print(i);
    }
    t = {};
    t.ref = x;
    x = t;
}

x = null;  /* refcount to zero in the whole chain */

/* XXX: some gc stats thing here? */
print("still here");
