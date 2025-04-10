/*
* Rochester Institute of Technology
* Department of Computer Engineering
* CMPE 460  Interfacing Digital Electronics
* LJBeato
* 1/14/2021
*
* Filename: main_timer_template.c
*/
#include <stdio.h>
#include <stdlib.h>

#include "msp.h"
#include "uart.h"
#include "Timer32.h"
#include "CortexM.h"
#include "Common.h"
#include "ADC14.h"
// The sprintf function seemed to cause a hange in the interrupt service routine.
// I think if we increase the HEAP size, it will work
// change to Heap_Size       EQU     0x00000200 in startup_msp432p401r_uvision.s

#define ADC_MAX 16384.0   // 14-bit ADC resolution
#define VREF 3.3          // Reference voltage in Volts

BOOLEAN Timer1RunningFlag = FALSE;

unsigned long MillisecondCounter = 0;

extern uint32_t SystemCoreClock;

volatile unsigned int adc_heartrate = 0;
volatile BOOLEAN new_heartbeat_data = FALSE;

double voltage, heartrate_bpm;


// Interrupt Service Routine for Timer32-1
void Timer32_1_ISR(void)
{
    adc_heartrate = ADC_In(); // Read ADC value from channel A6 (P4.7)
    new_heartbeat_data = TRUE;
}


void PORT1_IRQHandler(void) // main purpose is to see where the interrupt came from and we can handle it respective values. 
{
	if(P1->IFG & BIT1)
	{
        P1->IFG &= ~BIT1;
        Timer1RunningFlag = !Timer1RunningFlag;
  }
}	


// main
int main(void)
{
	char temp[64];
	unsigned int analogIn = 0;
	//initializations
	uart0_init();
	uart0_put("\r\nLab8 Heartbeat Demo\r\n");

	ADC0_InitSWTriggerCh6();

	Timer32_1_Init(&Timer32_1_ISR, SystemCoreClock/2, T32DIV1); // initialize Timer A32-1;

	EnableInterrupts();
  while(1)
	{
		for (;;) {
			if (new_heartbeat_data) {
				new_heartbeat_data = FALSE;
				
				// Convert ADC value to heart rate
				voltage = (adc_heartrate / ADC_MAX) * VREF;
				//temperature_C = (voltage * 100.0 - 50.0) / 1.8;
				
				sprintf(heartrate, "\r\nHeartrate is %u BPM", adc_heartrate);
				uart0_put(heartrate);
			}
		}
  }
}
