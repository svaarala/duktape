/*
 *  Create Object using a literal
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj;
    var i;

    for (i = 0; i < 1e6; i++) {
        obj = {
            key1: 'val1',
            key2: 'val2',
            key3: 'val3',
            key4: 'val4',
            key5: 'val5',
            key6: 'val6',
            key7: 'val7',
            key8: 'val8',
            key9: 'val9',
            key10: 'val10',
            key11: 'val11',
            key12: 'val12',
            key13: 'val13',
            key14: 'val14',
            key15: 'val15',
            key16: 'val16',
            key17: 'val17',
            key18: 'val18',
            key19: 'val19',
            key20: 'val20'
        };
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
