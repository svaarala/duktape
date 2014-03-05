/*
 *  Test using timers and intervals with curses.
 */

if (typeof ncurses !== 'object') {
    throw new Error('ncurses required');
}

function fillScreen(ch) {
    var size, w, h;
    var i, j;

    size = ncurses.getmaxyx();
    h = size[0];
    w = size[1];

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            ncurses.mvprintw(i, j, ch);
        }
    }
    ncurses.refresh();
}

function main() {
    var i, j;
    var counters = [];
    var size, w, h;

    ncurses.initscr();
    size = ncurses.getmaxyx();
    h = size[0];
    w = size[1];

    fillScreen('.');

    setInterval(function () {
        ncurses.mvprintw(1, 4, new Date().toISOString() + '  ' + _TIMERMANAGER.timers.length +
                               ' timers/intervals' + '    ');
        ncurses.refresh();
    }, 1000);

    function addCounter(row, index, interval) {
        counters[index] = 0;
        setInterval(function () {
            counters[index]++;
            ncurses.mvprintw(row, 4, '' + Date.now() + ' ' + counters[index]);
            ncurses.refresh();
        }, interval);
    }

    function addRandomChar(row, col, interval) {
        setTimeout(function () {
            ncurses.mvprintw(row, col, String.fromCharCode(Math.random() * 64 + 0x20));
            ncurses.refresh();
        }, interval);
    }

    for (i = 0; i < h - 5; i++) {
        addCounter(3 + i, i, 363 * i + 400);
    }

    /* Here the inserts take a lot of time because the underlying timer manager
     * data structure has O(n) insertion performance.
     */
    for (i = 0; i < h - 5; i++) {
        for (j = 0; j < w - 50; j++) {
            // Math.exp(0)...Math.exp(8) is an uneven distribution between 1...~2980.
            addRandomChar(3 + i, 28 + j, 58000 - Math.exp(Math.random() * 8) * 20);
        }
    }

/*
    ncurses.getch();
    ncurses.endwin();
    ncurses.delscreen();
    print('deleted');
*/
}

main();
