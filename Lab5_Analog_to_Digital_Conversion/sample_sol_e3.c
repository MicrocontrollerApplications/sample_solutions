/*
 * File:   main.c
 * Author: Sebastian Moosbauer
 *
 * Created on April 25, 2025, 11:58 AM
 */


#include <xc.h>

#include <LCD/GLCD_library.h>

void __init(void);
void __interrupt(high_priority) __isr(void);

uint16_t ADRES_to_mV(uint16_t register_val) {
    return (uint16_t) 3250 * (register_val / 1023.0);
}

static uint16_t local_ADRES = 0;

void main(void) {
    __init();

    while (1) {
        if (local_ADRES) {
            // update display
            uint16_t voltage = ADRES_to_mV(local_ADRES);
            GLCD_Value2Out_00(1, 1, voltage, 4);
            if (voltage == 3250) {
                LATB = 0;
            } else if (voltage >= 2438) {
                LATB = 0b00000100;
            } else if (voltage >= 1625) {
                LATB = 0b00001100;
            } else if (voltage >= 813) {
                LATB = 0b00011100;
            } else {
                LATB = 0b00111100;
            }
        }
    }

    return;
}

void __init(void) {
    OSCCONbits.IRCF = 5; // Fosc = ?
    GLCD_Init();
    GLCD_Text2Out(1, 1, "    mV");
    /*
     * PORTB configuration for LEDs
     */
    ANSELB = 0;
    TRISB &= 0b11000011;
    LATB |= 0b00111100;

    /*
     * ADC Conversion - Step 1: Configure Port
     */
    TRISAbits.TRISA0 = 1;
    ANSELAbits.ANSA0 = 1;

    /*
     * ADC Conversion - Step 2: Configure the ADC module
     */
    ADCON2bits.ADCS = 0b001; // Fosc/8 = 2µs
    ADCON1bits.PVCFG = 0b00; // Vdd
    ADCON1bits.NVCFG = 0b00; // Vss
    ADCON0bits.CHS = 0;
    ADCON2bits.ADFM = 1; // right justified format
    ADCON2bits.ACQT = 0b010; // 4T_AD = 8µs
    ADCON0bits.ADON = 1;
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;

    /*
     * Configure Timer 1 to run at least 100ms
     */
    T1CONbits.TMR1CS = 0b00;
    T1CONbits.T1CKPS = 0b01;
    TMR1H = 0;
    TMR1L = 0;
    PIR1bits.TMR1IF = 0;
    T1CONbits.TMR1ON = 1;

    /*
     * Configure CCP5 module to trigger an interrupt every 100ms using Timer1
     */
    CCP5CONbits.CCP5M = 0b1011;
    CCPTMRS1bits.C5TSEL = 0b00;
    CCPR5 = 50000;
    PIR4bits.CCP5IF = 0;
    PIE4bits.CCP5IE = 1;

    /*
     * Enable Interrupts
     */
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;

}

void __interrupt(high_priority) __isr(void) {
    if (PIE4bits.CCP5IE && PIR4bits.CCP5IF) {
        PIR4bits.CCP5IF = 0;
        return;
    }

    if (PIE1bits.ADIE && PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        local_ADRES = ADRES;
        return;
    }

    // catch unconsidered interrupts
    while (1) {
        Nop();
    }
}
