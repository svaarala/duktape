if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = {};
    var i, count;

    for (i = 0; i < 3; i++) {
        obj['prop' + i] = 1;
    }
    if (typeof Duktape !== 'undefined') { Duktape.compact(obj); }

    for (count = 1e8; count > 0;) {
        void obj.prop0; void obj.prop1; void obj.prop2;
        void obj.prop0; void obj.prop1; void obj.prop2;
        void obj.prop0; void obj.prop1; void obj.prop2;
        void obj.prop0; void obj.prop1; void obj.prop2;
        void obj.prop0; void obj.prop1; void obj.prop2;
        void obj.prop0; void obj.prop1; void obj.prop2;
        void obj.prop0; void obj.prop1; void obj.prop2;
        count -= 21;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
