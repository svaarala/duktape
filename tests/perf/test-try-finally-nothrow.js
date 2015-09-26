if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    for (i = 0; i < 1e7; i++) {
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
        try {
        } finally {
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
