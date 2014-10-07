/*===
getTime="0" getUTCFullYear="1970" getUTCMonth="0" getUTCDate="1" getUTCDay="4" getUTCHours="0" getUTCMinutes="0" getUTCSeconds="0" getUTCMilliseconds="0"
getTime="-6373554843354994" getUTCFullYear="-200000" getUTCMonth="1" getUTCDate="2" getUTCDay="3" getUTCHours="3" getUTCMinutes="4" getUTCSeconds="5" getUTCMilliseconds="6"
getTime="6249225956645006" getUTCFullYear="200000" getUTCMonth="1" getUTCDate="2" getUTCDay="3" getUTCHours="3" getUTCMinutes="4" getUTCSeconds="5" getUTCMilliseconds="6"
getTime="NaN" getUTCFullYear="NaN" getUTCMonth="NaN" getUTCDate="NaN" getUTCDay="NaN" getUTCHours="NaN" getUTCMinutes="NaN" getUTCSeconds="NaN" getUTCMilliseconds="NaN"
===*/

function printGetters(dt) {
    var tmp = [];

    function f(n,v) {
        if (typeof v !== 'number') {
            throw new Error('invalid type: ' + v);
        }
        tmp.push(n + '="' + String(v) + '"');
    }

    // XXX: how to test non-UTC getters reasonably?
    // test equality that local components + time offset == utc
    // components, and check component range?

    f('getTime', dt.getTime());
    //f('getFullYear', dt.getFullYear());
    f('getUTCFullYear', dt.getUTCFullYear());
    //f('getMonth', dt.getMonth());
    f('getUTCMonth', dt.getUTCMonth());
    //f('getDate', dt.getDate());
    f('getUTCDate', dt.getUTCDate());
    //f('getDay', dt.getDay());
    f('getUTCDay', dt.getUTCDay());
    //f('getHours', dt.getHours());
    f('getUTCHours', dt.getUTCHours());
    //f('getMinutes', dt.getMinutes());
    f('getUTCMinutes', dt.getUTCMinutes());
    //f('getSeconds', dt.getSeconds());
    f('getUTCSeconds', dt.getUTCSeconds());
    //f('getMilliseconds', dt.getMilliseconds());
    f('getUTCMilliseconds', dt.getUTCMilliseconds());

    print(tmp.join(' '));
}

function datePrototypeGetterTests() {
    var pg = printGetters;

    // XXX: more tests?

    pg(new Date(0));
    pg(new Date(Date.UTC(-200000, 1, 2, 3, 4, 5, 6)));
    pg(new Date(Date.UTC(200000, 1, 2, 3, 4, 5, 6)));

    // NaN
    pg(new Date(Date.UTC(-300000, 1, 2, 3, 4, 5, 6)));
}

try {
    datePrototypeGetterTests();
} catch (e) {
    print(e.name);
}
