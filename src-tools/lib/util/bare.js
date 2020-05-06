'use strict';

function createBareObject(props) {
    var o = Object.create(null);
    if (props) {
        Object.assign(o, props);
    }
    return o;
}
exports.createBareObject = createBareObject;

function makeBare(val, doArrays) {
    var count = 0;

    function process(x) {
        if (!(typeof x === 'object' && x !== null)) {
            return x;
        }
        if (Array.isArray(x) === doArrays) {
            if (Object.getPrototypeOf(x) !== null) {
                Object.setPrototypeOf(x, null);
                count++;
            }
        }
        Object.getOwnPropertyNames(x).forEach((k) => {
            x[k] = process(x[k]);
        });
        return x;
    }

    var res = process(val);
    //console.debug('made ' + count + ' objects bare');
    void count;

    return res;
}

function makeObjectsBareRecursive(x) {
    return makeBare(x, false);
}
exports.makeObjectsBareRecursive = makeObjectsBareRecursive;

function makeArraysBareRecursive(x) {
    return makeBare(x, true);
}
exports.makeArraysBareRecursive = makeArraysBareRecursive;
