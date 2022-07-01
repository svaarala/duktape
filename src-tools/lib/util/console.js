'use strict';

function disableConsoleDebug() {
    console.debug = function nop() {};
    console.trace = function nop() {};
}
exports.disableConsoleDebug = disableConsoleDebug;
