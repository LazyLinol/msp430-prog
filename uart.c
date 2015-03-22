#include <msp430g2553.h>

#include "uart.h"

void initUart(void) {
  P1SEL |= BIT1 + BIT2;
  P1SEL2 |= BIT1 + BIT2;
  UCA0CTL1 |= UCSSEL_2;
  UCA0BR0 = 104;
  UCA0BR1 = 0;
  UCA0MCTL = UCBRS0;
  UCA0CTL1 &= ~UCSWRST;
  IE2 |= UCA0RXIE;
}

void sendUart(unsigned char tx) {
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = tx;
}

unsigned char recvUart() {
  while (!(IFG2&UCA0RXIFG));
  return UCA0RXBUF;
}

void sendUartString(unsigned char* str, int length) {
  int i = 0;
  for (i = 0; i < length; i++) {
    sendUart(str[i]);
  }
}
