#include <msp430g2553.h>

#include "spi.h"

void initSpi(void) {
  P1SEL = BIT5 + BIT6 + BIT7;
  P1SEL2 = BIT5 + BIT6 + BIT7;

  UCB0CTL0 |= UCMSB + UCMST + UCSYNC;// + UCCKPH;
  UCB0CTL1 |= UCSSEL_2;
  UCB0BR0 |= 0x02;
  UCB0BR1 = 0;
  //UCB0MCTL = 0;
  UCB0CTL1 &= ~UCSWRST;
}

unsigned char readWriteSpi(unsigned char tx) {
  //IFG2 &= ~UCB0RXIFG;
  UCB0TXBUF = tx;
  while (!(IFG2 & UCB0RXIFG));
  return UCB0RXBUF;
}
