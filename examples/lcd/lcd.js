 /*
 *  DS1602 js dirver.
 *  It will be a part of RUFF Project.
 *
 *  Copyright (c) 2015 by Nanchao (Shanghai) Inc.
 *  Written by Shanjin Yang <sjyangv0@qq.com>
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution
 */

// Define GPIO to LCD mapping
LCD_CHR = 1;
LCD_CMD = 0;
LCD_WIDTH = 16;

False = 0;
True = 1;

LCD_LINE_1 = 0x80; // LCD RAM address for the 1st line
LCD_LINE_2 = 0xC0; // LCD RAM address for the 2nd line

var gpio = require('./gpio.so');
var rs = gpio.open(7, 1);
var en = gpio.open(8, 1);
var d4 = gpio.open(9, 1);
var d5 = gpio.open(24, 1);
var d6 = gpio.open(23, 1);
var d7 = gpio.open(18, 1);

function delay(ms) {
	while(ms--);
}

function lcd_init() {
	/*Initialise display*/
	lcd_byte(0x33,LCD_CMD);  // 110011 Initialise
	lcd_byte(0x32,LCD_CMD);  // 110010 Initialise
	lcd_byte(0x06,LCD_CMD);  // 000110 Cursor move direction
	lcd_byte(0x0C,LCD_CMD);  // 001100 Display On,Cursor Off, Blink Off
	lcd_byte(0x28,LCD_CMD);  // 101000 Data length, number of lines, font size
	lcd_byte(0x01,LCD_CMD);  // 000001 Clear display
	/*TODO*/
	delay(10000);
}

function lcd_toggle_enable() {
	gpio.write(en, 1);
	delay(10000);
	gpio.write(en, 0);
}

function lcd_byte(bits, mode) { 
	gpio.write(rs, mode);
	gpio.write(d4, 0);
	gpio.write(d5, 0);
	gpio.write(d6, 0);
	gpio.write(d7, 0);

	if ((bits&0x10) === 0x10) {
		gpio.write(d4, 1);
	}

	if ((bits&0x20) === 0x20) {
		gpio.write(d5, 1);
	}

	if ((bits&0x40)===0x40) {
		gpio.write(d6, 1);
	}

	if ((bits&0x80)===0x80) {
	  	gpio.write(d7, 1);
	}
	
	// Toggle 'Enable' pin
	lcd_toggle_enable();

	gpio.write(d4, 0);
	gpio.write(d5, 0);
	gpio.write(d6, 0);
	gpio.write(d7, 0);

	if ((bits&0x01)===0x01) {
		gpio.write(d4, 1);
	}

	if ((bits&0x02)===0x02) {
		gpio.write(d5, 1);
	}

	if ((bits&0x04)===0x04) {
		gpio.write(d6, 1);
	}

	if ((bits&0x08)===0x08) {
		gpio.write(d7, 1);
	}
	// Toggle 'Enable' pin
	lcd_toggle_enable();
}

function lcd_print(str) {
        var length = str.length;
	lcd_byte(LCD_LINE_1, LCD_CMD);
        for(var i = 0; (i < length) && (i < 16); i++) {
		lcd_byte(str.charCodeAt(i), LCD_CHR);
	}

	lcd_byte(LCD_LINE_2, LCD_CMD);
	for (; (i > 8) && (i < length); i++) {
		lcd_byte(str.charCodeAt(i), LCD_CHR);
	}
}

function main() {
	lcd_init();
	
	while (1) {
		lcd_print("hello world Shanjin Yang cool cool");
		delay(1000000);
	}
}

main();
