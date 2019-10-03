/* msp430_spi.c
 * Library for performing SPI I/O on a wide range of MSP430 chips.
 *
 * Serial interfaces supported:
 * 1. USI - developed on MSP430G2231
 * 2. USCI_A - developed on MSP430G2553
 * 3. USCI_B - developed on MSP430G2553
 * 4. USCI_A F5xxx - developed on MSP430F5172, added F5529
 * 5. USCI_B F5xxx - developed on MSP430F5172, added F5529
 *
 * Copyright (c) 2013, Eric Brundick <spirilis@linux.com>
 * 
 * Updated by Josiah Hester <josiah@northwestern.edu>
 * 1. eUSCI_B - for MSP430FR5994 Launchpad, UCB1 SPI on P5.x
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <msp430.h>
#include "msp430_spi.h"
#include "nrf_userconfig.h"


// For the MSP430FR5994 launchpad
#if defined(__MSP430FR5994__)
void spi_init()
{
	/* Configure ports on MSP430FR5994 device for USCI_B1 */
	//P5SEL0 |= (BIT0 | BIT1 | BIT2 | BIT3);
	//P5SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    // Configure ports on MSP430FR5994 device for USCI_A3
    P6SEL0 |= (BIT0 | BIT1 | BIT2 | BIT3);
    P6SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3);
	/* USCI_A3 specific SPI setup */
	UCA3CTLW0 |= UCSWRST;
	UCA3CTLW0 = UCCKPH | UCMST | UCMSB | UCSYNC | UCSSEL_2 | UCSWRST;
	UCA3BRW = 0x01;
	UCA3CTLW0 &= ~UCSWRST;
}

uint8_t spi_transfer(uint8_t inb)
{
	UCA3TXBUF = inb;
	while ( !(UCA3IFG & UCRXIFG) )  // Wait for RXIFG indicating remote byte received via SOMI
		;
	return UCA3RXBUF;
}

uint16_t spi_transfer16(uint16_t inw)
{
	uint16_t retw;
	uint8_t *retw8 = (uint8_t *)&retw, *inw8 = (uint8_t *)&inw;

	UCA3TXBUF = inw8[1];
	while ( !(UCA3IFG & UCRXIFG) )
		;
	retw8[1] = UCA3RXBUF;
	UCA3TXBUF = inw8[0];
	while ( !(UCA3IFG & UCRXIFG) )
		;
	retw8[0] = UCA3RXBUF;
	return retw;
}
#endif
