#include <MKL25Z4.H>
#include <stdbool.h>
#include "SysTick.h"
#include "gpio.h"


/* ------------------------------------------
       ECS642 Lab for week 2 2018
   Demonstration of simple digital input
   Use of RGB LED on Freedom board
  -------------------------------------------- */
        

/*----------------------------------------------------------------------------
  Turn LEDs on or off 
    onOff can be ON or OFF
*----------------------------------------------------------------------------*/
void setRedLED(int onOff) {
    if (onOff == ON) {
        PTB->PCOR = MASK(RED_LED_POS) ;               
    } 
    if (onOff == OFF) {
        PTB->PSOR =  MASK(RED_LED_POS) ;
    }
    // no change otherwise
}

void setBlueLED(int onOff) {
    if (onOff == ON) {
        PTD->PCOR = MASK(BLUE_LED_POS) ;              
    } 
    if (onOff == OFF) {
        PTD->PSOR = MASK(BLUE_LED_POS) ;
    }
    // no change otherwise
}



//@E added
void setExternalLED(int onOff) {
    if (onOff == ON) {
        PTD->PCOR = MASK(EXTERNAL_LED_POS) ;              
    } 
    if (onOff == OFF) {
        PTD->PSOR = MASK(EXTERNAL_LED_POS) ;
    }
}




/*----------------------------------------------------------------------------
  isPressed: test the switch

  Operating the switch connects the input to ground. A non-zero value
  shows the switch is not pressed.
 *----------------------------------------------------------------------------*/
bool isPressed(void) {
    if (PTD->PDIR & MASK(BUTTON_POS)) {
        return false ;
    }
    return true ;
}

/*----------------------------------------------------------------------------
  checkButton

This function checks whether the button has been pressed
*----------------------------------------------------------------------------*/
int buttonState ; // current state of the button
bool pressed ; // signal if button pressed

void initButton() {
    buttonState = BUTTONUP ;
    pressed = false ; 
}

void checkButton() {
    switch (buttonState) {
        case BUTTONUP:
            if (isPressed()) {
                buttonState = BUTTONDOWN ;
                pressed = true ; 
            }
            break ;
        case BUTTONDOWN:
            if (!isPressed()) {
                buttonState = BUTTONUP ;
            }
            break ;
    }                               
}

/*----------------------------------------------------------------------------
  nextFlash 

This function evaluates whether the system should change state. 
The system stays in each state for a number of cycles, counted by 
the 'count' variable. It changes state if the button is pressed.
*----------------------------------------------------------------------------*/
int state ; 
int count ;

void initFlash() {
    count = PERIOD ;
    state = REDON ;
    setRedLED(ON) ;
    setBlueLED(OFF) ;
		setExternalLED(OFF);
}

void nextFlash() {
	if (count > 0) count -- ;
	switch (state) {				 
				case REDON:
	        if (count == 0) { // The time transition has priority
                setRedLED(OFF) ;
                state = REDOFF ;
                count = PERIOD ;
	        } else if (pressed) { // The button transition can occur on next cycle 
                pressed = false ;
                setRedLED(OFF) ;
                setBlueLED(ON) ;
                state = BLUEON ;
            }
            break ;

        case REDOFF:
               if (count == 0) {
                   setRedLED(ON) ;
                   state = REDON ;
                   count = PERIOD ;
               } else if (pressed) {
                   pressed = false ;
                   state = BLUEOFF ;
               }
               break ;
                        
           case BLUEON:
               if (count == 0) {
                   setBlueLED(OFF) ;
                   state = BLUEOFF ;
                   count = PERIOD ;
               } else if (pressed) {
                   pressed = false ;
                   setBlueLED(OFF) ;
                   setExternalLED(ON) ;
									 state = EXTERNALON ; // change the external LED
               }
               break ;

           case BLUEOFF: 
               if (count == 0) {
                   setBlueLED(ON) ;
                   state = BLUEON ;
                   count = PERIOD ;
               } else if (pressed) {
                   pressed = false ;
                   state = EXTERNALOFF ; // change the external LED
               }
               break ;
							 
							 
//@E added
					case EXTERNALON:
               if (count == 0) {
                   setExternalLED(OFF) ;
                   state = EXTERNALOFF ;
                   count = PERIOD ;
               } else if (pressed) {
                   pressed = false ;
                   setExternalLED(OFF) ;
                   setRedLED(ON) ;
                   state = REDON ; // back to the red led
               }
               break ;

           case EXTERNALOFF: 
               if (count == 0) {
                   setExternalLED(ON) ;
                   state = EXTERNALON;
                   count = PERIOD ;
               } else if (pressed) {
                   pressed = false ;
                   state = REDOFF ; // back to the red LED
               }
               break ;
							 
							 
        }
}

/*----------------------------------------------------------------------------
  Configuration 
     The configuration of the GPIO port is explained in week 2
     Enabling the clocks will be covered in week 3.
     Configuring the PORTx peripheral, which controls the use of each pin, will
       be covered in week 3
*----------------------------------------------------------------------------*/
void configureOutput() {
     // Configuration steps
     //   1. Enable clock to GPIO ports
     //   2. Enable GPIO ports
     //   3. Set GPIO direction to output
     //   4. Ensure LEDs are off

     // Enable clock to ports B and D
     SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;
        
     // Make 3 pins GPIO
     PORTB->PCR[RED_LED_POS] &= ~PORT_PCR_MUX_MASK;          
     PORTB->PCR[RED_LED_POS] |= PORT_PCR_MUX(1);          
     PORTB->PCR[GREEN_LED_POS] &= ~PORT_PCR_MUX_MASK;          
     PORTB->PCR[GREEN_LED_POS] |= PORT_PCR_MUX(1);       
	
     PORTD->PCR[BLUE_LED_POS] &= ~PORT_PCR_MUX_MASK;	
     PORTD->PCR[BLUE_LED_POS] |= PORT_PCR_MUX(1);   
	
     // these lines make another pin of PortD a GPIO
     // define a suitable EXTERNAL_POS in gpio.h then uncomment	
		 PORTD->PCR[EXTERNAL_LED_POS] &= ~PORT_PCR_MUX_MASK;	
     PORTD->PCR[EXTERNAL_LED_POS] |= PORT_PCR_MUX(1);   

     // Set ports to outputs
     PTB->PDDR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
     PTD->PDDR |= MASK(BLUE_LED_POS) | MASK(EXTERNAL_LED_POS);
		 
		 //PTD->PDDR |= MASK(EXTERNAL_LED_POS); //@E external


     // Turn off LEDs     
     PTD->PSOR = MASK(BLUE_LED_POS) | MASK(EXTERNAL_LED_POS);
		 PTB->PSOR = MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
		 
		 //PTD->PSOR = MASK(EXTERNAL_LED_POS); //@E external
}

/*----------------------------------------------------------------------------
  GPIO Input Configuration

  Initialse a GPIO port D pin as an input (GPIO data direction register)
  Bit number given by BUTTON_POS
  Configure PORTD (not covered until week 3) so that the pin has no interrupt
     and a pull up resistor is enabled.
 *----------------------------------------------------------------------------*/
// 
void configureInput(void) {
    SIM->SCGC5 |=  SIM_SCGC5_PORTD_MASK; /* enable clock for port D */

    /* Select GPIO and enable pull-up resistors and no interrupts */
    PORTD->PCR[BUTTON_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | 
	           PORT_PCR_IRQC(0x0);
        
    /* Set port D switch bit to be an input */
    PTD->PDDR &= ~MASK(BUTTON_POS);

}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
    configureInput() ;  // configure the GPIO input for the button
    configureOutput() ;  // configure the GPIO outputs for the LED 
    initButton() ;
    initFlash() ;
	
		//setExternalLED(OFF);
    Init_SysTick(1000) ; // initialse SysTick every 1ms
    waitSysTickCounter(10) ; // cycle every 10ms
    while (1) {
        checkButton() ; // check button
        nextFlash() ; // flash LEDs 
        waitSysTickCounter(10) ; // wait to end of cycle
    }
}

