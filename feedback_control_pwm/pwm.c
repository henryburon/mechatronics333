#include "nu32dip.h"          // constants, functions for startup and UART


#define NUMSAMPS 1000  // number of points in waveform
static volatile int Waveform[NUMSAMPS];  // waveform

// Function prototypes
void makeWaveform();

// Interrupts
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void) { // call with Timer2 @ 1 kHz
  static int counter = 0;  // initialize counter once

  // set OC1RS (the duty cycle)
  OC1RS = Waveform[counter];

  counter++;  // add one to counter every timer ISR is entered
  if (counter == NUMSAMPS) {
    counter = 0;  //  roll the counter over when needed
  }

  // clear interrupt flag so it can be triggered again
  IFS0bits.T2IF = 0; // clear flag on T2 interrupt

}


int main(void) {

  NU32DIP_Startup();          // cache on, interrupts on, LED/button init, UART init

  // Enable Timer3 with a frequency of 20 kHz, and set up OC1
  T3CONbits.TCKPS = 0b000;  // set prescaler (N) on Timer3 to N=1 (1:1). Prescaler divides the incoming clock before incrementing the timer
  PR3 = 2399;               // period = (PR3 + 1) * N * (1/48,000,000) = 0.0005s, 50 us, 20 kHz
  TMR3 = 0;                 // initial timer3 count is 0
  OC1CONbits.OCM = 0b110;   // PWM mode on OC1, fault pin disabled
  OC1CONbits.OCTSEL = 1;   // timer3 is the clock source for this Output Compare module
  OC1RS = 600;             // duty cycle = OC1RS/(PR3 + 1) = 25%. Initializes duty cycle (and LED brightness) at 25% of max.
  OC1R = 600;              // initialize before turning OC1 on; afterward it is read only

  makeWaveform();

  T3CONbits.ON = 1;        // turn on Timer3
  RPB15Rbits.RPB15R = 0b0101;  // map OC1 to RPB15, pin 26 on board. 0b0101 designates the mapping as OC1
  OC1CONbits.ON = 1;       // turn on OC1

  // Enable Timer2 with frequency of 1 kHz
  T2CONbits.TCKPS = 0b000; // set prescaler (N) on Timer2 to N=1
  PR2 = 47999; // We want 1 kHz. So period = 0.001. (PR2 + 1) * N * (1/48,000,000) = 0.001. Solve for PR3.
  TMR2 = 0;  // initial timer2 count is 0
  T2CONbits.ON = 1;  // turn on Timer2

  // enable interrupts
  __builtin_disable_interrupts(); // disable interrupts
  IPC2bits.T2IP = 5; // set timer2 interrupt priority to 5
  IPC2bits.T2IS = 0; // set timer2 sub priority to 0, default
  IEC0bits.T2IE = 1; // enable the timer2 interrupt
  IFS0bits.T2IF = 0; // clear the timer2 interrupt flag
  __builtin_enable_interrupts(); // re-enable interrupts

  // continue forever
  while(1) {
    ;
    }

  return 0;
}

// Functions

void makeWaveform() {
  int i = 0, center = (PR3+1)/2, A = (PR3+1)/4; // square wave, center should be 50% of PR3, A should be 25% so it alternates between 25% and 75%
  for (i = 0; i < NUMSAMPS; ++i) {
    if ( i < NUMSAMPS/2) {
      Waveform[i] = center + A;
    } else {
      Waveform[i] = center - A;
    }
  }   
}