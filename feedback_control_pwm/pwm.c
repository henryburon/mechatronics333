#include "nu32dip.h"          // constants, functions for startup and UART


#define NUMSAMPS 1000  // number of points in waveform
#define PLOTPTS 200  // number of data points to plot
#define DECIMATION 10  // plot every 10th point

static volatile int Waveform[NUMSAMPS];  // waveform
static volatile int ADCarray[PLOTPTS];  // measured values to plot
static volatile int REFarray[PLOTPTS];  // reference values to plot
static volatile int StoringData = 0;    // if this flag = 1, currently storing plot data
static volatile int Eint = 0; 

// Controller gains

static volatile float Kp = 0.1;
static volatile float Ki = 0.08;

// Function prototypes
void makeWaveform();
unsigned int adc_sample_convert(int pin);

// Interrupts
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void) { // call with Timer2 @ 1 kHz
  static int counter = 0;  // initialize counter once
  static int plotind = 0;  // index for data arrays: counts up to PLOTPTS
  static int decctr = 0;   // counts to store data once every DECIMATION
  static int adcval = 0;   // ADC value
  static int error = 0; // error
  float u = 0; // control input
  float unew = 0; // new control input

  // set OC1RS (the duty cycle)
  OC1RS = Waveform[counter];

  // read the ADC value
  adcval = adc_sample_convert(0);  // sample and convert pin 0

  if (StoringData) {
    decctr++;
    if (decctr == DECIMATION) {   // after DECIMATION control loops,
      decctr = 0;                // reset the decimation counter
      ADCarray[plotind] = adcval;  // store data in global arrays
      REFarray[plotind] = Waveform[counter];
      plotind++;                   // increment the plot index
    }
    if (plotind == PLOTPTS) {      // if max number of plot points plot is reached
      plotind = 0;                // reset the plot index
      StoringData = 0;             // tell main that data is ready to be sent
    }
  }

  counter++;  // add one to counter every timer ISR is entered
  if (counter == NUMSAMPS) {
    counter = 0;  //  roll the counter over when needed
  }
  // clear interrupt flag so it can be triggered again
  IFS0bits.T2IF = 0; // clear flag on T2 interrupt

  error = Waveform[counter] - adcval; // reference minus sensed
  Eint = Eint + error; // update integral error
  u = Kp*error + Ki*Eint; // control input

  unew = u + 50.0;
  if (unew > 100.0) {
    unew = 100.0;
  } else if (unew < 0.0) {
    unew = 0.0;
  }

  // set OC1RS (the duty cycle)
  OC1RS = (unsigned int) ((unew/100.0) * PR3);




}


int main(void) {

  NU32DIP_Startup();          // cache on, interrupts on, LED/button init, UART init

  char message[100];  // message to and from Python
  int i = 0; // plot data loop counter

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

  // Enable/Set up ADC
  AD1CON1bits.ON = 1; // turn on A/D converter. Different than book code. Seems that this version of PIC32 does not need as much setup.

  // continue forever
  while(1) {
    
    StoringData = 1; // message to ISR to start storing data

    while (StoringData) {
      ; // wait until ISR says data storing is done. The ISR will change StoringData to 0 when it is done storing data
    }

    for (i=0; i<PLOTPTS; i++) {

      sprintf(message, "%d %d %d\r\n", PLOTPTS-i, ADCarray[i], REFarray[i]);
      NU32DIP_WriteUART1(message);
    }




    }
  return 0;
}

// Functions

void makeWaveform() {
  // int i = 0, center = (PR3+1)/2, A = (PR3+1)/4; // square wave, center should be 50% of PR3, A should be 25% so it alternates between 25% and 75%
  int i = 0;
  int center = 500;
  int A = 300;
  for (i = 0; i < NUMSAMPS; ++i) {
    if ( i < NUMSAMPS/2) {
      Waveform[i] = center + A;
    } else {
      Waveform[i] = center - A;
    }
  }   
}

unsigned int adc_sample_convert(int pin) { // pin refers to ANx pin... i.e. AN0 is 0, AN1 is 1, etc.
  unsigned int elapsed = 0;
  unsigned int finish_time = 0;
  AD1CHSbits.CH0SA = pin;  // connect chosen pin to MUXA for sampling
  AD1CON1bits.SAMP = 1;  // START sampling

  elapsed = _CP0_GET_COUNT();
  finish_time = elapsed + 10;
  while (_CP0_GET_COUNT() < finish_time) { ; }  // sample for more than 250 ns

  AD1CON1bits.SAMP = 0;  // STOP sampling and START converting

  while (!AD1CON1bits.DONE) { ; }  // wait for the conversion process to finish

  return ADC1BUF0;  // read the buffer with the result
}