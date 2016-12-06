/* Get object property count using Duktape.info(). */
function getObjectPropertyCount(obj) {
    var i = Duktape.info(obj);
    if (typeof i === 'object' && 6 in i) {
        return i[6];
    } else if (typeof i === 'object' && 'enext' in i) {
        return i.enext;
    }
    throw new Error('getObjectPropertyCount() relies on Duktape.info(), cannot parse result');
}
