/*
 * File:   main.c
 * Author: Sebastian Moosbauer
 *
 * Created on May 20, 2025, 11:59 AM
 */


#include <xc.h>

#include <LCD/GLCDnokia.h>

void __init(void);

void main(void) {
    __init();
    
    GLCD_Text2Out(1, 1, "Bit Banging rocks! =)");

    while (1) {
        for (int i = 0; i < 20000; ++i){
            Nop();
        }
        LATB ^= 0b00111100;
    }

    return;
}

void __init(void) {
    OSCCON = 0x50; // Fosc = 4MHz
    
    ANSELB = 0;
    TRISB &= 0b11000011;
    LATB |= 0b00010100;
    
    GLCD_Init();
}



void GLCD_Bit_Banging(unsigned char data)
{
    GLCD_nCS = 0;
    GLCD_CLK = 0; GLCD_DATA = data >> 7 & 0x01; GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_CLK = 0; GLCD_DATA = data >> 6 & 0x01; GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_CLK = 0; GLCD_DATA = data >> 5 & 0x01; GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_CLK = 0; GLCD_DATA = data >> 4 & 0x01; GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_CLK = 0; GLCD_DATA = data >> 3 & 0x01; GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_CLK = 0; GLCD_DATA = data >> 2 & 0x01; GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_CLK = 0; GLCD_DATA = data >> 1 & 0x01; GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_CLK = 0; GLCD_DATA = data & 0x01;      GLCD_DLY(); GLCD_CLK = 1; GLCD_DLY();
    GLCD_nCS = 1;
}
