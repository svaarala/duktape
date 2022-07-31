function test() {
    var obj = {};
    var i;

    for (i = 0; i < 10; i++) {
        obj['prop-' + i] = 1;
    }

    for (i = 0; i < 1e5; i++) {
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
        void Object.assign({}, obj);
    }
}

test();
