/*
 * File:   main.c
 * Author: Sebastian Moosbauer
 *
 * Created on March 24, 2025, 4:57 PM
 */


#include <xc.h>
#include "LCD/GLCD_library.h"
//
#include "GLCD_config.h"

// function prototype. Used to publish the functions name. It can then
// be implement elsewhere. In case of this function it is implemented 
// below main(), to increase readability.
void __init(void);
void update_time_on_display(unsigned int time);
void update_bar_on_display(void);

// main code, i.e. the application itself
void main(void) {
    // Execute initialization of Microcontroller (will be explained soon!)
    __init();

    /* up to 6553.5 seconds = 1.82 hours can be stored
     * using 1ms steps instead of 100ms steps would decrease the storable time
     * to approx. 1 minute ;)
     */
    unsigned int time_100ms = 0; 
    unsigned char led_counter = 0;

    // while(1) is an endless loop. It's a very common part of microcontroller
    // code, as it prevents the application from termination. Without it, the
    // application would terminate after one execution and the microcontroller
    // would restart. Not very useful...
    while (1) {

//        // only use for loop to waste time, if Timer is not enabled
//        // Timer0 can be enabled in line 83
//        if (T0CONbits.TMR0ON == 0) {
//            // delay for ?? ms (check OSCCON's value in __init first!)
//            for (unsigned int i = 0; i < 1042; ++i) {
//                Nop();
//            }
//            
//            time_100ms += 1;
//        }
        
        if(T0CONbits.TMR0ON == 1 && INTCONbits.TMR0IF) {
            INTCONbits.TMR0IF = 0;
            // time_100ms += 1;
            // toggle two LEDs with approx. 1Hz frequency.
            LATBbits.LATB2 ^= 1;
            LATBbits.LATB5 ^= 1;
        }

        update_time_on_display(time_100ms);

        if (PORTBbits.RB1 == 0) {
            update_bar_on_display();
            while (PORTBbits.RB1 == 0);
        }
        
        // Poll for CCP1 interrupt flag
        if(PIR1bits.CCP1IF){
             PIR1bits.CCP1IF = 0;
            time_100ms += 1;
        }
    }

    return;
}


void __init(void) {
    // Set Oscillator Frequency to ??kHz (you will use this setting a lot!)
    OSCCON = 0x20;
    // Enable LCD
    GLCD_Init();
    // set initial string for Display
    GLCD_Text2Out(0, 2, "00:00.0");   
    
    ANSELB = 0;
    LATB = 0;
    TRISB = 0b00000010;

    T0CONbits.T08BIT = 1;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS = 0b111; // PS = 256 => Overflow after approx. 500ms
    INTCONbits.TMR0IF = 0; // reset overflow flag to avoid false initial notification
    TMR0H = 0;
    TMR0L = 0;
    T0CONbits.TMR0ON = 1;
    
    /*
     * CONFIG TIMER 1
     */
    T1CONbits.TMR1CS = 0b00;
    TMR1H = 0;
    TMR1L = 0;
    T1CONbits.T1CKPS = 0b00; // PS = 1 (all other would work, too)
    T1CONbits.TMR1ON = 1;
    
    /*
     * CONFIG CCP1
     */
    CCP1CONbits.CCP1M = 0b1011;
    CCPTMRS0bits.C1TSEL = 0b00;
    CCPR1H = 0x30;
    CCPR1L = 0xD4;
    PIR1bits.CCP1IF = 0;
    
}

void update_time_on_display(unsigned int time) {
    static unsigned int previous_time = 0;
    
    if (time != previous_time) {
        previous_time = time;
        unsigned int minutes = (unsigned int) (time / 600);
        GLCD_Value2Out_00(0, 2, minutes, 2);
        
        unsigned int no_mins = (unsigned int) (time - minutes * 600);
        unsigned int seconds = (unsigned int) (no_mins / 10);
        GLCD_Value2Out_00(0, 5, seconds, 2);
        
        unsigned char ms = no_mins - seconds * 10;
        GLCD_Value2Out_00(0, 8, ms, 1);
    }
}

char num_chars_in_bar = 0;
void update_bar_on_display(void) {
    // update bar
    GLCD_Text2Out(2, num_chars_in_bar, "#");
    
    ++num_chars_in_bar;
    if (num_chars_in_bar >= 11) {
        num_chars_in_bar = 0;
        GLCD_Clear2Row(2);
    }
}
