/*
 * File:   main.c
 * Author: Sebastian Moosbauer
 *
 * Created on May 12, 2025, 17:21 AM
 */


#include <xc.h>

uint16_t test;

void __init(void);

static unsigned char led_mask = 0b00111100;
void main(void) {
    __init();

    while (1) {
       // time wasting loop for ?? ms
       for(int i = 0; i < 10000; ++i){
           Nop();
       }
       
       // toggle LEDs
       LATB ^= led_mask;
       // what's happening here? (We'll find out at the end of this lab ;) )
       unsigned char* bla = (unsigned char*)0xF8A;
       *bla = 0xFF;
    }

    return;
}

void __init(void) {
    OSCCON = 0x50; // Fosc = 4MHz
    
    ANSELB = 0x00;
    TRISB &= 0b11000011;
    LATB   = 0b00010100;
}

