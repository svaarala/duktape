/*
 *  ES2015 computed property name.
 */

/*===
{"3":"three","5":"five"}
three
five
toString 1
toString 2
{"tostring1":"value1","tostring2":"value2"}
undefined
value1
undefined
value2
{"[object Object]":"object","1,2,3,4":"array"}
URIError
===*/

function computedPropertyName() {
    var res;

    // Simple example.
    try {
        res = eval('({ [1+2]: "three", [2+3]: "five" })');
        print(JSON.stringify(res));
        print(res[3]);
        print(res['5']);
    } catch (e) {
        print(e.stack || e);
    }

    // Expression values are ToPropertyKey() coerced in order.  Before ES2015
    // Symbols that means ToString() in practice.
    try {
        res = eval('({ [ { valueOf: function () { print("valueOf 1"); return "valueof1"; }, toString: function () { print("toString 1"); return "tostring1"; } } ]: "value1", [ { valueOf: function () { print("valueOf 2"); return "valueof2"; }, toString: function () { print("toString 2"); return "tostring2"; } } ]: "value2" })');
        print(JSON.stringify(res));
        print(res.valueof1);
        print(res.tostring1);
        print(res.valueof2);
        print(res.tostring2);
    } catch (e) {
        print(e.stack || e);
    }

    // If an object is used as the computed key, it coerces to the string
    // '[object Object]'.  An Array coerces to a comma joined list.
    try {
        res = eval('({ [{}]: "object", [[1,2,3,4]]: "array" })');
        print(JSON.stringify(res));
    } catch (e) {
        print(e.stack || e);
    }

    // An error may be thrown.
    try {
        res = eval('({ [{}]: "object", [decodeURIComponent("%XX")]: "other" })');
        print(JSON.stringify(res));
    } catch (e) {
        print(e.name);
    }
}

try {
    computedPropertyName();
} catch (e) {
    print(e.stack || e);
}
