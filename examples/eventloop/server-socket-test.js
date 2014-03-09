
var HOST = 'localhost'
var PORT = 12345;
var EXIT_TIMEOUT = 300e3;

print('automatic exit after ' + (EXIT_TIMEOUT / 1e3) + ' seconds');
setTimeout(function () {
    print('exit timer');
}, EXIT_TIMEOUT);

print('listen on ' + HOST + ':' + PORT);
_EVENTLOOP.server(HOST, PORT, function (fd, addr, port) {
    print('new connection on fd ' + fd + ' from ' + addr + ':' + port);
    _EVENTLOOP.setReader(fd, function (fd, data) {
        print('read data on fd ' + fd + ', length ' + data.length);
        _EVENTLOOP.write(fd, String(data).toUpperCase());
    });
});
