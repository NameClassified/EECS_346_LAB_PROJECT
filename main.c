//#include <msp430.h>
#include "CTS_Layer.h"

/**
 * main.c
 */
#define DELAY 2000

#ifdef ELEMENT_CHARACTERIZATION_MODE
// Delta Counts returned from the API function for the sensor during characterization
unsigned int dCnt;
#endif

unsigned char tempflag;

void sleep(unsigned int time)
{
    TA0CCR0 = time;
    TA0CTL = TASSEL__ACLK + MC__UP + TACLR;
    TA0CCTL0 &= ~CCIFG;
    TA0CCTL0 = CCIE;
    __bis_SR_register(LPM3_bits+GIE);
    __no_operation();

}
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;
	
	// Clock setup
	CSCTL0_H = 0xA5;                          // Unlock CS
	CSCTL1 = DCORSEL + DCOFSEL_5;             // Set DCO = 20MHz, use this clk when fRO
	CSCTL2 = SELA__VLOCLK + SELS_3 + SELM_3;  // set SMCLK = MCLK = DCO  LFXT1 = VLO
	CSCTL0_H = 0;                             // Lock CS registers

	P8DIR |= BIT1 | BIT2 | BIT3;

	// Initialize Baseline measurement
	//TI_CAPT_Init_Baseline(&slider_5);
	//TI_CAPT_Init_Baseline(&slider_3);
	TI_CAPT_Init_Baseline(&slider_1);

	// Update baseline measurement (Average 5 measurements)
	//TI_CAPT_Update_Baseline(&slider_5,5);
	//TI_CAPT_Update_Baseline(&slider_3,5);
	TI_CAPT_Update_Baseline(&slider_1,5);

	  while (1)
	  {

	    #ifdef ELEMENT_CHARACTERIZATION_MODE
	    // Get the raw delta counts for element characterization
	    //TI_CAPT_Custom(&slider_5,&dCnt);
	    //TI_CAPT_CUSTOM(&slider_3,&dCnt);
	    TI_CAPT_CUSTOM(&slider_1,&dCnt);
	    __no_operation();                   // Set breakpoint here
	    #endif


	    #ifndef ELEMENT_CHARACTERIZATION_MODE
	    // Check if the middle element sensor has been triggered. The API call
	    // compares the value from the sensor against the threshold to determine
	    // trigger condition

	    if(TI_CAPT_Buttons(&slider_1) == (slider_1.arrayPtr[0]))
	    {
	        // Do something
	        P8OUT |= BIT1; // turn on LEDs
	    }
	    else if(TI_CAPT_Buttons(&slider_1) == (slider_1.arrayPtr[1])) {
	        P8OUT |= BIT2;
	    }
	    else if(TI_CAPT_Buttons(&slider_1) == (slider_1.arrayPtr[2])) {
	        P8OUT |= BIT3;
	    }
	    else
	    {
	        P8OUT &= ~BIT1; // TURN OFF LEDS
	        P8OUT &= ~BIT2;
	        P8OUT &= ~BIT3;
	    }

	    // Put the MSP430 into LPM3 for a certain DELAY period
	    //sleep(DELAY);
	    #endif
	  }
	return 0;
}
/******************************************************************************/
// Timer0_A0 Interrupt Service  Routine: Disables the timer and exists LPM3
/******************************************************************************/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ISR_Timer0_A0(void)
{
  TA0CTL &= ~(MC_1);
  TA0CCTL0 &= ~(CCIE);
  __bic_SR_register_on_exit(LPM3_bits+GIE);
}

#pragma vector=RTC_VECTOR,PORT2_VECTOR,TIMER0_A1_VECTOR,TIMER3_A1_VECTOR,     \
  USCI_A1_VECTOR,PORT1_VECTOR,TIMER1_A1_VECTOR,DMA_VECTOR,TIMER2_A1_VECTOR,   \
  USCI_B0_VECTOR,USCI_A0_VECTOR,TIMER0_B1_VECTOR,TIMER0_B0_VECTOR,            \
  UNMI_VECTOR,SYSNMI_VECTOR,AES256_VECTOR, PORT3_VECTOR,TIMER3_A0_VECTOR,     \
  PORT4_VECTOR, TIMER1_A0_VECTOR, TIMER2_A0_VECTOR,                           \
  ADC12_VECTOR /* ,COMP_E_VECTOR */
__interrupt void ISR_trap(void)
{
  // the following will cause an access violation which results in a SW BOR
    PMMCTL0 = PMMPW | PMMSWBOR;
}
