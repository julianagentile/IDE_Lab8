// CortexM.c
// Cortex M registers and basic functions used in these labs
// Daniel and Jonathan Valvano
// December 12, 2018
#include <stdint.h>

//******DisableInterrupts************
// sets the I bit in the PRIMASK to disable interrupts
// Inputs: none
// Outputs: none
  __asm void
  __weak DisableInterrupts(void)
  {
    CPSID   I
    bx      lr
  }


//******EnableInterrupts************
// clears the I bit in the PRIMASK to enable interrupts
// Inputs: none
// Outputs: none
  __asm void
  __weak EnableInterrupts(void)
  {
    CPSIE   I
    bx      lr
  }


//******StartCritical************
// StartCritical saves a copy of PRIMASK and disables interrupts
// Code between StartCritical and EndCritical is run atomically
// Inputs: none
// Outputs: copy of the PRIMASK (I bit) before StartCritical called
  __asm long
  __weak StartCritical(void)
  {
    MRS     R0, PRIMASK
    CPSID   I
    bx      lr
  }


//******EndCritical************
// EndCritical sets PRIMASK with value passed in
// Code between StartCritical and EndCritical is run atomically
// Inputs: PRIMASK (I bit) before StartCritical called
// Outputs: none
  __asm void
  __weak EndCritical(long sr)
  {
    MSR     PRIMASK, R0
    bx      lr
  }


//******WaitForInterrupt************
// enters low power sleep mode waiting for interrupt (WFI instruction)
// processor sleeps until next hardware interrupt
// returns after ISR has been run
// Inputs: none
// Outputs: none
  __asm void
  __weak WaitForInterrupt(void)
  {
    WFI
    bx      lr
  }
	

// delay function
// which delays about 6*ulCount cycles
// ulCount=8000 => 1ms = (8000 loops)*(6 cycles/loop)*(20.83 ns/cycle)
  __asm void
  delay(unsigned long ulCount)
  {
    subs    r0, #1
    bne     delay
    bx      lr
  }
// ------------Clock_Delay1ms------------
// Simple delay function which delays about n milliseconds.
// Inputs: n, number of msec to wait
// Outputs: none
void Clock_Delay1ms(uint32_t n)
{
  while(n)
	{
    delay(8000);   // 1 msec, tuned at 48 MHz
    n--;
  }
}