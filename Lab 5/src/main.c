/*  -------------------------------------------
    Lab 5: Advanced Viva
		
------------------------------------------- */
#include <MKL25Z4.H>
#include <stdbool.h>
#include <stdint.h>

#include "..\include\gpio_defs.h"
#include "..\include\adc_defs.h"
#include "..\include\SysTick.h"


/*----------------------------------------------------------------------------
    Task 1: checkButton

  This tasks polls the button and signals when it has been pressed
*----------------------------------------------------------------------------*/
int buttonState ; // current state of the button
int bounceCounter ; // counter for debounce
bool pressed ; // signal if button pressed

void init_ButtonState() {
    buttonState = BUTTONUP;
    pressed = false;
}

void task1ButtonPress() {
    if (bounceCounter > 0) bounceCounter-- ;
    switch (buttonState) {
        case BUTTONUP:
            if (isPressed()) {
                buttonState = BUTTONDOWN ;
                pressed = true ; 
            }
          break ;
        case BUTTONDOWN:
            if (!isPressed()) {
                buttonState = BUTTONBOUNCE ;
                bounceCounter = BOUNCEDELAY ;
            }
            break ;
        case BUTTONBOUNCE:
            if (isPressed()) {
                buttonState = BUTTONDOWN ;
            }
            else if (bounceCounter == 0) {
                buttonState = BUTTONUP ;
            }
            break ;
    }                
}

/*  -----------------------------------------
     Task 2: MeasureVoltage and Flash Blue LED
        this task first measures 2 voltages using them as the minimum and the maximum.
				if the minimuk voltae is bwlow the maximum then the blue LEd flashes with an on off period proportional to the value of the potenitomenter
        if the mimimum voltage is above then maximum then there is an error state that flashed the Blue LED on and off with 0.5s interval
    -----------------------------------------   */
#define MINIMUM 1 
#define MAXIMUM 2 
#define DONE 3

#define BLUEON 12 // can these be different?
#define BLUEOFF 13
#define ERRORON 14
#define ERROROFF 15

// declare volatile to ensure changes seen in debugger
volatile float measured_voltage ;  // scaled value
volatile float diff_measured_voltage ;  // scaled value
volatile float minVoltage;
volatile float maxVoltage;
int measureState ;
int blueLEDcount = PERIOD;
int onTime;


void Init_MeasureState(void) {
    measureState = MINIMUM ;
    greenLEDOnOff(LED_OFF) ;
    redLEDOnOff(LED_ON) ;  
}

void task2MeasureVoltageAndFlash(void) {   
    MeasureVoltage() ;    // updates sres variable
    measured_voltage = (VREF * sres) / ADCRANGE ;  
    onTime = ((measured_voltage - minVoltage) / (maxVoltage - minVoltage)) * 400;
    if (blueLEDcount > 0) blueLEDcount -- ;
    switch (measureState) {
        case MINIMUM:
            if (pressed) {
                pressed = false ;     // acknowledge event        
                // take a simple-ended voltage reading
                MeasureVoltage() ;    // updates sres variable
                // scale to an actual voltage, assuming VREF accurate
                measured_voltage = (VREF * sres) / ADCRANGE ;                
                measureState = MAXIMUM ;
                minVoltage = measured_voltage;
                greenLEDOnOff(LED_ON) ;
                redLEDOnOff(LED_OFF) ;     
            }
            break ;
        case MAXIMUM:
            if (pressed) {
                pressed = false ;     // acknowledge event

                // take a simple-ended voltage reading
                MeasureVoltage() ;    // updates sres variable
                // scale to an actual voltage, assuming VREF accurate
                measured_voltage = (VREF * sres) / ADCRANGE ;
                maxVoltage = measured_voltage;
                greenLEDOnOff(LED_OFF) ;
                if (minVoltage < maxVoltage) {
                    blueLEDcount = PERIOD;
                    measureState = BLUEON;
                } else {
                    blueLEDOnOff(LED_ON);
                    measureState = ERRORON;
                }                        
            }
            break ;
                      
        case BLUEON:
            if (blueLEDcount == 0) { // The time transition has priority                    
                blueLEDOnOff(LED_OFF) ;
                measureState = BLUEOFF ;
                blueLEDcount = onTime; //400 - onTime ;
            }
            break ;
                                
        case BLUEOFF:
            if (blueLEDcount == 0) { // The time transition has priority
                blueLEDOnOff(LED_ON) ;
                measureState = BLUEON ;
                blueLEDcount = 400 - onTime; //onTime ;
            }
            break ;
       
				case ERRORON:
            if (blueLEDcount == 0) { // The time transition has priority
                blueLEDOnOff(LED_OFF) ;
                measureState = ERROROFF ;
                blueLEDcount = 50; //400 - onTime ;
            }
            break ;
                
        case ERROROFF:
            if (blueLEDcount == 0) { // The time transition has priority
                blueLEDOnOff(LED_ON) ;
                measureState = ERRORON ;
                blueLEDcount = 50; //onTime ;
            }
            break ;                                          
    }
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
volatile uint8_t calibrationFailed ; // zero expected
int main (void) {
    // Enable clock to ports B, D and E
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK ;

    init_LED() ; // initialise LED
    init_ButtonGPIO() ; // initialise GPIO input
    init_ButtonState() ; // initialise button state variables
    Init_ADC() ; // Initialise ADC
    calibrationFailed = ADC_Cal(ADC0) ; // calibrate the ADC 
    while (calibrationFailed) ; // block progress if calibration failed
    Init_ADC() ; // Reinitialise ADC
    Init_MeasureState() ;  // Initialise measure state 
    Init_SysTick(1000) ; // initialse SysTick every 1ms    
    waitSysTickCounter(10) ;
    
    while (1) {        
        task1ButtonPress() ;
        task2MeasureVoltageAndFlash() ;
        // delay
      waitSysTickCounter(10) ;  // cycle every 10 ms
    }
}
