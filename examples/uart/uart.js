function test() {
	var fd = uart.open('/dev/ttyUSB0');
	fd = 3;
	//print('received message, type: %d', (int) fd);
	uart.write(fd, 'hello world\n');
	uart.read(fd);
	uart.close(fd);
}
test();
