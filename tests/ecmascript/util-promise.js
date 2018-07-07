function setupPromiseUnhandledCallbacks(unhandledRejection, rejectionHandled) {
    if (typeof Duktape === 'object' && typeof Promise === 'function' && Promise.isPolyfill) {
        Promise.unhandledRejection = function (args) {
            //console.log('unhandledCallback:', args);
            if (args.event === 'reject') {
                unhandledRejection(args.promise);
            } else if (args.event === 'handle') {
                rejectionHandled(args.promise);
            }
        };
    } else if (typeof process === 'object') {
        // https://nodejs.org/api/process.html#process_event_unhandledrejection
        // https://nodejs.org/api/process.html#process_event_rejectionhandled
        process.on('unhandledRejection', function (reason, promise) {
            unhandledRejection(promise);
        });
        process.on('rejectionHandled', function (promise) {
            rejectionHandled(promise);
        });
    } else {
        throw new TypeError('failed to setup Promise unhandled rejection callbacks');
    }
}

function promiseNextTick(cb) {
    if (typeof Duktape === 'object' && typeof Promise === 'function' && Promise.isPolyfill) {
        Promise.runQueue();
        cb();
    } else if (typeof setTimeout === 'function') {
        setTimeout(cb, 0);
    } else {
        throw new TypeError('failed to schedule callback for next Promise tick');
    }
}
