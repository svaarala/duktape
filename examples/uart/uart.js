function test() {
	uart.open('/dev/ttyUSB0');
	uart.write(3, 'hello world\n');
	uart.read(3);
	uart.close(3);
}
test();
