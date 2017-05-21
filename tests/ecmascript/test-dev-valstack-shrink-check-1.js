/*===
done
===*/

function test(a,b,c,d,e,f,g,h,i,j) {
    if (a <= 0) {
        return;
    }
    test(a - 1);
    return 123;
}

// Pure Ecma-to-Ecma calls only
test(1000);
for (var i = 0; i < 10; i++) {
    Duktape.gc();
}
print('done');
