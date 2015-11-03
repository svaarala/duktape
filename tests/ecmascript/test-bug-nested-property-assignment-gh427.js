/*
 *  https://github.com/svaarala/duktape/issues/427
 */

/*===
none true 0
text true 1
0 true none
1 true text
none true 0
text true 1
0 true none
1 true text
===*/

// Original issue
var obj1;
(function (obj1) {
    obj1[obj1["none"] = 0] = "none";
    obj1[obj1["text"] = 1] = "text";
})(obj1 || (obj1 = {}));

[ 'none', 'text', 0, 1 ].forEach(function (k) {
   print(k, k in obj1, obj1[k]);
});

// Slightly modified
var obj2;
(function (obj2) {
    var t = '';
    var i = 0;
    obj2[obj2[t + "none"] = i + 0] = "none";
    obj2[obj2[t + "text"] = i + 1] = "text";
})(obj2 || (obj2 = {}));

[ 'none', 'text', 0, 1 ].forEach(function (k) {
   print(k, k in obj2, obj2[k]);
});
