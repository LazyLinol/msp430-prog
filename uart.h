#ifndef UART_H
#define UART_H

void initUart(void);
void sendUart(unsigned char tx);
unsigned char recvUart();
void sendUartString(unsigned char* str, int length);

#endif
