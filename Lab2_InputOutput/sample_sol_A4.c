/*
 * File:   newmain.c
 * Author: Sebastian Moosbauer
 *
 * Created on March 19, 2025, 11:24 AM
 */


#include <xc.h>

/**
 * Initialization function for our program. You'll learn to extend this today :)
 */
void __init(void);

int main(void) {
    // initialize microcontroller
    __init();
    
    
    
    // variable to store button's (TL) state
    unsigned char button;
    // variable to store the number of times TM is pressed
    unsigned char button_pressed_counter = 1;
    // save initial state of LATB
    unsigned char init_LATB = 0b00111100;
    while(1){
        button = PORTBbits.RB1; // read TM's state
        if(button == 0){ // is TM pressed?
            while(button == 0){ // do nothing while button is pressed
                Nop();
            }
            button_pressed_counter += 1; // increase button pressed counter
            if (button_pressed_counter > 5){
                LATB = init_LATB; // turn off all LEDs
                button_pressed_counter = 1; // reset counter
            }
            // calculate mask to toggle relavent bit
            unsigned char mask = 0x01 << button_pressed_counter;
            // toggle 
            LATB = init_LATB ^ mask;
        }
    }
    
//    while(1){
//        button = PORTBbits.RB2; // Read button TL
//
//        if (button == 0) { // TL pressed?
//            LATBbits.LATB3 = 0; // LED2 on
//        } else{
//            LATBbits.LATB3 = 1;  // LED2 off
//        }
//
//        if (PORTBbits.RB4 == 0) { // TR pressed?
//            while (PORTBbits.RB4 == 0) { // wait until TR is not pressed anymore
//                Nop();
//            }
//
//            LATBbits.LATB5 = !LATBbits.LATB5; //toggle LED4
//        }
//        Nop(); // Do nothing, a breakpoint may be placed here.
//    }
    
    return 0;
}

void __init(void){
    OSCCON = 0x50; // 500kHz internal clock (we'll learn that soon!))
    ANSELB = 0; // set all pins in register B to be digital
    TRISB = 0b00010100; //Input: RB2 and RB4; Output: all others
    // LATB = 0b00010100; // Turn LEDs off
    LATB = 0b00111100; // Turn all LEDs off
}
