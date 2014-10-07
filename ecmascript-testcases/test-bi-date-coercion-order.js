function dateToComponentString(dt) {
    return [
               dt.getUTCFullYear(),
               dt.getUTCMonth(),
               dt.getUTCDate(),
               dt.getUTCHours(),
               dt.getUTCMinutes(),
               dt.getUTCSeconds(),
               dt.getUTCMilliseconds()
           ].join(' ');
}

function utcDiffAtTime(dt) {
    var d1;

    // This is not exactly accurate but good enough for testing:
    // take the UTC components and use them as local ones and see
    // how much difference that makes.
    d1 = new Date(dt.getUTCFullYear(),
                  dt.getUTCMonth(),
                  dt.getUTCDate(),
                  dt.getUTCHours(),
                  dt.getUTCMinutes(),
                  dt.getUTCSeconds(),
                  dt.getUTCMilliseconds());

    // Positive offset = local time is ahead of UTC (e.g. EET is
    // +7200e3 or +10800e3
    return dt.getTime() - d1.getTime();
}

/*===
constructor as function
no side effects
constructor components
valueOf 1
valueOf 2
valueOf 3
valueOf 4
valueOf 5
valueOf 6
valueOf 7
2013 1 2 3 4 5 6
constructor time value
valueOf 8
12345 1 2 3 4 5 6
component setters
valueOf 1
valueOf 1
valueOf 1
valueOf 2
valueOf 1
valueOf 2
valueOf 1
valueOf 2
valueOf 3
valueOf 1
valueOf 2
valueOf 3
valueOf 1
valueOf 2
valueOf 3
valueOf 4
valueOf 1
valueOf 2
valueOf 3
valueOf 4
valueOf 1
valueOf 1
valueOf 1
valueOf 2
valueOf 1
valueOf 2
valueOf 1
valueOf 2
valueOf 3
valueOf 1
valueOf 2
valueOf 3
valueOf 1
===*/

/* Side effects and evaluation order of coercions */

function coercionSideEffectsTest() {
    var d;
    var obj1 = { toString: function() { print('toString 1'); return 'toString 1'; },
                 valueOf: function() { print('valueOf 1'); return 2013; } };
    var obj2 = { toString: function() { print('toString 2'); return 'toString 2'; },
                 valueOf: function() { print('valueOf 2'); return 1; } };
    var obj3 = { toString: function() { print('toString 3'); return 'toString 3'; },
                 valueOf: function() { print('valueOf 3'); return 2; } };
    var obj4 = { toString: function() { print('toString 4'); return 'toString 4'; },
                 valueOf: function() { print('valueOf 4'); return 3; } };
    var obj5 = { toString: function() { print('toString 5'); return 'toString 5'; },
                 valueOf: function() { print('valueOf 5'); return 4; } };
    var obj6 = { toString: function() { print('toString 6'); return 'toString 6'; },
                 valueOf: function() { print('valueOf 6'); return 5; } };
    var obj7 = { toString: function() { print('toString 7'); return 'toString 7'; },
                 valueOf: function() { print('valueOf 7'); return 6; } };
    var obj8 = { toString: function() { print('toString 8'); return 'toString 8'; },
                 valueOf: function() { print('valueOf 8'); return 327406158245006; } };
                 // Date.UTC(12345,1,2,3,4,5,6) -> above return value

    function toUtcAndPrint(dt) {
        var utcDiff = utcDiffAtTime(dt);
        dt = new Date(dt.getTime() + utcDiff);  // Now UTC time
        print(dateToComponentString(dt));
    }

    // Date constructor, called as a function (ignore all arguments)
    print('constructor as function');
    d = Date(obj1, obj2, obj3, obj4, obj5, obj6, obj7);
    print('no side effects');

    // Date constructor, component call
    print('constructor components');
    d = new Date(obj1, obj2, obj3, obj4, obj5, obj6, obj7);
    toUtcAndPrint(d);

    // Date constructor, time value call
    print('constructor time value');
    d = new Date(obj8);
    print(dateToComponentString(d));

    // year setters will convert internal NaN timevalue to +0, but
    // otherwise proceed normally
    print('component setters');
    d = new Date(NaN);
    d.setUTCMilliseconds(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setMilliseconds(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setUTCSeconds(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setSeconds(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setUTCMinutes(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setMinutes(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setUTCHours(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setHours(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setUTCDate(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setDate(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setUTCMonth(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setMonth(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setUTCFullYear(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setFullYear(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
    d.setYear(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);

    // test-bi-date-tojson-generic covers toJSON() coercion order
}

try {
    coercionSideEffectsTest();
} catch (e) {
    print(e.name);
}

/*===
this check
valueOf
TypeError
===*/

function coercionThisValueCheck() {
    // The 'this' check should happen before anything else is coerced.
    // This is not tested exhaustively now.
    print('this check')
    d = new Date(NaN);
    d.setUTCSeconds.call(d,
                         { valueOf: function() { print('valueOf'); return 123; } });
    d.setUTCSeconds.call('foo',
                         { valueOf: function() { print('valueOf'); return 123; } });
}

try {
    coercionThisValueCheck();
} catch (e) {
    print(e.name);
}
