*
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
    this.timers = [];         // active timers, sorted (nearest expiry last)
    this.expiring = null;     // set to timer being expired (needs special handling in clearTimeout/clearInterval)
    this.nextTimerId = 1;
    this.minimumDelay = 1;
    this.minimumWait = 1;
    this.maxExpirys = 10;
};

TimerManager.prototype.dumpState = function() {
    print('TIMER STATE:');
    this.timers.forEach(function(t) {
        print('    ' + Duktape.enc('jsonx', t));
    });
    if (this.expiring) {
        print('    EXPIRING: ' + Duktape.enc('jsonx', this.expiring));
    }
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
// either on the active list or it may be an expired timer (this.expiring)
// whose user callback we're running when this function gets called.
TimerManager.prototype.removeTimerById = function(timer_id) {
    var timers = this.timers;
    var i, n, t;

    t = this.expiring;
    if (t) {
        if (t.id === timer_id) {
            // Timer has expired and we're processing its callback.  User
            // callback has requested timer deletion.  Mark removed, so
            // that the timer is not reinserted back into the active list.
            // This is actually a common case because an interval may very
            // well cancel itself.
            t.removed = true;
            return;
        }
    }

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

   // no such ID, ignore
}

TimerManager.prototype.processTimers = function() {
    var now = Date.now();
    var timers = this.timers;
    var sanity = this.maxExpirys;
    var n, t;

    /*
     *  Here we must be careful with mutations: user callback may add and
     *  delete an arbitrary number of timers.
     *
     *  Current solution is simple: check whether the timer at the end of
     *  the list has expired.  If not, we're done.  If it has expired,
     *  remove it from the active list, record it in this.expiring, and call
     *  the user callback.  If user code deletes the this.expiring timer,
     *  there is special handling which just marks the timer deleted so
     *  it won't get inserted back into the active list.
     *
     *  This process is repeated at most maxExpirys times to ensure we don't
     *  get stuck forever; user code could in principle add more and more
     *  already expired timers.
     */

    while (sanity-- > 0) {
        n = timers.length;
        if (n <= 0) {
            break;
        }
        t = timers[n - 1];
        if (now <= t.target) {
            // Timer has not expired, and no other timer could have expired
            // either because the list is sorted.
            break;
        }
        timers.pop();

        // Remove the timer from the active list and process it.  The user
        // callback may add new timers which is not a problem.  The callback
        // may also delete timers which is not a problem unless the timer
        // being deleted is the timer whose callback we're running; this is
        // why the timer is recorded in this.expiring so that clearTimeout()
        // and clearInterval() can detect this situation.

        if (t.oneshot) {
            t.removed = true;  // flag for removal
        } else {
            t.target = now + t.delay;
        }
        this.expiring = t;
        try {
            t.cb();
        } catch (e) {
            print('timer callback failed, ignored: ' + e);
        }
        this.expiring = null;

        // If the timer was one-shot, it's marked 'removed'.  If the user callback
        // requested deletion for the timer, it's also marked 'removed'.  If the
        // timer is an interval (and is not marked removed), insert it back into
        // the timer list.

        if (!t.removed) {
            // Reinsert interval timer to correct sorted position.  The timer
            // must be an interval timer because one-shot timers are marked
            // 'removed' above.
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

    if (typeof func === 'string') {
        // Legacy case: callback is a string.
        cb_func = eval.bind(this, func);
    } else if (typeof func !== 'function') {
        throw new TypeError('callback is not a function/string');
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

    if (typeof func === 'string') {
        // Legacy case: callback is a string.
        cb_func = eval.bind(this, func);
    } else if (typeof func !== 'function') {
        throw new TypeError('callback is not a function/string');
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
            Poll.poll({}, wait);
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
