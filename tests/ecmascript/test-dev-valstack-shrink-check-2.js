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

// Deep valstack inside a native call so valstack reserve is restored.
[1].forEach(function () {
    test(1000);
});
for (var i = 0; i < 10; i++) {
    Duktape.gc();
}
print('done');
