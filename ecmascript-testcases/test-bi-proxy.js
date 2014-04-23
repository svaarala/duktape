/*
 *  Proxy (ES6 draft)
 */

/*===
===*/

function test() {
    print('Proxy exists:', 'Proxy' in this, typeof this.Proxy);
    print('Proxy.revocable exists:', 'revocable' in this.Proxy, typeof this.Proxy.revocable);

    // FIXME: descriptor dump
    // FIXME: lengths and names
}

try {
    test();
} catch (e) {
    print(e);
}

// FIXME: proxy creation and use
// FIXME: proxy revocation
