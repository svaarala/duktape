function test(){
	uart.open('/dev/ttyUSB0');
	uart.write(3, 'hello world\n');
	uart.close(3);
	}
test();
