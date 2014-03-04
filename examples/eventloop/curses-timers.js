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

    ncurses.initscr();
    fillScreen('.');

    setInterval(function () {
        ncurses.mvprintw(1, 4, new Date().toISOString() + '  ' + _TIMERMANAGER.active.length +
                               ' timers/intervals' + '    ');
        ncurses.refresh();
    }, 1000);

    function addCounter(row, interval) {
        setInterval(function () {
            ncurses.mvprintw(row, 4, '' + Date.now());
            ncurses.refresh();
        }, interval);
    }

    function addRandomChar(row, col, interval) {
        setTimeout(function () {
            ncurses.mvprintw(row, col, String.fromCharCode(Math.random() * 64 + 0x20));
            ncurses.refresh();
        }, interval);
    }

    for (i = 0; i < 16; i++) {
        addCounter(3 + i, 363 * i + 400);
    }

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 48; j++) {
            addRandomChar(3 + i, 20 + j, Math.random() * 10000 + 1000);
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
