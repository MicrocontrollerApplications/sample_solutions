/*
 * File:   main.c
 * Author: Sebastian Moosbauer
 *
 * Created on April 2, 2025, 3:21 PM
 */


#include <xc.h>

#include <LCD/GLCD_library.h>

#include "clock.h"

void __init(void);
void __interrupt(high_priority) __isr(void);

static clock_t clock;
static unsigned char time_step_in_ms;

/*
 * Exercise 3.a
 * Define a global variable as a flag for clock updates
 */
static int time_to_add = 0;

void main(void) {
    __init();

    while (1) {
        Nop();
        /*
         * Exercise 3.b
         * Implement clock update
         */
        if(time_to_add){
            add_ms_to_watch(clock, time_to_add);
            time_to_add = 0;
            
        }
    }

    return;
}

void __init(void) {
    OSCCONbits.IRCF = 0b010; // Fosc = 500 kHz
    GLCD_Init();
    GLCD_Text2Out(0, 2, "00:00.0");
    clock_init(&clock);

    /*
     * Exercise 1.b
     * Pin configuration. 
     *  - Keep in mind the two possible modes.
     *  - Check the instructions for potentially relevant input pins.
     */
    ANSELB = 0; // all pins digital mode
    TRISB = 0x04; // all pins output, except RB2
    LATB = 0b00111000; //all LEDs off


    /* 
     * Timer0 shall overflow after approx. 130ms (no CCP required)
     */
    T0CONbits.T08BIT = 1;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    /*
     * Exercise 1.c
     * Add missing configurations for Timer0 and enable the related interrupt
     */
    T0CONbits.T0PS = 0b101;
    TMR0IF = 0;
    TMR0IE = 1;
    T0CONbits.TMR0ON = 1;


    time_step_in_ms = 100;
    T1CONbits.T1CKPS = 2; // PS = 4
    T1CONbits.T1SYNC = 1;
    TMR1 = 0;
    /*
     * Exercise 2.a
     * Add missing configurations for Timer1
     */
    T1CONbits.TMR1CS = 0; // Fosc/4
    T1CONbits.TMR1ON = 1;

    /*
     * Exercise 2.b
     * Configure CCP Module and the related interrupt
     */
    CCP1CONbits.CCP1M = 0b1011; // special event trigger mode
    CCPTMRS0bits.C1TSEL = 0; // use Timer1
    CCPR1 = 3125; // timer value to trigger interrupt (100ms))
    PIR1bits.CCP1IF = 0;
    PIE1bits.CCP1IE = 1;

    /*
     * Experts exercise
     * Configure an external interrupt for RB2
     */
    INTCON2bits.INTEDG2 = 0; // interrupt on falling edge (i.e. immediately on push)
    INTCON3bits.INT2IF = 0;
    INTCON3bits.INT2IE = 1;
    

    /*
     * Exercise 1.d
     * Enable global and peripheral interrupts
     */
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
     
}

/*
 * Exercise 1.a
 * Implement interrupt service routine. Check provided code blocks 
 * within instruction!
 */
void __interrupt(high_priority) __isr(void){
    if(TMR0IE && TMR0IF){
        TMR0IF = 0;
        LATBbits.LATB4 ^= 1;
        return;
    }
    
    if(PIE1bits.CCP1IE && PIR1bits.CCP1IF){
        PIR1bits.CCP1IF = 0;
        // add_ms_to_watch(clock, time_step_in_ms);
        time_to_add += time_step_in_ms;
        return;
    }
    
    /*
     * Experts exercise
     */
    if(INTCON3bits.INT2IE && INTCON3bits.INT2IF){
        INTCON3bits.INT2IF = 0;
        if(PIE1bits.CCP1IE == 0){
            GLCD_Text2Out(0, 2, "00:00.0");
            clock_init(&clock);
        }
        
        time_to_add = 0;
        PIE1bits.CCP1IE ^= 1;
        
        return;
    }
    
    while(1){
        Nop();
    }
}
