/*
 * Rochester Institute of Technology
 * Department of Computer Engineering
 * CMPE 460 Interfacing Digital Electronics
 * LJBeato
 * 1/14/2021
 *
 * Modified for heartbeat detection using MSP432
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "msp.h"
#include "uart.h"
#include "Timer32.h"
#include "CortexM.h"
#include "Common.h"
#include "ADC14.h"

#define ADC_MAX 16384.0     // 14-bit ADC resolution
#define ADC_THRESHOLD 9830  // Approx 60% of full scale
#define VREF 2.5            // ADC reference voltage in Volts

#define MIN_HEARTBEAT_INTERVAL_MS 200  // Minimum time between peaks (in ms)

volatile uint32_t adc_sample = 0;
volatile bool new_adc_data = false;
unsigned long MillisecondCounter = 0;

typedef enum {
    BELOW_THRESHOLD,
    ABOVE_THRESHOLD
} PeakState;

unsigned long last_beat_time = 0;
unsigned long current_beat_time = 0;
double heartrate_bpm = 0;
int beat_count = 0;

// ISR for 1ms Timer32 interrupt
void Timer32_1_ISR(void)
{
    adc_sample = ADC_In();         // Read ADC from channel A6 (P4.7)
    new_adc_data = true;           // Signal new data
    MillisecondCounter++;          // Track time in milliseconds
}

// ISR for button on P1.1 (toggle sampling)
void PORT1_IRQHandler(void)
{
    if (P1->IFG & BIT1) {
        P1->IFG &= ~BIT1;
    }
}

int main(void)
{
    char output[64];
    uart0_init();
    uart0_put("\r\nHeartbeat Detection (MSP432)\r\n");

    ADC0_InitSWTriggerCh6();  // Initialize ADC (P4.7)
    Timer32_1_Init(&Timer32_1_ISR, SystemCoreClock / 1000, T32DIV1);  // 1ms ticks

    EnableInterrupts();

    int prev = 0, curr = 0, next = 0;
    PeakState state = BELOW_THRESHOLD;

    while (1) {
        if (new_adc_data) {
            new_adc_data = false;

            // Shift ADC samples
            next = curr;
            curr = prev;
            prev = adc_sample;

            // Detect peak using a state machine
            if (state == BELOW_THRESHOLD) {
                if (curr > ADC_THRESHOLD && curr >= prev && curr >= next) {
                    current_beat_time = MillisecondCounter;

                    if ((current_beat_time - last_beat_time) > MIN_HEARTBEAT_INTERVAL_MS) {
                        double interval = (current_beat_time - last_beat_time) / 1000.0;
                        heartrate_bpm = 60.0 / interval;
                        beat_count++;

                        sprintf(output, "\r\nBeat #%d at %lu ms", beat_count, current_beat_time);
                        uart0_put(output);
                        sprintf(output, "\r\nHeart Rate: %.2f BPM", heartrate_bpm);
                        uart0_put(output);

                        last_beat_time = current_beat_time;
                        state = ABOVE_THRESHOLD;  // Wait for drop
                    }
                }
            } else if (curr < ADC_THRESHOLD) {
                state = BELOW_THRESHOLD;
            }
        }
    }
}
