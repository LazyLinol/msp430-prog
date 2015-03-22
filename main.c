#include <msp430g2553.h>

#include "uart.h"
#include "spi.h"
#include "device_codes.h"

#define LED1 BIT0
#define LED2 BIT6

#define AVR_RESET BIT3

#define ledOnRed() P1OUT |= LED1
#define ledOffRed() P1OUT &= ~LED1

#define ledOnGreen() P1OUT |= LED2
#define ledOffGreen() P1OUT &= ~LED2

#define avrResetUp() P1OUT |= AVR_RESET
#define avrResetDown() P1OUT &= ~AVR_RESET

#define AVR_910_OK 0x0D
#define AVR_910_NOK 0x3F

/*
 * main.c
 */
int main(void) {
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3);

  // port 1 init
  P1DIR |= LED1 + LED2 + AVR_RESET;
  P1OUT = AVR_RESET;

  initSpi();

  __delay_cycles(256);

  initUart();

  //__bis_SR_register(LPM0_bits + GIE); // Enter LPM0, interrupts enabled

  // received command
  unsigned char uart_recv;
  unsigned char spi_recv;

  // device id
  unsigned char device = 0;
  unsigned char page_mode = 0;
  unsigned char programming_mode = 0;
  unsigned char device_ok = 1;

  unsigned int address;

  unsigned char data_byte;

  unsigned char i;

  while (1) {
    while ((uart_recv = recvUart()) == 0x1B) {
    }

    switch (uart_recv) {
      // device type
      case 'T':
        device = recvUart();
        // check device against device codes
        for (i = 0; device_codes[i][0] != 0xFF; i++) {
          if (device == device_codes[i][0]) {
            page_mode = device_codes[i][1];
            device_ok = 1;
            break;
          }
          else {
            device_ok = 0;
          }

        }
        if (device_ok) {
          // found device in device list
          sendUart(AVR_910_OK);
        }
        else {
          // device not supported
          sendUart(AVR_910_NOK);
        }
        break;
      case 'S':
        // programmer id
        sendUartString("MSP AVR", 7);
        break;
      case 'V':
        // software version
        sendUart('0');
        sendUart('1');
        break;
      case 'v':
        // hardware version
        sendUart('0');
        sendUart('1');
        break;
      case 't':
        // supported devices
        for (i = 0; device_codes[i][0] != 0xFF; i++) {
          sendUart(device_codes[i][0]);
        }
        sendUart(0x00);
        break;
      case 'p':
        // programmer type
        sendUart('S');
        break;
      case 'a':
        // address auto-increment support
        sendUart('Y');
        break;
      case 'b':
        // blockmode support
        sendUart('N');
        break;
      case 'x':
        // set led
      case 'y':
        // clear led
        recvUart();
        sendUart(AVR_910_OK);
        break;
      default:
        break;
    }

    if (!device_ok) {
      sendUart(AVR_910_NOK);
      continue;
    }
    else {
      switch (uart_recv) {
        case 'P':
          // enter programming mode
          for (i = 32; i != 0xFF; i--) {
            avrResetUp();
            __delay_cycles(256);
            avrResetDown();
            __delay_cycles(256);
            readWriteSpi(0xAC);
            readWriteSpi(0x53);
            spi_recv = readWriteSpi(0x00);
            if (spi_recv == 0x53) {
              ledOnRed();
              break;
            }
            spi_recv = readWriteSpi(0x00);
          }
          sendUart(AVR_910_OK);
          break;
        case 'C':
          // write program memory hi-byte
          data_byte = recvUart();
          // here we write this byte into AVR
          readWriteSpi(0x48);
          readWriteSpi((unsigned char) (address >> 8));
          readWriteSpi((unsigned char) (address & 0xFF));
          readWriteSpi(data_byte);
          address++;
          sendUart(AVR_910_OK);
          break;
        case 'c':
          // write program memory lo-byte
          data_byte = recvUart();
          // here we write this byte into AVR
          readWriteSpi(0x40);
          readWriteSpi((unsigned char) (address >> 8));
          readWriteSpi((unsigned char) (address & 0xFF));
          readWriteSpi(data_byte);
          address++;
          sendUart(AVR_910_OK);
          break;
        case 'R':
          // read program memory
          readWriteSpi(0x28);
          readWriteSpi((unsigned char) (address >> 8));
          readWriteSpi((unsigned char) (address & 0xFF));
          sendUart(readWriteSpi(0x00));
          readWriteSpi(0x20);
          readWriteSpi((unsigned char) (address >> 8));
          readWriteSpi((unsigned char) (address & 0xFF));
          sendUart(readWriteSpi(0x00));
          address++;
          break;
        case 'A':
          // load address
          address = 256 * recvUart();
          address += recvUart();
          sendUart(AVR_910_OK);
          break;
        case 'D':
          // write data memory
          recvUart();
          sendUart(AVR_910_OK);
          break;
        case 'd':
          // read data memory
          sendUart(0x69);
          break;
        case 'L':
          // exit programming mode
          sendUart(AVR_910_OK);
          //ledOffGreen();
          ledOffRed();
          break;
        case 'e':
          // erase chip
          readWriteSpi(0xAC);
          readWriteSpi(0x80);
          readWriteSpi(0x04);
          readWriteSpi(0x00);
          __delay_cycles(256);
          avrResetUp();
          __delay_cycles(256);
          avrResetDown();
          sendUart(AVR_910_OK);
          break;
        case 'l':
          // write lock bits
          recvUart();
          sendUart(AVR_910_OK);
          break;
        case 's':
          // read signature bytes
          for (i = 0x02; i != 0xFF; i--) {
            readWriteSpi(0x30);
            readWriteSpi(0x00);
            readWriteSpi(i);
            sendUart(readWriteSpi(0x00));
          }
          break;
        case 'm':
          // write program memory page
          sendUart(AVR_910_OK);
          break;
        case ':':
          // universal command
        case '.':
          // universal command
          recvUart();
          recvUart();
          recvUart();
          if (uart_recv == ':') {
            recvUart();
          }
          sendUart(0x0F);
          sendUart(AVR_910_OK);
          break;
        default:
          break;
      }
    }
  }
  //return 0;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  /*switch(UCA0RXBUF) {
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
   }*/
}
