'use strict';

function estimateCoreCount() {
    try {
        let os = require('os');
        if (typeof os?.cpus === 'function') {
            return os.cpus().length;
        } else {
            return;
        }
    } catch (e) {
        void e;
        return;
    }
}

exports.estimateCoreCount = estimateCoreCount;
