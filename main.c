#include <msp430g2553.h>

#define LED1 BIT0
#define LED2 BIT6

void initUart() {
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

void sendUartString(char* str, int length) {
  int i = 0;
  for (i = 0; i < length; i++) {
    sendUart(str[i]);
  }
}

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL2 &= ~(DIVS_3);

    // port 1 init
    P1DIR |= LED1 + LED2;
    P1OUT = 0x00;

    initUart();

    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0, interrupts enabled

    //while (1) {
    	//sendUart('a');
    //}

	//return 0;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	switch(UCA0RXBUF) {
	case 'e':
		P1OUT |= LED1;
		P1OUT |= LED2;
		break;
	case 'd':
		P1OUT &= ~LED1;
		P1OUT &= ~LED2;
		break;
	default:
	    sendUart(UCA0RXBUF);
	}
}
