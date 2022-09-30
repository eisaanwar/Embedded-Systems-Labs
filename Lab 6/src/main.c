/*----------------------------------------------------------------------------
    Code for Lab 6
    In this project the PIT and the TPM are used to create a tone. 
    Buttons can be pushed to adjust the frequency and volume of the tones
 *---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include <MKL25Z4.H>
#include "../include/gpio.h"
#include "../include/pit.h"
#include "../include/tpmPwm.h"

osEventFlagsId_t evtFlagsTone; // event flags
osEventFlagsId_t evtFlagsVolume; // event flags
   // Flag PRESS_EVT is set when the button is pressed

/*--------------------------------------------------------------
 *     Tone task - iterate though different tones
 *--------------------------------------------------------------*/

osThreadId_t t_tone;        /*  task id of task to flash led */

void toneTask (void *arg) { 
    int nextTone = 0;
    const uint32_t tones [12] = {20040, 18915 ,17853, 16851 ,15905, 15013, 14170, 13375, 12624, 11916, 11247, 10616};
    while (1) {
        osEventFlagsWait (evtFlagsTone, MASK(PRESS_EVT), osFlagsWaitAny, osWaitForever);        
        setTimer(0, tones[nextTone]);  // Frequency for MIDI tones
        startTimer(0);            
        nextTone = nextTone + 1;
            
        if (nextTone >= 11) { // if reached the end of the array then go back to the start
            nextTone = 0;
        }
    }
}

/*--------------------------------------------------------------
 *     Volume task - iterate though volumes
 *--------------------------------------------------------------*/

osThreadId_t t_volume;

void volumeTask (void *arg) {
    int nextVolume = 1; // Max is 128; off is 0 
    while (1) {
        osEventFlagsWait (evtFlagsVolume, MASK(PRESS_EVT), osFlagsWaitAny, osWaitForever);        
        setPWMDuty(nextVolume);            
        nextVolume = nextVolume*2;            
        
        if (nextVolume > 128) {
            nextVolume = 1;
        }
    }
}

/*------------------------------------------------------------
 *     Tone button task - poll tone button and send signal when pressed
 *------------------------------------------------------------*/

osThreadId_t t_buttonTone;      /* task id of task to read button for the tone */

void buttonTaskTone (void *arg) {
    int bState = UP ;
    int bCounter = 0 ;    
    while (1) {
        osDelay(10) ;
        if (bCounter) bCounter-- ;
        switch (bState) {
            case UP:
                if (isPressed()) {
                    osEventFlagsSet(evtFlagsTone, MASK(PRESS_EVT));
                    bState = DOWN ;
                }
                break ;
            case DOWN:
                if (!isPressed()) {
                    bCounter = BOUNCEP ;
                    bState = BOUNCE ;
                }
                break ;
            case BOUNCE:
                if (isPressed()) {
                  bCounter = BOUNCEP ;
                    bState = DOWN ;
                } else {
                    if (!bCounter) {
                        bState = UP ;
                    } 
                }
                break ;
        }
    }
}

/*------------------------------------------------------------
 *     Volume Button task - poll volume button and send signal when pressed
 *------------------------------------------------------------*/

osThreadId_t t_buttonVolume;      /* task id of task to read button for volume*/

void buttonTaskVolume (void *arg) {
    int bState = UP ;
    int bCounter = 0 ;    
    while (1) {
        osDelay(10) ;
        if (bCounter) bCounter-- ;
        switch (bState) {
            case UP:
                if (isPressedVolume()) {
                    osEventFlagsSet(evtFlagsVolume, MASK(PRESS_EVT));
                    bState = DOWN ;
                }
                break ;
            case DOWN:
                if (!isPressedVolume()) {
                    bCounter = BOUNCEP ;
                    bState = BOUNCE ;
                }
                break ;
            case BOUNCE:
                if (isPressedVolume()) {
                  bCounter = BOUNCEP ;
                    bState = DOWN ;
                } else {
                    if (!bCounter) {
                        bState = UP ;
                    } 
                }
                break ;
        }
    }
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
    configureGPIOinput() ;       // Initialise button
    configureGPIOoutput() ;      // Initialise output    
    configurePIT(0) ;            // Configure PIT channel 0
    configureTPM0forPWM() ;                         
    SystemCoreClockUpdate() ;
    // Initialize CMSIS-RTOS
    osKernelInitialize();    
    // Create event flags
    evtFlagsTone = osEventFlagsNew(NULL);
    evtFlagsVolume = osEventFlagsNew(NULL);

    // Create threads
    t_tone = osThreadNew(toneTask, NULL, NULL);
    t_volume = osThreadNew(volumeTask, NULL, NULL);    
    t_buttonTone = osThreadNew(buttonTaskTone, NULL, NULL);    
    t_buttonVolume = osThreadNew(buttonTaskVolume, NULL, NULL);
    
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
