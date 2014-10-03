/*
 *  Compare slice() against substr() and substring().
 */

var str = new String('abcdefghij');

/*===
10
de 
de de
defgh fgh
===*/

try {
    print(str.length);

    // slice won't swap arguments like substring()
    print(str.slice(3, 5), str.slice(5, 3));
    print(str.substring(3, 5), str.substring(5, 3));

    // substring() takes a start and end offset as arguments, but
    // substr() takes a start offset and *length*
    print(str.substr(3, 5), str.substr(5, 3));

} catch (e) {
    print(e);
}
