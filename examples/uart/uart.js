function test() {
	var fd = uart.open('/dev/ttyUSB0');
	uart.write(fd, 'hello world\n');
	var i = 10;
	while (i--) {
		var buf = uart.read(fd);
		print('JS read: ' + buf + '\n');
	}
	uart.close(fd);
}

test();
