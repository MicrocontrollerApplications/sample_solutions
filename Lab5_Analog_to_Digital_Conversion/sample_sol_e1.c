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
uint16_t ADRES_to_mV(uint16_t register_val){
    // calculate measured voltage from register_val
    return (uint16_t) 3250*(register_val/1023.0);
}

void main(void) {
    __init();

    while (1) {
        /*
         * ADC Conversion - Step 3: Start conversion
         */
        ADCON0bits.GO = 1;

        /*
         * ADC Conversion - Step 4: Wait for conversion to complete by polling the GO/DONE bit
         */
        while(ADCON0bits.NOT_DONE){
            Nop();
        }

        /*
         * ADC Conversion - Step 5: Read ADC result (and write it to display)
         */
        GLCD_Value2Out_00(1, 1, ADRES_to_mV(ADRES), 4);
        

        // waste some time to not update the display to often
        for (unsigned int ii = 0; ii < 10000; ++ii)
            Nop();

    }

    return;
}

void __init(void) {
    OSCCONbits.IRCF = 5; // Fosc = ?
    GLCD_Init();
    GLCD_Text2Out(1, 1, "    mV");
    
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
    ADCON0bits.CHS = 1;
    ADCON2bits.ADFM = 1; // right justified format
    ADCON2bits.ACQT = 0b010; // 4T_AD = 8µs
    ADCON0bits.ADON = 1;
}
