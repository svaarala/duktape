#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define FALSE  -1
#define TRUE   0

struct termios options;

int speed_arr[] = {
	B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
};

int name_arr[] = {
	115200, 38400, 19200, 9600, 4800, 2400, 1200, 300,
	115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
};

static int _enable_raw_mode(int fd, struct termios *orig_termios)
{
	struct termios raw;

	if (!isatty(fd)) goto fatal;
	/* XXX:
	 if (!atexit_registered) {
	    atexit(linenoiseAtExit);
	    atexit_registered = 1;
	}*/
	if (tcgetattr(fd, orig_termios) == -1) goto fatal;

	raw = *orig_termios;  /* modify the original mode */
	/* input modes: no break, no CR to NL, no parity check, no strip char,
	 * no start/stop output control. */
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	/* output modes - disable post processing */
	/* raw.c_oflag &= ~(OPOST); */
	/* control modes - set 8 bit chars */
	raw.c_cflag |= (CS8);
	/* local modes - choing off, canonical off, no extended functions,
	 * no signal chars (^Z,^C) */
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN
			 /* | ISIG XXX: ignore Ctrl-C in future */
		);
	/* control chars - set return condition: min number of bytes and timer.
	 * We want read to return every single byte, without timeout. */
	raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

	/* put terminal in raw mode after flushing */
	if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;
		return 0;

fatal:
	errno = ENOTTY;
	return -1;
}

static int _disable_raw_mode(int fd, struct termios *orig_termios)
{
	if (tcsetattr(fd, TCSAFLUSH, orig_termios) < 0) goto fatal;
		return 0;

fatal:
	errno = ENOTTY;
	return -1;
}

int set_speed(int fd, int speed)
{
	int i;
	int status;
	struct termios opt;
	tcgetattr(fd, &opt);
	for ( i= 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&opt, speed_arr[i]);
			cfsetospeed(&opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &opt);
			if  (status != 0) {
				perror("tcsetattr fd1");
				return 0;
			}
			tcflush(fd,TCIOFLUSH);
		}
	}

	return 0;
}

int set_parity(int fd,int databits,int stopbits,int parity)
{
	if  ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n"); return (FALSE);
	}
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;             /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;    /* Enable parity */
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;    /* Disnable parity checking */
		break;
	case 'S':
	case 's':  /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
	}
	/* 设置停止位*/
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return (FALSE);
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* timeout 15 seconds*/
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/
	return (TRUE);
}

int uart_write(int fd, char *buf, int size)
{
	if ((fd < 0) && (buf == NULL) && (size < 0))
		assert(0);
	
	int len = write(fd, buf, size);
	return len;
}

int uart_read(int fd, char *buf, int size)
{
	if ((fd < 0) && (buf == NULL) && (size < 0))
		assert(0);
	
	int len = read(fd, buf, size);
	return len;
}

int uart_close(int fd)
{
	_disable_raw_mode(fd, &options);
	close(fd);
	return 0;
}

int uart_open(char *filename)
{

	int fd;

	fd = open(filename, O_RDWR);

	if (fd < 0) {
		perror(filename);
		exit(1);
	}

	set_speed(fd,115200);
	if (set_parity(fd,8,1,'N') == FALSE)  {
		printf("Set Parity Error\n");
		exit (0);
	}
	_enable_raw_mode(fd, &options);
#define test_uart 0
#if test_uart
	int res;
	printf("Reading...\n");
	while(1) {
		res = uart_read(fd, buf, 255);
		char buf[256];

		uart_write(fd, buf, res);
		if(res==0)
			continue;
		buf[res]=0;

		fprintf(stderr, "%s", buf);

		if (buf[0] == 0x0d)
			printf("\n");

		if (buf[0] == '@') break;
	}

	_disable_raw_mode(fd, &options);
	printf("Close...\n");
	close(fd);
#endif
	return fd;
}
