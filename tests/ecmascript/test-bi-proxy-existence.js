/*===
proxy existence
Proxy exists: true function
Proxy.length: 2
Proxy.name: Proxy
Proxy desc: writable=true enumerable=false configurable=true
Proxy.revocable exists: true function
Proxy.revocable.length: 2
Proxy.revocable.name: revocable
Proxy.revocable desc: writable=true enumerable=false configurable=true
===*/

function proxyExistenceTest() {
    var pd;

    print('Proxy exists:', 'Proxy' in this, typeof this.Proxy);
    print('Proxy.length:', this.Proxy.length);
    print('Proxy.name:', this.Proxy.name);
    pd = Object.getOwnPropertyDescriptor(this, 'Proxy');
    if (pd) {
        print('Proxy desc:', 'writable=' + pd.writable, 'enumerable=' + pd.enumerable,
              'configurable=' + pd.configurable);
    }

    print('Proxy.revocable exists:', 'revocable' in this.Proxy, typeof this.Proxy.revocable);
    print('Proxy.revocable.length:', this.Proxy.revocable.length);
    print('Proxy.revocable.name:', this.Proxy.revocable.name);
    pd = Object.getOwnPropertyDescriptor(this.Proxy, 'revocable');
    if (pd) {
        print('Proxy.revocable desc:', 'writable=' + pd.writable, 'enumerable=' + pd.enumerable,
              'configurable=' + pd.configurable);
    }
}

print('proxy existence');

try {
    proxyExistenceTest();
} catch (e) {
    print(e.stack || e);
}
