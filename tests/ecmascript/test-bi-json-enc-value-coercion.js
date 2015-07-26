/*
 *  Test the value coercion steps of E5.1 Section, Str() algorithm:
 *
 *    - toJSON (step 2)
 *    - replacer (step 3)
 *    - coercion of Number, String, Boolean objects to primitive values (step 4)
 *    - final Number coercion (step 9)
 *    - special handling for callable objects (step 10)
 */

/*===
replacer called: this=[object Object], key=, val=[object Object]
toJSON called: this=[object Object], key=number_object
replacer called: this=[object Object], key=number_object, val=tojson-retval-1
Number.prototype.valueOf() called, typeof this=object
toJSON called: this=[object Object], key=string_object
replacer called: this=[object Object], key=string_object, val=tojson-retval-2
String.prototype.toString() called, typeof this=object
toJSON called: this=[object Object], key=boolean_object
replacer called: this=[object Object], key=boolean_object, val=tojson-retval-3
{"number_object":321,"string_object":"mystr-tostring","boolean_object":true}
===*/

/* Basic test which covers all steps. */

function valueCoercionTest1() {
    var old_Number_toString;
    var old_Number_valueOf;
    var old_String_toString;
    var old_String_valueOf;
    var old_Boolean_toString;
    var old_Boolean_valueOf;

    var obj = {
        number_object: {
            toJSON: function(k) {
                print('toJSON called: this=' + this + ', key=' + k);
                return 'tojson-retval-1';
            }
        },
        string_object: {
            toJSON: function(k) {
                print('toJSON called: this=' + this + ', key=' + k);
                return 'tojson-retval-2';
            }
        },
        boolean_object: {
            toJSON: function(k) {
                print('toJSON called: this=' + this + ', key=' + k);
                return 'tojson-retval-3';
            }
        }
    };

    function replacer(k,v) {
        print('replacer called: this=' + this + ', key=' + k + ', val=' + v);
        if (v === 'tojson-retval-1') {
            return new Number(123);
        } else if (v === 'tojson-retval-2') {
            return new String('foo');
        } else if (v === 'tojson-retval-3') {
            return new Boolean(true);
        } else {
            return v;
        }
    }

    old_Number_toString = Number.prototype.toString;
    old_Number_valueOf = Number.prototype.valueOf;
    old_String_toString = String.prototype.toString;
    old_String_valueOf = String.prototype.valueOf;
    old_Boolean_toString = Boolean.prototype.toString;
    old_Boolean_valueOf = Boolean.prototype.valueOf;

    // Careful to avoid recursion in print() calls

    Number.prototype.toString = function () {
        print('Number.prototype.toString() called, typeof this=' + typeof this);
        return '432';
    };
    Number.prototype.valueOf = function () {
        print('Number.prototype.valueOf() called, typeof this=' + typeof this);
        return 321;
    };
    String.prototype.toString = function () {
        print('String.prototype.toString() called, typeof this=' + typeof this);
        return 'mystr-tostring';
    };
    String.prototype.valueOf = function () {
        print('String.prototype.valueOf() called, typeof this=' + typeof this);
        return 'mystr-valueof';
    };
    Boolean.prototype.toString = function () {
        print('Boolean.prototype.toString() called, typeof this=' + typeof this);
        return "'tis true";
    };
    Boolean.prototype.valueOf = function () {
        print('Boolean.prototype.valueOf() called, typeof this=' + typeof this);
        return false;
    };

    /*
     * Number object:
     *
     * toJSON() -> 'tojson-retval-2'
     * replacer -> new Number(123)
     * ToNumber() -> Number.prototype.valueOf() -> 321
     * ToString() -> primitive number conversion -> '321'  (Number.ptototype.toString() not called)
     *
     * String object:
     *
     * toJSON() -> 'tojson-retval-2'
     * replacer -> new String('foo')
     * ToString() -> 'mystr-tostring'
     *
     * Boolean object:
     *
     * toJSON() -> 'tojson-retval-3'
     * replacer -> new Boolean(true)
     * Str() accesses Boolean [[PrimitiveValue]] directly, no valueOf()/toString() calls
     */

    try {
        print(JSON.stringify(obj, replacer));
    } finally {
        Number.prototype.toString = old_Number_toString;
        Number.prototype.valueOf = old_Number_valueOf;
        String.prototype.toString = old_String_toString;
        String.prototype.valueOf = old_String_valueOf;
        Boolean.prototype.toString = old_Boolean_toString;
        Boolean.prototype.valueOf = old_Boolean_valueOf;
    }
}

try {
    valueCoercionTest1();
} catch (e) {
    print(e.name, e, e.stack || e);
}

/*===
===*/

/* XXX: replacer/toJSON returns an object with toJSON() */
