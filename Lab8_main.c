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
#define ADC_THRESHOLD 9830.0
#define VREF 2.5          // Reference voltage in Volts

//#define THRESHOLD_VOLTAGE 1.2  // Adjust based on your sensor output
#define MIN_HEARTBEAT_INTERVAL_MS 200  // Min time between peaks (bpm < 240ish)

#define MAX_BPM 	240
#define MIN_BPM		60

BOOLEAN Timer1RunningFlag = FALSE;

unsigned long MillisecondCounter = 0;

extern uint32_t SystemCoreClock;

volatile unsigned int adc_heartrate = 0;
volatile BOOLEAN new_heartbeat_data = FALSE;

double voltage;
double heartrate_bpm;

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

// take adc value array, make it bigger (large enough to accomodate slowest frequency).
// Sample adc and fill that array
// 
	
	EnableInterrupts();
	double last_beat_time = 0.0;
	double current_beat_time = 0.0;	
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
					//if (adcvals[0] < adcvals[1] && adcvals[1] > adcvals[2]) {
					if(adcvals[1] > ADC_THRESHOLD && adcvals[1] > adcvals[0]){
						current_beat_time = MillisecondCounter/1000.0;
            double interval = (current_beat_time - last_beat_time);

            //if (interval > (MIN_HEARTBEAT_INTERVAL_MS)/1000.0) {
							heartrate_bpm = (double)(MAX_BPM) / (double)(interval*2000.0);
							//heartrate_bpm = 60.0 / ((double)(interval*1000.0));
						sprintf(heartrate, "\r\nLast Time %f", last_beat_time);
						uart0_put(heartrate);
							last_beat_time = current_beat_time;
							isPeak = 1;
							adc_peak = adcvals[1];
							beat_count++;
						//}
							
						sprintf(heartrate, "\r\nCurrent Time %f", current_beat_time);
						uart0_put(heartrate);
						
							sprintf(heartrate, "\r\nBeat #%d", beat_count);
							uart0_put(heartrate);

							sprintf(heartrate, "\r\nInterval: %f ms", interval);
							uart0_put(heartrate);

							sprintf(heartrate, "\r\nHeartrate is %u BPM\n", (int)(heartrate_bpm));
							uart0_put(heartrate);
					}
				}
			}		
				
			// Convert ADC value to heart rate
			
			//voltage = (adc_peak / ADC_MAX) * VREF;			
			//heartrate_bpm = (voltage / VREF) * MAX_BPM;
			
			//if (MillisecondCounter * 60000 >= 1) {
			
			//sprintf(heartrate, "\r\nMillisecond counter: %lu", MillisecondCounter);
			//uart0_put(heartrate);
				
			//sprintf(heartrate, "\r\nBeat count: %u", beat_count);
			//uart0_put(heartrate);
			
			//heartrate_bpm = (MillisecondCounter/beat_count) * 30;
			//heartrate_bpm = ((double)(beat_count * 60000)) / ((double)(MillisecondCounter));

			
			//sprintf(heartrate, "\r\n ADC Peak: %u", adc_peak);
				//uart0_put(heartrate);
	
				//sprintf(heartrate, "\r\nHeartrate is %u BPM", (int)(heartrate_bpm));
				//uart0_put(heartrate);
			//}
		}	
	}
}
