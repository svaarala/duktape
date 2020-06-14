// https://github.com/svaarala/duktape/issues/2315

/*===
done
===*/

function main() {
    var v1 = [13.37,13.37,13.37];
    var v2 = [13.37];
    var v3 = {a:v2,length:v2};
    var v4 = v3;
    v4.__proto__ = v1;
    var v5 = v4.sort();
    for (var v6 in v5) {
    }
}
main();
print('done');
