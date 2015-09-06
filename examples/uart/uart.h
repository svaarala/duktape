#ifndef UART_H
#define UART_H

int uart_write(int fd, char *buf, int size);
int uart_read(int fd, char *buf, int size);
int uart_open(char *filename);
int uart_close(int fd);

#endif
