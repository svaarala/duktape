if (typeof print !== 'function') { print = console.log; }

var jobs = [];

function runJob(job) {
    var iter = 100000, k, c, xx, yy, xx2, yy2, x0 = job.x0, y0 = job.y0;

    for (k = 0, xx = 0, yy = 0, c = '#'; k < iter; k++) {
        xx2 = xx*xx; yy2 = yy*yy;

        if (xx2 + yy2 < 4.0) {
            yy = 2*xx*yy + y0;
            xx = xx2 - yy2 + x0;
        } else {
            /* xx^2 + yy^2 >= 4.0 */
            if (k < 3) { c = '.'; }
            else if (k < 5) { c = ','; }
            else if (k < 10) { c = '-'; }
            else { c = '='; }
            job.resolve(c);
            return;
        }
    }
    job.resolve('#');
}

function createMandelPromise() {
    var w = 76, h = 28;
    var i, j;
    var x0, y0;
    var row;
    var rows = [];
    var P;

    for (i = 0; i < h; i++) {
        y0 = (i / h) * 2.5 - 1.25;

        for (j = 0, row = []; j < w; j++) {
            x0 = (j / w) * 3.0 - 2.0;

            P = new Promise(function (resolve, reject) {
                jobs.push({ x0: x0, y0: y0, resolve: resolve, reject: reject });
            });
            row.push(P);
        }

        rows.push(Promise.all(row));
    }

    return Promise.all(rows);
}

try {
    createMandelPromise().then(function (rows) {
        rows.forEach(function (row) {
            print(row.join(''));
        });
    }, function (e) {
        print(e);
    });
    jobs.forEach(runJob);
} catch (e) {
    print(e.stack || e);
    throw e;
}
