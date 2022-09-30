/*----------------------------------------------------------------------------
    Code for Lab 4
    In this project
        - the red and green lights light flashes on/off periodically 
        - pressing the button turn the lights on or off
    There are three tasks/threads
       t_button: polls the button and signals t_greenLED
       t_LED: flashes the red and green led on and off, using a delay and listens for a button input
 *---------------------------------------------------------------------------*/
 
#include "cmsis_os2.h"
#include <MKL25Z4.h>
#include <stdbool.h>
#include "gpio.h"

osEventFlagsId_t evtFlags ;          // event flags    



/*--------------------------------------------------------------
 *   Thread t_LED
 *       Flash Red and Green LEDs using a delay
 *--------------------------------------------------------------*/
osThreadId_t t_LED;        /* id of thread to flash red and green les */

// This attribute structure can change the default 
//   attributes (e.g. priority) of the thread. See documentation.
//   Not usually needed: this is to illustrate.
const osThreadAttr_t LED_attr = {
  .name = "LED",          // Name is used by debugger
  .priority = osPriorityHigh, // Set initial thread priority to high
                              //   defaults is osPriorityNormal
  .stack_size = 256       // Create the thread stack with a size of 256
                          //   this overrides the default set in RTX Config
};

void LEDThread (void *arg) { // thread to flash the green and red leds
    int ledState = RED_LED_ON ;
    uint32_t result;
    redLEDOnOff(LED_ON);
    while (1) {
        result = osEventFlagsWait (evtFlags, MASK(PRESS_EVT), osFlagsWaitAny, 3000); // wait 3 seconds to see if the button is pushed
        if (result == osFlagsErrorTimeout) { // button not pushed as there is a timout
            switch (ledState) {
                case RED_LED_ON:
                    redLEDOnOff(LED_OFF);
                    greenLEDOnOff(LED_ON);                                    
                    ledState = GREEN_LED_ON;
                    break;
                case GREEN_LED_ON:
                    redLEDOnOff(LED_ON);
                    greenLEDOnOff(LED_OFF);
                    ledState = RED_LED_ON ;
                    break;
		        }
        } else { // the button has been pushed
            switch (ledState) {
                case RED_LED_ON:
                    redLEDOnOff(LED_OFF);
                    ledState = RED_LED_OFF;
                    break;
                case GREEN_LED_ON:
                    greenLEDOnOff(LED_OFF);
                    ledState = GREEN_LED_OFF ;
                    break;
                case RED_LED_OFF:
                    redLEDOnOff(LED_ON);
                    ledState = RED_LED_ON ;
                    break;
                case GREEN_LED_OFF:
                    greenLEDOnOff(LED_ON);
                    ledState = GREEN_LED_ON ;
                    break;
            }
        }
    }
}

/*------------------------------------------------------------
 *  Thread t_button
 *      Poll the button
 *      Signal if button pressed
 *------------------------------------------------------------*/
osThreadId_t t_button;        /* id of thread to poll button */

void buttonThread (void *arg) {
    int state ; // current state of the button
    int bCounter ;
    state = BUTTONUP ;
    
    while (1) {
        osDelay(10);  // 10 ticks delay - 10ms
        switch (state) {
            case BUTTONUP:
                if (isPressed()) {
                    state = BUTTONDOWN ;
                    osEventFlagsSet(evtFlags, MASK(PRESS_EVT));
                }
                break ;
            case BUTTONDOWN:
                if (!isPressed()) {
                    state = BUTTONBOUNCE ;
                    bCounter = BOUNCE_COUNT ;
                }
                break ;
            case BUTTONBOUNCE:
                if (bCounter > 0) bCounter -- ;
                if (isPressed()) {
                    state = BUTTONDOWN ; }
                else if (bCounter == 0) {
                    state = BUTTONUP ;
                }
                break ;
        }
    }
}


/*----------------------------------------------------------------------------
 * Application main
 *   Initialise I/O
 *   Initialise kernel
 *   Create threads
 *   Start kernel
 *---------------------------------------------------------------------------*/
 
int main (void) { 
    // System Initialization
    SystemCoreClockUpdate();

    // Initialise peripherals
    configureGPIOoutput();
    configureGPIOinput();
 
    // Initialize CMSIS-RTOS
    osKernelInitialize();
    
    // Create event flags
    evtFlags = osEventFlagsNew(NULL);

    // Create threads
    t_LED = osThreadNew(LEDThread, NULL, &LED_attr); 
    t_button = osThreadNew(buttonThread, NULL, NULL); 
    
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
