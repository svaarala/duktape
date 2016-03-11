/*
 *  Db utilities.
 */

var Promise = require('bluebird');

// Delete all documents.
function deleteAll(db) {
    return new Promise(function (resolve, reject) {
        db.remove({
        }, {
            multi: true
        }, function (err, numRemoved) {
            console.log('removed', numRemoved, 'documents');
            if (err) {
                reject(err);
            } else {
                resolve(numRemoved);
            }
        });
    });
}

// Find document(s).
function find(db, arg) {
    return new Promise(function (resolve, reject) {
        db.find(arg, function (err, docs) {
            if (err) {
                reject(err);
            } else {
                resolve(docs);
            }
        });
    });
}

exports.deleteAll = deleteAll;
exports.find = find;
