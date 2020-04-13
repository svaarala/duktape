if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = {};
    var i, count;

    for (i = 0; i < 2; i++) {
        obj['prop' + i] = 1;
    }
    if (typeof Duktape !== 'undefined') { Duktape.compact(obj); }

    for (count = 1e8; count > 0;) {
        void obj.prop0; void obj.prop1; void obj.prop0; void obj.prop1; void obj.prop0;
        void obj.prop1; void obj.prop0; void obj.prop1; void obj.prop0; void obj.prop1;
        void obj.prop0; void obj.prop1; void obj.prop0; void obj.prop1; void obj.prop0;
        void obj.prop1; void obj.prop0; void obj.prop1; void obj.prop0; void obj.prop1;
        count -= 20;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
