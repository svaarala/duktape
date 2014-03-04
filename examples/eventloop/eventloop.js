/*
 *  Pure Ecmascript eventloop example.
 *
 *  Timer state handling is very inefficient in this trivial example.
 *  An actual eventloop would use a sorted timer list or a heap structure
 *  to manage timers and to get the "nearest timer" more efficiently.
 *
 *  https://developer.mozilla.org/en-US/docs/Web/JavaScript/Timers
 */

/*
 *  Timer manager
 */

function TimerManager() {
    this.active = [];
    this.nextTimerId = 1;
    this.minimumDelay = 1;
    this.minimumWait = 1;
};

TimerManager.prototype.dumpState = function() {
    print('TIMER STATE:');
    this.active.forEach(function(t) {
        print('    ' + Duktape.enc('jsonx', t));
    });
}

TimerManager.prototype.getEarliestTimer = function() {
    var i, n, t;
    var timers = this.active;
    var res;

    n = timers.length;
    if (n <= 0) {
        return null;
    }

    res = timers[0];
    for (i = 1; i < n; i++) {
        t = timers[i];
        if (t.target < res.target) {
            res = t;
        }
    }

    return res;
}

TimerManager.prototype.getEarliestWait = function() {
    var t = this.getEarliestTimer();
    if (!t) {
        return null;
    }
    var res = t.target - Date.now();
    return Math.max(this.minimumWait, res);
}

TimerManager.prototype.processTimers = function() {
    var i, n, t;
    var timers = this.active;
    var res;
    var now = Date.now();
    var work = [];

    /*
     *  Here we must be careful with mutations: one-shot timers are removed,
     *  and user callbacks can both insert and remove timers and intervals.
     *  We want to process those timers which are present before any callbacks
     *  have potentially mutated the timer list.  A very inefficient manual
     *  clone is used.
     */

    n = timers.length;
    for (i = 0; i < n; i++) {
        var t = timers[i];
        if (t.removed) {
            // if a callback uses clearTimeout() or clearInterval(), the
            // timer may have been cancelled and we don't want to call its
            // callback even if expired
            continue;
        }
        if (now > t.target) {
            work.push(timers[i]);
        }
    }

    n = work.length;
    for (i = 0; i < n; i++) {
        t = work[i];  // 'work' is safe from mutation
        if (t.oneshot) {
            t.remove = true;
        } else {
            t.target = now + t.delay;
        }
        try {
            t.cb();
        } catch (e) {
            print('timer callback failed, ignored: ' + e);
        }
    }

    /*
     *  Remove timers marked with remove=true, iterating in reverse order
     *  so that indexing works correctly even with removals.
     */

     n = timers.length;
     for (i = n - 1; i >= 0; i--) {
         t = timers[i];
         if (t.remove) {
             timers.splice(i, 1);
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

    mgr.active.push({
        id: timer_id,
        oneshot: true,
        cb: cb_func,
        delay: delay,
        target: Date.now() + delay
    });

    return timer_id;
}

function clearTimeout(timer_id) {
    var i, n, t;
    var mgr = _TIMERMANAGER;
    var timers = mgr.active;

    if (typeof timer_id !== 'number') {
        throw new TypeError('timer ID is not a number');
    }

    n = timers.length;
    for (i = 0; i < n; i++) {
        t = timers[i];
        if (t.id === timer_id) {
            t.removed = true;
            mgr.active.splice(i, 1);
            return;
        }
    }

    // no such ID, ignore
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

    mgr.active.push({
        id: timer_id,
        oneshot: false,
        cb: cb_func,
        delay: delay,
        target: Date.now() + delay
    });

    return timer_id;
}

function clearInterval(timer_id) {
    // same handling for now
    clearTimeout(timer_id);
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

        socket.poll({}, wait);
    }
}

EventLoop.prototype.exit = function() {
    this.exitRequested = true;
}

var _EVENTLOOP = new EventLoop();

