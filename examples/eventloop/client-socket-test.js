
var HOST = 'localhost';
var PORT = 80;
var EXIT_TIMEOUT = 300e3;

print('automatic exit after ' + (EXIT_TIMEOUT / 1e3) + ' seconds');
setTimeout(function () {
    print('exit timer');
}, EXIT_TIMEOUT);

_EVENTLOOP.connect(HOST, PORT, function (fd) {
    print('connected to ' + HOST + ':' + PORT + ', fd', fd);
    _EVENTLOOP.setReader(fd, function (fd, data) {
        print('read from fd', fd);
        print(data);
        _EVENTLOOP.close(fd);
    });
    _EVENTLOOP.write(fd, "GET / HTTP/1.1\r\n" +
                         "Host: " + HOST + "\r\n" +
                         "User-Agent: client-socket-test.js\r\n" +
                         "\r\n");
});

