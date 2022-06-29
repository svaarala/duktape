'use strict';

// Init code to run which allows the testcase to run on multiple engines.
exports.ecmascriptTestFrameworkSource = `\
/* START TEST INIT */
(function initTestFramework() {
    'use strict';
    var Test = {};
    var G = new Function('return this')();

    // Engine detection.
    if (typeof G.Duktape === 'object' && G.Duktape !== null) {
        Test.isDuktape = true;
        Test.engine = 'duktape';
    } else if (typeof G.Packages === 'object' && G.Packages !== null && String(Packages) === '[JavaPackage ]') {
        Test.isRhino = true;
        Test.engine = 'rhino';
    } else if (typeof G.process === 'object' && G.process !== null && typeof G.process.version === 'string') {
        Test.isV8 = true;  // not exact, detects via Node.js
        Test.engine = 'v8';
    } else {
        Test.engine = 'unknown';
    }

    // Ensure print() binding exists.
    if (typeof G.print !== 'function') {
        if (G.process && G.process.stdout && typeof G.process.stdout.write === 'function') {
            G.print = function print() {
                process.stdout.write(Array.prototype.map.call(arguments, String).join(' ') + '\\n');
            };
        } else if (G.console && typeof G.console.log === 'function') {
            G.print = function print() {
                console.log(Array.prototype.map.call(arguments, String).join(' '));
            };
        }
    }

    // Ensure console and console.log() exist.
    if (typeof G.console === 'undefined') {
        G.console = {};
    }
    if (typeof G.console === 'object' && G.console !== null) {
        if (typeof G.console.log !== 'function') {
            if (typeof G.print === 'function') {
                G.console.log = G.print;
            }
        }
    }
})();
/* END TEST INIT */
`;
