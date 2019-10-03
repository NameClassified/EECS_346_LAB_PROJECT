#include <msp430.h>
#include <CTS_Layer.h>
#include <msprf24.h>
#include <nrf_userconfig.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * main.c
 */
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // disable low-power impedance mode

    // variable initialization
    int snap[3] = {0,0,0}; // snap progress array - tracks progress of snap

    // initialize reset button
    P5DIR &= ~BIT5; // pin 5.5 is an input
    P5REN |= BIT5; // enable hardware resistor for pin 5.5
    P5OUT |= BIT5; // set pull up (logic HIGH until a press)


    // initialize transmission status LED
	P1DIR |= BIT1; // pin 1.1 is an output
    P1OUT &= ~BIT1; // LED starts in the OFF position

	// Clock setup
	CSCTL0_H = 0xA5;                          // Unlock CS to modify clock settings
	CSCTL1 = DCORSEL + DCOFSEL_4;             // Set fDCOCLK to 16MHz
	CSCTL2 = SELA__VLOCLK + SELS_3 + SELM_3;  // set all clocks to DCOCLK
	CSCTL0_H = 0;                             // Lock CS registers to use the clock as normal

	// LED PIN INITIALIZATION (STONES)
	P8DIR |= BIT1 | BIT2 | BIT3; // pins 8.1, 8.2, 8.3 are outputs
	P5DIR |= BIT3 | BIT7; // pins 5.3, 5.7 are also outputs
	P4DIR |= BIT4; // as is pin 4.4

	//radio transmission variables
    uint8_t addr[5]; // holds address of tx/rx radios
    uint8_t buf[32]; // holds the stuff that gets sent

    /* Initial values for nRF24L01+ library config variables */
    rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width = 5; // specify width of radio address (5 bytes wide to avoid interference)
    rf_speed_power = RF24_SPEED_1MBPS | RF24_POWER_0DBM; //set radio transmission speed
    rf_channel = 120; // specify RF channel

    msprf24_init();  // initialize radio with empty pipes
    msprf24_set_pipe_packetsize(0, 32); // set size of transmission packet
    msprf24_open_pipe(0, 0);  // open data pipe 0, allow auto-ack


    msprf24_standby(); // put radio to sleep while not transmitting

    // specify address of transmitter/receiver radios
    addr[0] = 0xCA;
    addr[1] = 0xFE;
    addr[2] = 0xF0;
    addr[3] = 0x0D;
    addr[4] = 0x01;
    w_tx_addr(addr); // specify address of transmitting radio
    w_rx_addr(0, addr);  // specify that the receiver with address addr will be listening on pipe #0

	TI_CAPT_Init_Baseline(&slider_1); // define the baseline/control measurement for the CapTouch pad
	TI_CAPT_Update_Baseline(&slider_1,5); // take 5 measurements, average them, define that as new baseline

	  while (1) {
        w_tx_payload(32, buf); // load buf[0] into the next radio payload
        msprf24_activate_tx(); // send radio payload to the receiver

	    //check CapTouch slider sections and update snap completion
	    if(TI_CAPT_Buttons(&slider_1) == (slider_1.arrayPtr[0])) {snap[2] = 1;} // if first slider section is pressed, progress snap
	    else if(TI_CAPT_Buttons(&slider_1) == (slider_1.arrayPtr[1])) {snap[1] = 1;} // if second slider section is pressed, progress snap
	    else if(TI_CAPT_Buttons(&slider_1) == (slider_1.arrayPtr[2])) {snap[0] = 1;} // if third slider section is pressed, progress snap

	    // check for snap
	    if (snap[0] && snap[1] && snap[2]) { // if all snap sections are complete, turn on LED's and kill spider-man
	        buf[0] = 69; // send 69 (snap signal) to radio over SPI, killing the spider-man
	        // turn on LED's
            P1OUT |= BIT1; // snap status LED
            P8OUT |= BIT1 | BIT2 | BIT3; // stone LEDs
            P5OUT |= BIT3 | BIT7;
            P4OUT |= BIT4;
            // PERFECTLY BALANCED, AS ALL THINGS SHOULD BE
	    }
	    else { // if the snap is not complete, keep LED's off and transmit a 'no snap' signal
	        buf[0] = 42; // send 'idle' signal
	        // turn off LED's
            P1OUT &= ~BIT1; // turn off the 'ongoing snap' LED
            P8OUT &= ~(BIT1 | BIT2 | BIT3); // turn off the stone LEDs
            P5OUT &= ~(BIT3 | BIT7);
            P4OUT &= ~(BIT4);
	    }

	    // RESET
	    // YOU COULD NOT LIVE WITH YOUR CODE'S FAILURE, AND WHERE DID THAT BRING YOU?
	    // THIS IF STATEMENT RIGHT HERE
	    if (!(P5IN & BIT5)) { // reset system on button press
	        buf[0] = 42; // send 'idle' signal
	        // turn off LED's
            P1OUT &= ~BIT1; // no ongoing snap
            P8OUT &= ~(BIT1 | BIT2 | BIT3); // i used the bitmask to destroy the stones
            P5OUT &= ~(BIT3 | BIT7); // as before, turn off the stone LEDs when a reset occurs
            P4OUT &= ~(BIT4);
            // reset the snap progress array
            snap[0] = 0;
            snap[1] = 0;
            snap[2] = 0;
	    }
	  }
}

/******************************************************************************/
// Timer0_A0 Interrupt Service  Routine: Disables the timer and exists LPM3
/******************************************************************************/

// CapTouch timer interrupt handling
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ISR_Timer0_A0(void) {
  TA0CTL &= ~(MC_1); // halt Timer_A0
  TA0CCTL0 &= ~(CCIE); // disable Timer_A0 interrupt flag
  __bic_SR_register_on_exit(LPM3_bits+GIE); //exit interrupt
}

