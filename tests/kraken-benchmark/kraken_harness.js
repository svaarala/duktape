/*
 *  Harness / glue functions for running Kraken
 *
 *  Don't print() anything here because it's used by the 'sunspider' command
 *  to build loadable source files for result analysis.
 */

function load(filename) {
    alert('Load: ' + filename);
    if (typeof readFile !== 'function') {
        throw new Error('expecting "duk" to provide file i/o bindings');
    }
    try {
        var indirectEval = eval;  // use indirect eval to ensure 'var xxx = ...' gets registered to global variables
        var data = readFile(filename);
        return indirectEval('' + data);
    } catch (e) {
        alert(e.stack || e);  // may fail on purpose if file not found, just log
        return;
    }
}

function gc() {
    Duktape.gc();
}
