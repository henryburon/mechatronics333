#include "nu32dip.h"          // constants, functions for startup and UART

int main(void) {

   NU32DIP_Startup();          // cache on, interrupts on, LED/button init, UART init
   
   T3CONbits.TCKPS = 0b000;  // set prescaler (N) on Timer3 to N=1 (1:1)
   PR3 = 2399;               // period = (PR3 + 1) * N * (1/48,000,000) = 0.0005s, 50 us, 20 kHz
   TMR3 = 0;                 // initial timer3 count is 0
   OC1CONbits.OCM = 0b110;   // PWM mode on OC1, fault pin disabled
   OC1CONbits.OCTSEL = 1;   // timer3 is the clock source for this Output Compare module
   OC1RS = 600;             // duty cycle = OC1RS/(PR3 + 1) = 25%
   OC1R = 600;              // initialize before turning OC1 on; afterward it is read only

   T3CONbits.ON = 1;        // turn on Timer3

   // Need to map OC1 to a pin
   RPB15Rbits.RPB15R = 0b0101;  // map OC1 to RPB15, pin 26 on board. 0b0101 designates the mapping as OC1

   OC1CONbits.ON = 1;       // turn on OC1    



  _CP0_SET_COUNT(0);       // delay 2 seconds to see the 25% duty cycle on a 'scope
  while(_CP0_GET_COUNT() < 2 * 40000000) {
    ;
  }
  OC1RS = 1800;            // set duty cycle to 50%

  _CP0_SET_COUNT(0);       // delay 2 seconds to see the 25% duty cycle on a 'scope
  while(_CP0_GET_COUNT() < 2 * 40000000) {
    ;
  }
  OC1RS = 600;            // set duty cycle to 50%

  _CP0_SET_COUNT(0);       // delay 2 seconds to see the 25% duty cycle on a 'scope
  while(_CP0_GET_COUNT() < 2 * 40000000) {
    ;
  }
  OC1RS = 1800;            // set duty cycle to 50%
  
  while(1) {
    ;                      // infinite loop
  }
  return 0;
}

