// XXX: util
function dumpValue(v) {
    var i, n;
    var clipped;
    var tmp = [];

    if (v === undefined) {
        return 'undefined';
    } else if (v === null) {
        return 'null';
    }

    n = Math.floor(v.length);
    if (n > 1000) {
        n = 1000;
        clipped = true;
    }
    for (i = 0; i < n; i++) {
        if (v.hasOwnProperty(i)) {
            tmp.push(v[i]);
        } else {
            tmp.push('nonexistent');
        }
    }
    if (clipped) {
        tmp.push('...');
    }

    return typeof v + ' ' + v.length + ' ' + tmp.join(',');
}

function test(this_value, args, no_pre_post) {
    var t;

    try {
        if (!no_pre_post) {
            print('pre', dumpValue(this_value));
        }
        t = Array.prototype.splice.apply(this_value, args);
        print('res', dumpValue(t));
    } catch (e) {
        print(e.name);
    }
    if (!no_pre_post) {
        print('post', dumpValue(this_value));
    }
}

/*===
basic
dense 0 0 -Infinity -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 1 -Infinity -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 2 -Infinity -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 3 -Infinity -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 4 -Infinity -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 5 -Infinity -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 6 -Infinity -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 7 -Infinity -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 8 -Infinity -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 9 -Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 10 -Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 0 11 -Infinity 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 0 12 -Infinity 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 0 13 -Infinity 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 0 14 -Infinity 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 0 15 -Infinity 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 0 16 -Infinity 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 0 17 -Infinity 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 0 18 -Infinity 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 0 19 -Infinity Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 0 20 -Infinity NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 0 -1000000000 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 1 -1000000000 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 2 -1000000000 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 3 -1000000000 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 4 -1000000000 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 5 -1000000000 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 6 -1000000000 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 7 -1000000000 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 8 -1000000000 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 9 -1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 10 -1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 1 11 -1000000000 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 1 12 -1000000000 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 1 13 -1000000000 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 1 14 -1000000000 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 1 15 -1000000000 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 1 16 -1000000000 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 1 17 -1000000000 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 1 18 -1000000000 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 1 19 -1000000000 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 1 20 -1000000000 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 0 -7 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 1 -7 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 2 -7 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 3 -7 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 4 -7 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 5 -7 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 6 -7 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 7 -7 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 8 -7 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 9 -7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 10 -7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 2 11 -7 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 2 12 -7 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 2 13 -7 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 2 14 -7 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 2 15 -7 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 2 16 -7 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 2 17 -7 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 2 18 -7 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 2 19 -7 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 2 20 -7 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 0 -6 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 1 -6 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 2 -6 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 3 -6 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 4 -6 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 5 -6 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 6 -6 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 7 -6 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 8 -6 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 9 -6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 10 -6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 3 11 -6 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 3 12 -6 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 3 13 -6 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 3 14 -6 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 3 15 -6 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 3 16 -6 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 3 17 -6 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 3 18 -6 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 3 19 -6 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 3 20 -6 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 0 -5 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 1 -5 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 2 -5 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 3 -5 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 4 -5 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 5 -5 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 6 -5 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 7 -5 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 8 -5 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 9 -5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 10 -5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 4 11 -5 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 4 12 -5 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 4 13 -5 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 4 14 -5 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 4 15 -5 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 4 16 -5 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 4 17 -5 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 4 18 -5 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 4 19 -5 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 4 20 -5 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 0 -4 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 1 -4 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 2 -4 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 3 -4 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 4 -4 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 5 -4 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 6 -4 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 7 -4 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 8 -4 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 9 -4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 10 -4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 5 11 -4 1
pre object 5 1,2,3,4,5
res object 1 2
post object 4 1,3,4,5
dense 5 12 -4 2
pre object 5 1,2,3,4,5
res object 2 2,3
post object 3 1,4,5
dense 5 13 -4 3
pre object 5 1,2,3,4,5
res object 3 2,3,4
post object 2 1,5
dense 5 14 -4 4
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 5 15 -4 5
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 5 16 -4 6
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 5 17 -4 7
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 5 18 -4 1000000000
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 5 19 -4 Infinity
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 5 20 -4 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 0 -3 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 1 -3 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 2 -3 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 3 -3 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 4 -3 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 5 -3 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 6 -3 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 7 -3 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 8 -3 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 9 -3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 10 -3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 6 11 -3 1
pre object 5 1,2,3,4,5
res object 1 3
post object 4 1,2,4,5
dense 6 12 -3 2
pre object 5 1,2,3,4,5
res object 2 3,4
post object 3 1,2,5
dense 6 13 -3 3
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 6 14 -3 4
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 6 15 -3 5
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 6 16 -3 6
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 6 17 -3 7
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 6 18 -3 1000000000
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 6 19 -3 Infinity
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 6 20 -3 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 0 -2 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 1 -2 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 2 -2 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 3 -2 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 4 -2 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 5 -2 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 6 -2 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 7 -2 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 8 -2 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 9 -2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 10 -2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 7 11 -2 1
pre object 5 1,2,3,4,5
res object 1 4
post object 4 1,2,3,5
dense 7 12 -2 2
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 13 -2 3
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 14 -2 4
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 15 -2 5
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 16 -2 6
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 17 -2 7
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 18 -2 1000000000
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 19 -2 Infinity
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 7 20 -2 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 0 -1 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 1 -1 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 2 -1 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 3 -1 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 4 -1 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 5 -1 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 6 -1 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 7 -1 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 8 -1 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 9 -1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 10 -1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 8 11 -1 1
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 12 -1 2
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 13 -1 3
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 14 -1 4
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 15 -1 5
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 16 -1 6
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 17 -1 7
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 18 -1 1000000000
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 19 -1 Infinity
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 8 20 -1 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 0 0 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 1 0 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 2 0 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 3 0 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 4 0 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 5 0 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 6 0 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 7 0 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 8 0 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 9 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 10 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 9 11 0 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 9 12 0 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 9 13 0 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 9 14 0 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 9 15 0 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 9 16 0 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 9 17 0 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 9 18 0 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 9 19 0 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 9 20 0 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 0 0 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 1 0 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 2 0 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 3 0 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 4 0 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 5 0 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 6 0 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 7 0 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 8 0 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 9 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 10 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 10 11 0 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 10 12 0 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 10 13 0 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 10 14 0 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 10 15 0 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 10 16 0 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 10 17 0 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 10 18 0 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 10 19 0 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 10 20 0 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 0 1 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 1 1 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 2 1 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 3 1 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 4 1 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 5 1 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 6 1 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 7 1 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 8 1 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 9 1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 10 1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 11 11 1 1
pre object 5 1,2,3,4,5
res object 1 2
post object 4 1,3,4,5
dense 11 12 1 2
pre object 5 1,2,3,4,5
res object 2 2,3
post object 3 1,4,5
dense 11 13 1 3
pre object 5 1,2,3,4,5
res object 3 2,3,4
post object 2 1,5
dense 11 14 1 4
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 11 15 1 5
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 11 16 1 6
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 11 17 1 7
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 11 18 1 1000000000
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 11 19 1 Infinity
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
dense 11 20 1 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 0 2 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 1 2 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 2 2 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 3 2 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 4 2 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 5 2 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 6 2 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 7 2 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 8 2 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 9 2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 10 2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 12 11 2 1
pre object 5 1,2,3,4,5
res object 1 3
post object 4 1,2,4,5
dense 12 12 2 2
pre object 5 1,2,3,4,5
res object 2 3,4
post object 3 1,2,5
dense 12 13 2 3
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 12 14 2 4
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 12 15 2 5
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 12 16 2 6
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 12 17 2 7
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 12 18 2 1000000000
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 12 19 2 Infinity
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
dense 12 20 2 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 0 3 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 1 3 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 2 3 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 3 3 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 4 3 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 5 3 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 6 3 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 7 3 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 8 3 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 9 3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 10 3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 13 11 3 1
pre object 5 1,2,3,4,5
res object 1 4
post object 4 1,2,3,5
dense 13 12 3 2
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 13 3 3
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 14 3 4
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 15 3 5
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 16 3 6
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 17 3 7
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 18 3 1000000000
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 19 3 Infinity
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
dense 13 20 3 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 0 4 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 1 4 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 2 4 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 3 4 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 4 4 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 5 4 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 6 4 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 7 4 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 8 4 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 9 4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 10 4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 14 11 4 1
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 12 4 2
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 13 4 3
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 14 4 4
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 15 4 5
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 16 4 6
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 17 4 7
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 18 4 1000000000
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 19 4 Infinity
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
dense 14 20 4 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 0 5 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 1 5 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 2 5 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 3 5 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 4 5 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 5 5 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 6 5 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 7 5 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 8 5 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 9 5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 10 5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 11 5 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 12 5 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 13 5 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 14 5 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 15 5 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 16 5 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 17 5 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 18 5 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 19 5 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 15 20 5 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 0 6 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 1 6 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 2 6 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 3 6 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 4 6 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 5 6 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 6 6 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 7 6 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 8 6 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 9 6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 10 6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 11 6 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 12 6 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 13 6 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 14 6 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 15 6 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 16 6 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 17 6 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 18 6 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 19 6 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 16 20 6 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 0 7 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 1 7 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 2 7 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 3 7 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 4 7 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 5 7 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 6 7 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 7 7 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 8 7 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 9 7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 10 7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 11 7 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 12 7 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 13 7 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 14 7 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 15 7 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 16 7 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 17 7 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 18 7 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 19 7 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 17 20 7 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 0 1000000000 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 1 1000000000 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 2 1000000000 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 3 1000000000 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 4 1000000000 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 5 1000000000 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 6 1000000000 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 7 1000000000 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 8 1000000000 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 9 1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 10 1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 11 1000000000 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 12 1000000000 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 13 1000000000 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 14 1000000000 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 15 1000000000 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 16 1000000000 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 17 1000000000 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 18 1000000000 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 19 1000000000 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 18 20 1000000000 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 0 Infinity -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 1 Infinity -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 2 Infinity -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 3 Infinity -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 4 Infinity -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 5 Infinity -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 6 Infinity -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 7 Infinity -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 8 Infinity -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 9 Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 10 Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 11 Infinity 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 12 Infinity 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 13 Infinity 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 14 Infinity 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 15 Infinity 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 16 Infinity 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 17 Infinity 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 18 Infinity 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 19 Infinity Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 19 20 Infinity NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 0 NaN -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 1 NaN -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 2 NaN -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 3 NaN -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 4 NaN -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 5 NaN -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 6 NaN -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 7 NaN -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 8 NaN -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 9 NaN 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 10 NaN 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
dense 20 11 NaN 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
dense 20 12 NaN 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
dense 20 13 NaN 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
dense 20 14 NaN 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
dense 20 15 NaN 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 20 16 NaN 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 20 17 NaN 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 20 18 NaN 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 20 19 NaN Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
dense 20 20 NaN NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 0 -Infinity -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 1 -Infinity -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 2 -Infinity -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 3 -Infinity -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 4 -Infinity -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 5 -Infinity -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 6 -Infinity -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 7 -Infinity -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 8 -Infinity -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 9 -Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 10 -Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 0 11 -Infinity 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 0 12 -Infinity 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 0 13 -Infinity 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 0 14 -Infinity 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 0 15 -Infinity 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 0 16 -Infinity 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 0 17 -Infinity 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 0 18 -Infinity 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 0 19 -Infinity Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 0 20 -Infinity NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 0 -1000000000 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 1 -1000000000 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 2 -1000000000 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 3 -1000000000 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 4 -1000000000 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 5 -1000000000 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 6 -1000000000 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 7 -1000000000 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 8 -1000000000 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 9 -1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 10 -1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 1 11 -1000000000 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 1 12 -1000000000 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 1 13 -1000000000 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 1 14 -1000000000 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 1 15 -1000000000 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 1 16 -1000000000 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 1 17 -1000000000 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 1 18 -1000000000 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 1 19 -1000000000 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 1 20 -1000000000 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 0 -7 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 1 -7 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 2 -7 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 3 -7 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 4 -7 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 5 -7 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 6 -7 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 7 -7 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 8 -7 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 9 -7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 10 -7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 2 11 -7 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 2 12 -7 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 2 13 -7 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 2 14 -7 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 2 15 -7 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 2 16 -7 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 2 17 -7 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 2 18 -7 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 2 19 -7 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 2 20 -7 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 0 -6 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 1 -6 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 2 -6 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 3 -6 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 4 -6 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 5 -6 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 6 -6 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 7 -6 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 8 -6 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 9 -6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 10 -6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 3 11 -6 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 3 12 -6 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 3 13 -6 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 3 14 -6 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 3 15 -6 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 3 16 -6 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 3 17 -6 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 3 18 -6 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 3 19 -6 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 3 20 -6 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 0 -5 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 1 -5 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 2 -5 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 3 -5 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 4 -5 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 5 -5 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 6 -5 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 7 -5 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 8 -5 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 9 -5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 10 -5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 4 11 -5 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 4 12 -5 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 4 13 -5 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 4 14 -5 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 4 15 -5 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 4 16 -5 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 4 17 -5 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 4 18 -5 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 4 19 -5 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 4 20 -5 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 0 -4 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 1 -4 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 2 -4 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 3 -4 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 4 -4 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 5 -4 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 6 -4 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 7 -4 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 8 -4 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 9 -4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 10 -4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 5 11 -4 1
pre object 5 1,2,3,4,5
res object 1 2
post object 4 1,3,4,5
sparse 5 12 -4 2
pre object 5 1,2,3,4,5
res object 2 2,3
post object 3 1,4,5
sparse 5 13 -4 3
pre object 5 1,2,3,4,5
res object 3 2,3,4
post object 2 1,5
sparse 5 14 -4 4
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 5 15 -4 5
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 5 16 -4 6
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 5 17 -4 7
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 5 18 -4 1000000000
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 5 19 -4 Infinity
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 5 20 -4 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 0 -3 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 1 -3 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 2 -3 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 3 -3 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 4 -3 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 5 -3 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 6 -3 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 7 -3 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 8 -3 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 9 -3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 10 -3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 6 11 -3 1
pre object 5 1,2,3,4,5
res object 1 3
post object 4 1,2,4,5
sparse 6 12 -3 2
pre object 5 1,2,3,4,5
res object 2 3,4
post object 3 1,2,5
sparse 6 13 -3 3
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 6 14 -3 4
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 6 15 -3 5
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 6 16 -3 6
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 6 17 -3 7
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 6 18 -3 1000000000
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 6 19 -3 Infinity
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 6 20 -3 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 0 -2 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 1 -2 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 2 -2 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 3 -2 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 4 -2 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 5 -2 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 6 -2 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 7 -2 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 8 -2 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 9 -2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 10 -2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 7 11 -2 1
pre object 5 1,2,3,4,5
res object 1 4
post object 4 1,2,3,5
sparse 7 12 -2 2
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 13 -2 3
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 14 -2 4
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 15 -2 5
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 16 -2 6
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 17 -2 7
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 18 -2 1000000000
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 19 -2 Infinity
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 7 20 -2 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 0 -1 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 1 -1 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 2 -1 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 3 -1 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 4 -1 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 5 -1 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 6 -1 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 7 -1 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 8 -1 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 9 -1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 10 -1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 8 11 -1 1
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 12 -1 2
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 13 -1 3
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 14 -1 4
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 15 -1 5
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 16 -1 6
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 17 -1 7
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 18 -1 1000000000
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 19 -1 Infinity
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 8 20 -1 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 0 0 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 1 0 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 2 0 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 3 0 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 4 0 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 5 0 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 6 0 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 7 0 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 8 0 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 9 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 10 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 9 11 0 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 9 12 0 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 9 13 0 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 9 14 0 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 9 15 0 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 9 16 0 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 9 17 0 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 9 18 0 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 9 19 0 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 9 20 0 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 0 0 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 1 0 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 2 0 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 3 0 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 4 0 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 5 0 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 6 0 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 7 0 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 8 0 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 9 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 10 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 10 11 0 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 10 12 0 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 10 13 0 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 10 14 0 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 10 15 0 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 10 16 0 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 10 17 0 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 10 18 0 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 10 19 0 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 10 20 0 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 0 1 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 1 1 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 2 1 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 3 1 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 4 1 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 5 1 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 6 1 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 7 1 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 8 1 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 9 1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 10 1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 11 11 1 1
pre object 5 1,2,3,4,5
res object 1 2
post object 4 1,3,4,5
sparse 11 12 1 2
pre object 5 1,2,3,4,5
res object 2 2,3
post object 3 1,4,5
sparse 11 13 1 3
pre object 5 1,2,3,4,5
res object 3 2,3,4
post object 2 1,5
sparse 11 14 1 4
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 11 15 1 5
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 11 16 1 6
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 11 17 1 7
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 11 18 1 1000000000
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 11 19 1 Infinity
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
sparse 11 20 1 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 0 2 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 1 2 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 2 2 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 3 2 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 4 2 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 5 2 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 6 2 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 7 2 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 8 2 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 9 2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 10 2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 12 11 2 1
pre object 5 1,2,3,4,5
res object 1 3
post object 4 1,2,4,5
sparse 12 12 2 2
pre object 5 1,2,3,4,5
res object 2 3,4
post object 3 1,2,5
sparse 12 13 2 3
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 12 14 2 4
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 12 15 2 5
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 12 16 2 6
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 12 17 2 7
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 12 18 2 1000000000
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 12 19 2 Infinity
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
sparse 12 20 2 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 0 3 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 1 3 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 2 3 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 3 3 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 4 3 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 5 3 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 6 3 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 7 3 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 8 3 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 9 3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 10 3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 13 11 3 1
pre object 5 1,2,3,4,5
res object 1 4
post object 4 1,2,3,5
sparse 13 12 3 2
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 13 3 3
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 14 3 4
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 15 3 5
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 16 3 6
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 17 3 7
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 18 3 1000000000
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 19 3 Infinity
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
sparse 13 20 3 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 0 4 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 1 4 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 2 4 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 3 4 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 4 4 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 5 4 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 6 4 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 7 4 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 8 4 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 9 4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 10 4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 14 11 4 1
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 12 4 2
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 13 4 3
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 14 4 4
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 15 4 5
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 16 4 6
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 17 4 7
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 18 4 1000000000
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 19 4 Infinity
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
sparse 14 20 4 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 0 5 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 1 5 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 2 5 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 3 5 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 4 5 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 5 5 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 6 5 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 7 5 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 8 5 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 9 5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 10 5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 11 5 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 12 5 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 13 5 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 14 5 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 15 5 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 16 5 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 17 5 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 18 5 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 19 5 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 15 20 5 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 0 6 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 1 6 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 2 6 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 3 6 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 4 6 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 5 6 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 6 6 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 7 6 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 8 6 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 9 6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 10 6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 11 6 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 12 6 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 13 6 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 14 6 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 15 6 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 16 6 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 17 6 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 18 6 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 19 6 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 16 20 6 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 0 7 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 1 7 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 2 7 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 3 7 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 4 7 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 5 7 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 6 7 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 7 7 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 8 7 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 9 7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 10 7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 11 7 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 12 7 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 13 7 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 14 7 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 15 7 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 16 7 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 17 7 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 18 7 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 19 7 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 17 20 7 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 0 1000000000 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 1 1000000000 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 2 1000000000 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 3 1000000000 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 4 1000000000 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 5 1000000000 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 6 1000000000 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 7 1000000000 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 8 1000000000 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 9 1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 10 1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 11 1000000000 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 12 1000000000 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 13 1000000000 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 14 1000000000 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 15 1000000000 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 16 1000000000 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 17 1000000000 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 18 1000000000 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 19 1000000000 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 18 20 1000000000 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 0 Infinity -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 1 Infinity -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 2 Infinity -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 3 Infinity -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 4 Infinity -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 5 Infinity -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 6 Infinity -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 7 Infinity -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 8 Infinity -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 9 Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 10 Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 11 Infinity 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 12 Infinity 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 13 Infinity 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 14 Infinity 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 15 Infinity 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 16 Infinity 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 17 Infinity 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 18 Infinity 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 19 Infinity Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 19 20 Infinity NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 0 NaN -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 1 NaN -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 2 NaN -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 3 NaN -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 4 NaN -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 5 NaN -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 6 NaN -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 7 NaN -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 8 NaN -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 9 NaN 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 10 NaN 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
sparse 20 11 NaN 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
sparse 20 12 NaN 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
sparse 20 13 NaN 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
sparse 20 14 NaN 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
sparse 20 15 NaN 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 20 16 NaN 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 20 17 NaN 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 20 18 NaN 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 20 19 NaN Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
sparse 20 20 NaN NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 0 -Infinity -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 1 -Infinity -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 2 -Infinity -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 3 -Infinity -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 4 -Infinity -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 5 -Infinity -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 6 -Infinity -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 7 -Infinity -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 8 -Infinity -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 9 -Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 10 -Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 0 11 -Infinity 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 0 12 -Infinity 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 0 13 -Infinity 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 0 14 -Infinity 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 0 15 -Infinity 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 0 16 -Infinity 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 0 17 -Infinity 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 0 18 -Infinity 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 0 19 -Infinity Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 0 20 -Infinity NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 0 -1000000000 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 1 -1000000000 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 2 -1000000000 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 3 -1000000000 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 4 -1000000000 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 5 -1000000000 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 6 -1000000000 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 7 -1000000000 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 8 -1000000000 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 9 -1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 10 -1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 1 11 -1000000000 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 1 12 -1000000000 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 1 13 -1000000000 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 1 14 -1000000000 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 1 15 -1000000000 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 1 16 -1000000000 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 1 17 -1000000000 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 1 18 -1000000000 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 1 19 -1000000000 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 1 20 -1000000000 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 0 -7 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 1 -7 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 2 -7 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 3 -7 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 4 -7 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 5 -7 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 6 -7 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 7 -7 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 8 -7 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 9 -7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 10 -7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 2 11 -7 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 2 12 -7 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 2 13 -7 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 2 14 -7 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 2 15 -7 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 2 16 -7 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 2 17 -7 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 2 18 -7 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 2 19 -7 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 2 20 -7 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 0 -6 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 1 -6 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 2 -6 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 3 -6 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 4 -6 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 5 -6 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 6 -6 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 7 -6 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 8 -6 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 9 -6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 10 -6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 3 11 -6 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 3 12 -6 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 3 13 -6 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 3 14 -6 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 3 15 -6 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 3 16 -6 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 3 17 -6 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 3 18 -6 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 3 19 -6 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 3 20 -6 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 0 -5 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 1 -5 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 2 -5 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 3 -5 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 4 -5 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 5 -5 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 6 -5 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 7 -5 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 8 -5 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 9 -5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 10 -5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 4 11 -5 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 4 12 -5 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 4 13 -5 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 4 14 -5 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 4 15 -5 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 4 16 -5 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 4 17 -5 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 4 18 -5 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 4 19 -5 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 4 20 -5 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 0 -4 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 1 -4 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 2 -4 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 3 -4 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 4 -4 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 5 -4 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 6 -4 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 7 -4 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 8 -4 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 9 -4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 10 -4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 5 11 -4 1
pre object 5 1,2,3,4,5
res object 1 2
post object 4 1,3,4,5
nonarray 5 12 -4 2
pre object 5 1,2,3,4,5
res object 2 2,3
post object 3 1,4,5
nonarray 5 13 -4 3
pre object 5 1,2,3,4,5
res object 3 2,3,4
post object 2 1,5
nonarray 5 14 -4 4
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 5 15 -4 5
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 5 16 -4 6
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 5 17 -4 7
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 5 18 -4 1000000000
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 5 19 -4 Infinity
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 5 20 -4 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 0 -3 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 1 -3 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 2 -3 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 3 -3 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 4 -3 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 5 -3 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 6 -3 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 7 -3 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 8 -3 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 9 -3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 10 -3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 6 11 -3 1
pre object 5 1,2,3,4,5
res object 1 3
post object 4 1,2,4,5
nonarray 6 12 -3 2
pre object 5 1,2,3,4,5
res object 2 3,4
post object 3 1,2,5
nonarray 6 13 -3 3
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 6 14 -3 4
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 6 15 -3 5
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 6 16 -3 6
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 6 17 -3 7
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 6 18 -3 1000000000
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 6 19 -3 Infinity
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 6 20 -3 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 0 -2 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 1 -2 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 2 -2 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 3 -2 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 4 -2 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 5 -2 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 6 -2 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 7 -2 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 8 -2 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 9 -2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 10 -2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 7 11 -2 1
pre object 5 1,2,3,4,5
res object 1 4
post object 4 1,2,3,5
nonarray 7 12 -2 2
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 13 -2 3
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 14 -2 4
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 15 -2 5
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 16 -2 6
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 17 -2 7
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 18 -2 1000000000
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 19 -2 Infinity
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 7 20 -2 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 0 -1 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 1 -1 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 2 -1 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 3 -1 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 4 -1 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 5 -1 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 6 -1 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 7 -1 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 8 -1 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 9 -1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 10 -1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 8 11 -1 1
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 12 -1 2
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 13 -1 3
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 14 -1 4
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 15 -1 5
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 16 -1 6
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 17 -1 7
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 18 -1 1000000000
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 19 -1 Infinity
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 8 20 -1 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 0 0 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 1 0 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 2 0 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 3 0 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 4 0 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 5 0 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 6 0 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 7 0 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 8 0 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 9 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 10 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 9 11 0 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 9 12 0 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 9 13 0 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 9 14 0 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 9 15 0 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 9 16 0 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 9 17 0 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 9 18 0 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 9 19 0 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 9 20 0 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 0 0 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 1 0 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 2 0 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 3 0 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 4 0 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 5 0 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 6 0 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 7 0 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 8 0 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 9 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 10 0 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 10 11 0 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 10 12 0 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 10 13 0 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 10 14 0 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 10 15 0 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 10 16 0 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 10 17 0 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 10 18 0 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 10 19 0 Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 10 20 0 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 0 1 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 1 1 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 2 1 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 3 1 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 4 1 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 5 1 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 6 1 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 7 1 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 8 1 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 9 1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 10 1 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 11 11 1 1
pre object 5 1,2,3,4,5
res object 1 2
post object 4 1,3,4,5
nonarray 11 12 1 2
pre object 5 1,2,3,4,5
res object 2 2,3
post object 3 1,4,5
nonarray 11 13 1 3
pre object 5 1,2,3,4,5
res object 3 2,3,4
post object 2 1,5
nonarray 11 14 1 4
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 11 15 1 5
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 11 16 1 6
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 11 17 1 7
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 11 18 1 1000000000
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 11 19 1 Infinity
pre object 5 1,2,3,4,5
res object 4 2,3,4,5
post object 1 1
nonarray 11 20 1 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 0 2 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 1 2 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 2 2 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 3 2 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 4 2 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 5 2 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 6 2 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 7 2 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 8 2 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 9 2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 10 2 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 12 11 2 1
pre object 5 1,2,3,4,5
res object 1 3
post object 4 1,2,4,5
nonarray 12 12 2 2
pre object 5 1,2,3,4,5
res object 2 3,4
post object 3 1,2,5
nonarray 12 13 2 3
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 12 14 2 4
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 12 15 2 5
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 12 16 2 6
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 12 17 2 7
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 12 18 2 1000000000
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 12 19 2 Infinity
pre object 5 1,2,3,4,5
res object 3 3,4,5
post object 2 1,2
nonarray 12 20 2 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 0 3 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 1 3 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 2 3 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 3 3 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 4 3 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 5 3 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 6 3 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 7 3 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 8 3 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 9 3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 10 3 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 13 11 3 1
pre object 5 1,2,3,4,5
res object 1 4
post object 4 1,2,3,5
nonarray 13 12 3 2
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 13 3 3
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 14 3 4
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 15 3 5
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 16 3 6
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 17 3 7
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 18 3 1000000000
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 19 3 Infinity
pre object 5 1,2,3,4,5
res object 2 4,5
post object 3 1,2,3
nonarray 13 20 3 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 0 4 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 1 4 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 2 4 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 3 4 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 4 4 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 5 4 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 6 4 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 7 4 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 8 4 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 9 4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 10 4 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 14 11 4 1
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 12 4 2
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 13 4 3
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 14 4 4
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 15 4 5
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 16 4 6
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 17 4 7
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 18 4 1000000000
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 19 4 Infinity
pre object 5 1,2,3,4,5
res object 1 5
post object 4 1,2,3,4
nonarray 14 20 4 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 0 5 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 1 5 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 2 5 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 3 5 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 4 5 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 5 5 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 6 5 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 7 5 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 8 5 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 9 5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 10 5 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 11 5 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 12 5 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 13 5 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 14 5 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 15 5 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 16 5 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 17 5 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 18 5 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 19 5 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 15 20 5 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 0 6 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 1 6 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 2 6 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 3 6 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 4 6 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 5 6 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 6 6 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 7 6 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 8 6 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 9 6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 10 6 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 11 6 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 12 6 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 13 6 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 14 6 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 15 6 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 16 6 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 17 6 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 18 6 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 19 6 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 16 20 6 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 0 7 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 1 7 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 2 7 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 3 7 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 4 7 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 5 7 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 6 7 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 7 7 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 8 7 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 9 7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 10 7 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 11 7 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 12 7 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 13 7 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 14 7 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 15 7 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 16 7 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 17 7 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 18 7 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 19 7 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 17 20 7 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 0 1000000000 -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 1 1000000000 -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 2 1000000000 -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 3 1000000000 -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 4 1000000000 -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 5 1000000000 -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 6 1000000000 -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 7 1000000000 -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 8 1000000000 -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 9 1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 10 1000000000 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 11 1000000000 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 12 1000000000 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 13 1000000000 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 14 1000000000 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 15 1000000000 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 16 1000000000 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 17 1000000000 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 18 1000000000 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 19 1000000000 Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 18 20 1000000000 NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 0 Infinity -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 1 Infinity -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 2 Infinity -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 3 Infinity -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 4 Infinity -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 5 Infinity -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 6 Infinity -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 7 Infinity -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 8 Infinity -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 9 Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 10 Infinity 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 11 Infinity 1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 12 Infinity 2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 13 Infinity 3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 14 Infinity 4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 15 Infinity 5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 16 Infinity 6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 17 Infinity 7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 18 Infinity 1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 19 Infinity Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 19 20 Infinity NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 0 NaN -Infinity
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 1 NaN -1000000000
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 2 NaN -7
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 3 NaN -6
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 4 NaN -5
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 5 NaN -4
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 6 NaN -3
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 7 NaN -2
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 8 NaN -1
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 9 NaN 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 10 NaN 0
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
nonarray 20 11 NaN 1
pre object 5 1,2,3,4,5
res object 1 1
post object 4 2,3,4,5
nonarray 20 12 NaN 2
pre object 5 1,2,3,4,5
res object 2 1,2
post object 3 3,4,5
nonarray 20 13 NaN 3
pre object 5 1,2,3,4,5
res object 3 1,2,3
post object 2 4,5
nonarray 20 14 NaN 4
pre object 5 1,2,3,4,5
res object 4 1,2,3,4
post object 1 5
nonarray 20 15 NaN 5
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 20 16 NaN 6
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 20 17 NaN 7
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 20 18 NaN 1000000000
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 20 19 NaN Infinity
pre object 5 1,2,3,4,5
res object 5 1,2,3,4,5
post object 0 
nonarray 20 20 NaN NaN
pre object 5 1,2,3,4,5
res object 0 
post object 5 1,2,3,4,5
pre object 0 
res object 0 
post object 0 
pre object 3 1,2,3
res object 0 
post object 3 1,2,3
pre object 0 
res object 0 
post object 0 
pre object 3 1,2,3
res object 2 1,2
post object 1 3
===*/

print('basic');

function basicTest() {
    var obj;
    var s, d;
    var i;
    var start_vals = [ Number.NEGATIVE_INFINITY, -1e9, -7, -6, -5, -4, -3, -2, -1, -0, +0,
                       1, 2, 3, 4, 5, 6, 7, 1e9, Number.POSITIVE_INFINITY, Number.NaN ];
    var delcount_vals = [ Number.NEGATIVE_INFINITY, -1e9, -7, -6, -5, -4, -3, -2, -1, -0, +0,
                          1, 2, 3, 4, 5, 6, 7, 1e9, Number.POSITIVE_INFINITY, Number.NaN ];

    // dense

    for (i = 0; i < start_vals.length; i++) {
        for (j = 0; j < delcount_vals.length; j++) {
            s = start_vals[i]; d = delcount_vals[j];
            print('dense', i, j, s, d);
            test([1,2,3,4,5], [s,d]);
        }
    }

    // sparse

    for (i = 0; i < start_vals.length; i++) {
        for (j = 0; j < delcount_vals.length; j++) {
            s = start_vals[i]; d = delcount_vals[j];
            print('sparse', i, j, s, d);
            obj = [1];
            obj[100] = 'foo';
            obj[1] = 2; obj[2] = 3; obj[3] = 4; obj[4] = 5;
            obj.length = 5;
            test(obj, [s,d]);
        }
    }

    // non-object

    for (i = 0; i < start_vals.length; i++) {
        for (j = 0; j < delcount_vals.length; j++) {
            s = start_vals[i]; d = delcount_vals[j];
            print('nonarray', i, j, s, d);
            obj = { '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, length: 5 };
            test(obj, [s,d]);
        }
    }

    // zero deleteCount without items

    test([], [0, 0]);

    // zero deleteCount with items

    test([1,2,3], [0, 0]);

    // non-zero deleteCount without items

    test([], [0, 2]);

    // non-zero deleteCount with items

    test([1,2,3], [0, 2]);

    // NOTE: don't test for missing deleteCount: the standard behavior for
    // that is to treat deleteCount as undefined (same as 0) but the real
    // world behavior is to splice to end of array.  This is covered by a
    // separate testcase.
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
protected
pre object 5 1,2,3,4,5
TypeError
post object 5 1,2,3,4,5
===*/

print('protected');

function protectedTest() {
    var obj;

    // XXX: protected elements, this test is incomplete

    // 'length' gets written regardless of whether it would change,
    // causing a TypeError if protected

    obj = { '0': 1, '1': 2, '2': 3, '3': 4, '4': 5 };
    Object.defineProperties(obj, {
        'length': { value: 5, writable: false, enumerable: false, configurable: false }
    });
    test(obj, [0, 0]);  // no change, but length still written
}

try {
    protectedTest();
} catch (e) {
    print(e);
}

/*===
coercion
pre undefined
TypeError
post undefined
pre null
TypeError
post null
pre boolean undefined 
res object 0 
post boolean undefined 
pre boolean undefined 
res object 0 
post boolean undefined 
pre number undefined 
res object 0 
post number undefined 
pre string 4 q,u,u,x
TypeError
post string 4 q,u,u,x
pre object 2 1,2
res object 0 
post object 2 1,2
pre object undefined 
res object 0 
post object 0 
res object 3 1,2,3
res object 3 1,2,3
res object 4 1,2,3,4
length valueOf
start valueOf
deleteCount valueOf
res object 2 2,3
===*/

print('coercion');

function coercionTest() {
    var obj;

    // this coercion, arguments [0,0] are given to avoid the case where
    // deleteCount is omitted (it has different standard and real world
    // behavior and is covered by a separate testcase).

    test(undefined, [0,0]);
    test(null, [0,0]);
    test(true, [0,0]);
    test(false, [0,0]);
    test(123, [0,0]);
    test('quux', [0,0]);  // even though deleteCount=0 and no new items, 'length' is written, which causes TypeError
    test([1,2], [0,0]);
    test({ foo: 1, bar: 2 }, [0,0]);

    // length coercion

    obj = { '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, '5': 6, '6': 7, length: '3.9' };
    test(obj, [0, 100], true);

    obj = { '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, '5': 6, '6': 7, length: 256*256*256*256 + 3.9 };
    test(obj, [0, 100], true);

    obj = { '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, '5': 6, '6': 7, length: -256*256*256*256 + 3.9 };
    test(obj, [0, 100], true);

    // coercion order: ToUint32(length), ToInteger(start), ToInteger(deleteCount)

    obj = { '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, '5': 6, '6': 7, length: {
        toString: function() { print('length toString'); return -256*256*256*256 + 4; },
        valueOf: function() { print('length valueOf'); return 256*256*256*256 + 5; }
    } };
    test(obj, [
        {
            toString: function() { print('start toString'); return 2; },
            valueOf: function() { print('start valueOf'); return 1; }
        },
        {
            toString: function() { print('deleteCount toString'); return 3; },
            valueOf: function() { print('deleteCount valueOf'); return 2; }
        }
    ], true);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

// XXX: attributes test
