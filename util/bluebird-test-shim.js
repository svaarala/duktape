// Bluebird requires an async scheduler.  Provide a very fake setTimeout()
// to allow simple testing.

var fakeEventLoop;
var window;

(function () {
    var timers = [];

    window = Function('return this')();

    window.setTimeout = function (fn, timeout) {
        timers.push(fn);
    };

    window.clearTimeout = function () {};

    fakeEventLoop = function () {
        print('fake eventloop, run timers');
        while (timers.length > 0) {
            var fn = timers.shift();
            print('run timer');
            fn();
        }
        print('fake eventloop exiting, no more timers');
    };
})();
