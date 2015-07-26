/*
 *  Simple label parsing test without actual label semantics tests.
 */

/*===
foo
bar
quux
===*/

label1:
print("foo");

label1:
label2:
print("bar");

label1:
label2:
label3:
do {
    print("quux");
} while(0);
