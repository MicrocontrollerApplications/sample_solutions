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

uint16_t previous_adc_val = 0;
/**
 * Check wether voltage increased or decreased. 
 * According to that return 1 for increase (clockwise turn), 
 * -1 for decrease (counter clockwise turn), 0 else.
 * @param register_val register value of ADRES
 * @return direction; 1 for clockwise, -1 for counter clockwise, 0 else
 */
int8_t get_direction_from_ADC(unsigned int register_val);

/**
 * rotate LEDs depending on detected direction. For clockwise turn toggle the
 * LED to the right of the current LED, otherwise toggle the left one. In case
 * of no change, do nothing.
 * @param direction detected direction of Potentiometer turn.
 *          1 for clockwise, -1 for counter clockwise, 0 else
 */
void rotate_leds(int8_t direction);

static uint8_t local_ADRES = 0;

void main(void) {
    __init();

    while (1) {
        if (local_ADRES) {
            // update display
            GLCD_Value2Out_00(1, 1, ADRES_to_mV(local_ADRES), 4);
            rotate_leds(get_direction_from_ADC(local_ADRES));
            previous_adc_val = local_ADRES;            
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
    ADCON2bits.ADFM = 0; // left justified format
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
        local_ADRES = ADRESH;
        return;
    }

    // catch unconsidered interrupts
    while (1) {
        Nop();
    }
}

int8_t get_direction_from_ADC(unsigned int register_val) {
    int8_t direction = 0;
    // no rotation
    if (previous_adc_val == register_val)
        return direction;

    // rotation clockwise
    if (previous_adc_val < register_val)
        direction = 1;
        // rotation counterclockwise
    else
        direction = -1;

    return direction;
}

void rotate_leds(int8_t direction) {
    // early return in case no rotation is needed
    if (direction == 0)
        return;

    uint8_t current_state = PORTB & 0b00111100;
    // all LEDs off
    if (0b00111100 == current_state) {
        if (direction > 0) {
            // LED1 on
            LATB = 0b00111000;
        } else {
            // LED4 on
            LATB = 0b00011100;
        }
        // return function as no further actions are needed
        return;
    }
    // invert LED bits
    current_state ^= 0b00111100;

    // rotate LED to the right
    if (direction == 1) {
        current_state <<= 1;
    } else {
        current_state >>= 1;
    }

    // invert LED bits
    current_state ^= 0b00111100;
    LATB = current_state;

}