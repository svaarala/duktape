/*===
{"foo":1,"bar":2}
123 34 102 111 111 34 58 49 44 34 98 97 114 34 58 50 44 34 113 117 117 120 34 58 34 4660 34 125
===*/

var t;
var arr;
var i;

try {
    // simple ascii
    print(JSON.stringify({foo:1,bar:2}));

    // the Unicode character will not be escaped, so print output codepoints
    t = JSON.stringify({foo:1,bar:2,quux:"\u1234"});
    arr = [];
    for (i = 0; i < t.length; i++) {
        arr.push(t.charCodeAt(i));
    }
    print(arr.join(' '));
} catch (e) {
    print(e.name);
}
