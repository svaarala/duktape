'use strict';

// Extract Unicode categories as a map from category name (e.g. 'Zs') to
// list of codepoints.
function extractCategories(cpMap) {
    var categories = {};
    cpMap.forEach((ent, cp) => {
        void cp;
        if (ent) {
            categories[ent.gc] = categories[ent.gc] || [];
            categories[ent.gc].push(ent.cp);
        }
    });
    var res = {};  // sorted keys
    Object.getOwnPropertyNames(categories).sort().forEach((gc) => {
        res[gc] = categories[gc];
    });
    return res;
}
exports.extractCategories = extractCategories;
