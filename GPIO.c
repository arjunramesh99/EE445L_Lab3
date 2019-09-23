#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "GPIO.h"

// PF4 is input
// Make PF2 an output, enable digital I/O, ensure alt. functions off
void PortF_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_PCTL_R &= ~0x000F0F0F; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x15;      // 4) disable analog function on PF2, PF4
  GPIO_PORTF_PUR_R |= 0x11;         // 5) pullup for PF4
  GPIO_PORTF_DIR_R |= 0x04;         // 5) set PF2 direction to output
	GPIO_PORTF_DIR_R &= ~(0x11);			// PF4 input
  GPIO_PORTF_AFSEL_R &= ~0x15;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x15;         // 7) enable digital port
}

void PortB_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x02;        // 1) activate clock for Port B
  while((SYSCTL_PRGPIO_R&0x02)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTB_PCTL_R &= ~0x000F00FF; // 3) regular GPIO
  GPIO_PORTB_AMSEL_R &= ~0x13;      // 4) disable analog function on PF2, PF4
  GPIO_PORTB_PUR_R |= 0x13;         // 5) pullup for PF4
  GPIO_PORTB_DIR_R &= ~(0x13);         // 5) set direction to output
  GPIO_PORTB_AFSEL_R &= ~0x13;      // 6) regular port function
  GPIO_PORTB_DEN_R |= 0x13;         // 7) enable digital port
}


void GPIO_Init(void) {
	PortF_Init();
	PortB_Init();
}	
