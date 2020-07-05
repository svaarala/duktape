// When we revive array index 0 (and 1), array index 2 is always replaced
// with a new array.  When we walk that array, the same happens, without
// limit.

/*===
RangeError
done
===*/

function revive(key, value) {
    this[2] = [ 0, 0, 0 ];
    return value;
}

try {
    print(JSON.parse('[1, 2, 3]', revive));
} catch (e) {
    print(e.name);
}
print('done');
