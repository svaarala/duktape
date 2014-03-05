/*
 *  Pure Ecmascript eventloop example.
 *
 *  Timer state handling is inefficient in this trivial example.  Timers are
 *  kept in an array sorted by their expiry time which works well for expiring
 *  timers, but has O(n) insertion performance.  A better implementation would
 *  use a heap or some other efficient structure for managing timers so that
 *  all operations (insert, remove, get nearest timer) have good performance.
 *
 *  https://developer.mozilla.org/en-US/docs/Web/JavaScript/Timers
 */

/*
 *  Timer manager
 *
 *  Timers are sorted by 'target' property which indicates expiry time of
 *  the timer.  The timer expiring next is last in the array, so that
 *  removals happen at the end, and inserts for timers expiring in the
 *  near future displace as few elements in the array as possible.
 */

function TimerManager() {
    this.timers = [];   // active timers, sorted (nearest expiry last)
    this.work = [];     // work list when processing expired timers
    this.nextTimerId = 1;
    this.minimumDelay = 1;
    this.minimumWait = 1;
};

TimerManager.prototype.dumpState = function() {
    print('TIMER STATE:');
    this.timers.forEach(function(t) {
        print('    ' + Duktape.enc('jsonx', t));
    });
}

// Get timer with lowest expiry time.  Since the active timers list is
// sorted, it's always the last timer.
TimerManager.prototype.getEarliestTimer = function() {
    var timers = this.timers;
    n = timers.length;
    return (n > 0 ? timers[n - 1] : null);
}

TimerManager.prototype.getEarliestWait = function() {
    var t = this.getEarliestTimer();
    return (t ? Math.max(this.minimumWait, t.target - Date.now()) : null);
}

TimerManager.prototype.insertTimer = function(timer) {
    var timers = this.timers;
    var i, n, t;

    /*
     *  Find 'i' such that we want to insert *after* timers[i] at index i+1.
     *  If no such timer, for-loop terminates with i-1, and we insert at -1+1=0.
     */

    n = timers.length;
    for (i = n - 1; i >= 0; i--) {
        t = timers[i];
        if (timer.target <= t.target) {
            // insert after 't', to index i+1
            break;
        }
    }

    timers.splice(i + 1 /*start*/, 0 /*deleteCount*/, timer);
}

// Remove timer/interval with a timer ID.  The timer/interval can reside
// either on the active list, or if we're processing expired timers, on
// the work list.
TimerManager.prototype.removeTimerById = function(timer_id) {
    var timers = this.timers;
    var work = this.work;
    var i, n, t;

    n = timers.length;
    for (i = 0; i < n; i++) {
        t = timers[i];
        if (t.id === timer_id) {
            // Timer on active list: mark removed (not really necessary, but
            // nice for dumping), and remove from active list.
            t.removed = true;
            this.timers.splice(i /*start*/, 1 /*deleteCount*/);
            return;
        }
    }

    if (!work) {
        // no such ID, ignore
        return;
    }

    n = work.length;
    for (i = 0; i < n; i++) {
        t = work[i];
        if (t.id === timer_id) {
            // Timer on work list: timer is being deleted from user callback
            // while expiring timers are being processed.  Mark removed, so
            // that the timer is not reinserted back into the active list,
            // but don't remove from work list.
            t.removed = true;
            return;
        }
    }

    // no such ID, ignore
}

TimerManager.prototype.processTimers = function() {
    var now = Date.now();
    var timers = this.timers;
    var work;
    var i, n, t;

    /*
     *  Here we must be careful with mutations: one-shot timers are removed,
     *  and user callbacks can both insert and remove timers and intervals.
     *  We want to process (only) those expired timers which are present
     *  before any callbacks have potentially mutated the timer list.
     *
     *  Current simple approach is to move expired timers to a work list
     *  before calling any callback functions.  Since the active timer list
     *  is sorted, active timer scan can bail out early once a non-expired
     *  timer has been found.
     */

    // Find 'i' such that timers[i] has not yet expired, worklist will then
    // be timers[i+1] onwards.

    n = timers.length;
    for (i = n - 1; i >= 0; i--) {
        var t = timers[i];
        if (now <= t.target) {
            // Timer has not expired, last timer to keep is timers[i].
            break;
        }
    }
    work = timers.splice(i + 1, timers.length - i);
    this.work = work;  // make accessible to callbacks (e.g. clearTimeout)

    // Process expired timers, callback for nearest expiry is called first
    // (not mandatory but nice).  The callbacks may add new timers (not a
    // problem), or cancel timers either on the active list or the work list.
    // When a timer on the work list is cancelled, it is marked 'removed'
    // but is not removed from the worklist, as we handle that below.

    n = work.length;
    for (i = n - 1; i >= 0; i--) {
        t = work[i];
        if (t.oneshot) {
            t.removed = true;  // flag for removal
        } else {
            t.target = now + t.delay;
        }
        try {
            t.cb();
        } catch (e) {
            print('timer callback failed, ignored: ' + e);
        }
    }
    this.work = null;  // no longer needs to be reachable

    // Drop one-shot timers, reinsert interval timers to active list so that
    // they get sorted to correct position.  If timer/interval has been removed
    // (= marked removed) by user callback or above (for oneshot timers), just
    // ignore them here and the timers are removed.

    n = work.length;
    for (i = n - 1; i >= 0; i--) {
        t = work[i];
        if (t.removed) {
            // Also covers oneshot timers which are marked removed above.
            continue;
        } else {
            // Reinsert interval timer to correct sorted position.
            this.insertTimer(t);
        }
    }
}

var _TIMERMANAGER = new TimerManager();  // singleton instance

/*
 *  Timer API
 *
 *  These interface with the singleton TimerManager.
 */

function setTimeout(func, delay) {
    var cb_func;
    var bind_args;
    var timer_id;
    var mgr = _TIMERMANAGER;

    if (typeof delay !== 'number') {
        throw new TypeError('delay is not a number');
    }
    delay = Math.max(mgr.minimumDelay, delay);

    if (typeof func !== 'function') {
        // Legacy case: callback is a string.
        cb_func = eval.bind(this, func);
    } else if (arguments.length > 2) {
        // Special case: callback arguments are provided.
        bind_args = arguments.slice(2);  // [ arg1, arg2, ... ]
        bind_args = bind_args.unshift(this);  // [ global(this), arg1, arg2, ... ]
        cb_func = func.bind.apply(func, bind_args);
    } else {
        // Normal case: callback given as a function without arguments.
        cb_func = func;
    }

    timer_id = mgr.nextTimerId++;

    mgr.insertTimer({
        id: timer_id,
        oneshot: true,
        cb: cb_func,
        delay: delay,
        target: Date.now() + delay
    });

    return timer_id;
}

function clearTimeout(timer_id) {
    var mgr = _TIMERMANAGER;

    if (typeof timer_id !== 'number') {
        throw new TypeError('timer ID is not a number');
    }
    mgr.removeTimerById(timer_id);
}

function setInterval(func, delay) {
    var cb_func;
    var bind_args;
    var timer_id;
    var mgr = _TIMERMANAGER;

    if (typeof delay !== 'number') {
        throw new TypeError('delay is not a number');
    }
    delay = Math.max(mgr.minimumDelay, delay);

    if (typeof func !== 'function') {
        // Legacy case: callback is a string.
        cb_func = eval.bind(this, func);
    } else if (arguments.length > 2) {
        // Special case: callback arguments are provided.
        bind_args = arguments.slice(2);  // [ arg1, arg2, ... ]
        bind_args = bind_args.unshift(this);  // [ global(this), arg1, arg2, ... ]
        cb_func = func.bind.apply(func, bind_args);
    } else {
        // Normal case: callback given as a function without arguments.
        cb_func = func;
    }

    timer_id = mgr.nextTimerId++;

    mgr.insertTimer({
        id: timer_id,
        oneshot: false,
        cb: cb_func,
        delay: delay,
        target: Date.now() + delay
    });

    return timer_id;
}

function clearInterval(timer_id) {
    var mgr = _TIMERMANAGER;

    if (typeof timer_id !== 'number') {
        throw new TypeError('timer ID is not a number');
    }
    mgr.removeTimerById(timer_id);
}

/*
 *  Event loop
 */

function EventLoop() {
}

EventLoop.prototype.run = function() {
    var wait;

    for (;;) {
        _TIMERMANAGER.processTimers();
        //_TIMERMANAGER.dumpState();

        if (this.exitRequested) {
            //print('exit requested, exit');
            break;
        }
        wait = _TIMERMANAGER.getEarliestWait();
        if (!wait) {
            //print('no active timers, exit');
            break;
        }

        /* FIXME: sockets */
        try {
            socket.poll({}, wait);
        } catch (e) {
            // Eat errors silently.  When resizing curses window an EINTR
            // happens now.
        }
    }
}

EventLoop.prototype.exit = function() {
    this.exitRequested = true;
}

var _EVENTLOOP = new EventLoop();

