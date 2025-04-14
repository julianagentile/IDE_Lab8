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
#define VREF 2.5          // Reference voltage in Volts

//#define THRESHOLD_VOLTAGE 1.2  // Adjust based on your sensor output
#define MIN_HEARTBEAT_INTERVAL_MS 300  // Min time between peaks (bpm < 200)

#define MAX_BPM 	240
#define MIN_BPM		60

BOOLEAN Timer1RunningFlag = FALSE;

unsigned long MillisecondCounter = 0;

extern uint32_t SystemCoreClock;

volatile unsigned int adc_heartrate = 0;
volatile BOOLEAN new_heartbeat_data = FALSE;

double voltage;
int heartrate_bpm;

unsigned long last_beat_time = 0;
unsigned long current_time = 0;

int isPeak = 0;


// Interrupt Service Routine for Timer32-1
void Timer32_1_ISR(void)
{
    adc_heartrate = ADC_In(); // Read ADC value from channel A6 (P4.7)
    new_heartbeat_data = TRUE;
		MillisecondCounter++;
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
	char heartrate[64];
	//initializations
	uart0_init();
	uart0_put("\r\nLab8 Heartbeat Demo\r\n");

	ADC0_InitSWTriggerCh6();

	Timer32_1_Init(&Timer32_1_ISR, SystemCoreClock/2, T32DIV1); // initialize Timer A32-1;

	EnableInterrupts();
	current_time = 0;
	int adcvals[3];
	int beat_count = 0;
	int adc_peak = 0;
	
  while(1)
	{
		for (;;) {
			isPeak = 0;
			while(!isPeak){
				if(new_heartbeat_data) {
					adcvals[2] = adcvals[1];
					adcvals[1] = adcvals[0];
					adcvals[0] = adc_heartrate;
					new_heartbeat_data = FALSE;
					if (adcvals[0] < adcvals[1] && adcvals[1] > adcvals[2]) {
						isPeak = 1;
						adc_peak = adcvals[1];
						beat_count++;
					}
				}
			}		
				
			// Convert ADC value to heart rate
			voltage = (adc_peak / ADC_MAX) * VREF;	
			heartrate_bpm = (voltage / VREF) * MAX_BPM;
				
			sprintf(heartrate, "\r\nHeartrate is %u BPM", heartrate_bpm);
			uart0_put(heartrate);
		}	
	}
}
