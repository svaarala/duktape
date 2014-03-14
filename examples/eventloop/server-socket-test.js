
var HOST = 'localhost'
var PORT = 12345;
var EXIT_TIMEOUT = 300e3;

print('automatic exit after ' + (EXIT_TIMEOUT / 1e3) + ' seconds');
setTimeout(function () {
    print('exit timer');
    EventLoop.requestExit();
}, EXIT_TIMEOUT);

print('listen on ' + HOST + ':' + PORT);
EventLoop.server(HOST, PORT, function (fd, addr, port) {
    print('new connection on fd ' + fd + ' from ' + addr + ':' + port);
    EventLoop.setReader(fd, function (fd, data) {
        // FIXME: uppercase as bytes, no decode errors
        print('read data on fd ' + fd + ', length ' + data.length);
        EventLoop.write(fd, String(data).toUpperCase());
    });
});
