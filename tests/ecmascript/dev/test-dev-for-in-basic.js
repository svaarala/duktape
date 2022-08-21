/*===
foo -> 1
bar -> 2
===*/

obj = {foo:1,bar:2};

for (i in obj) {
    print(i,'->',obj[i]);
}
