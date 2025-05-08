/*
 * File:   main.c
 * Author: Sebastian Moosbauer
 *
 * Created on May 7, 2025, 11:45 AM
 */


#include <xc.h>

#include <Libraries/PLIB/PLIB_USART.h>
#include <Libraries/LCD/GLCDnokia.h>

void __init(void);
void __interrupt(high_priority) __isr(void);

void writeValToUSART(unsigned int val);
void sendStatus(char status);
void write_to_LCD(char *data);

void append_sign_to_LCD(char data);
void adc_to_LCD(unsigned int data);

const static int MAX_VOLTAGE_AD = 3250;
static char state_text_mode = 0;
static char text_mode_column = 0;

void main(void) {
    __init();

    while (1) {

    }

    return;
}

void __init(void) {
    OSCCON = 0x70; // 16 MHz internal clock
    OSCTUNEbits.PLLEN = 1; // PLL enabled (Fosc = 4*Fosc)

    /***************************************************/
    // Pin configuration
    /***************************************************/
    LATB = 255; // all LEDs off
    TRISAbits.TRISA0 = 1; // Poti as input
    ANSELA = 0b00000001; // Port A0 to analog, all others digital
    ANSELB = ANSELC = 0x00; // all pins digital IO 
    TRISB = 0b00000010; // button input

    /***************************************************/
    // UART configuration
    /***************************************************/
    // define the pins C6 and C7 as input pins
    TRISCbits.TRISC6 = 1; // TX as input pin
    TRISCbits.TRISC7 = 1; // RX as input pin


    // TODO: configure using plib_usart Open1USART(....)
    // const unsigned int spbrg_value = 832;
    const unsigned int spbrg_value = 138;
    // Open1USART(0b01011000, 51);
    Open1USART(0b01011000, spbrg_value);


    // this has to be set manually because PLIB_USART does not support this (yet?)
    TXSTA1bits.SENDB = 0; // do not send sync break

    // TODO: configure using plib_usart baud1USART(....)       
    baud1USART(0x08);

    /***************************************************/
    // Configure AD module (measurement time 0.2 ms @ 4 MHz Fosz)
    /***************************************************/
    ADCON0bits.CHS = 0b00000; // select poti PORTA0
    ADCON1bits.PVCFG = 0b00; // upper limit 5V (Vdd)
    ADCON1bits.NVCFG = 0b00; // lower limit 0V (Vss)
    ADCON2bits.ACQT = 0b101; // 12 Tad = 96 us
    ADCON2bits.ADCS = 0b010; // Tad = 1/(Fosc/32) = 8 us
    ADCON2bits.ADFM = 0b1; // right just => use full 10 bit
    ADCON0bits.ADON = 1; // ADC on
    PIR1bits.ADIF = 0; // reset interrupt flag
    PIE1bits.ADIE = 1; // enable interrupts for adc

    GLCD_Init(); // Initialize display
    GLCD_Text2Out(1, 1, "Hello =)");

    // enable interrupt for button
    INTCON3bits.INT1IE = 1;

    // enable interrupts
    INTCONbits.PEIE = 1; // enable interrupt peripheral interrupts
    INTCONbits.GIE = 1; // enable interrupt in general   
}

void __interrupt(high_priority) high_isr(void) {
    // button was pressed
    if (INTCON3bits.INT1IE && INTCON3bits.INT1IF) {
        INTCON3bits.INT1IF = 0;
        // Send the sign '*'... TXREG1 = 42; would have the same effect        
        Write1USART('*');
        return;
    }

    // adc complete interrupt         
    if (PIE1bits.ADIE && PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        // calculate value
        unsigned int val = ((unsigned long)(ADRES) * MAX_VOLTAGE_AD) / 1023;
        // send to LCD
        adc_to_LCD(val);
        // send over USART - needs conversion to single chars
        writeValToUSART(val);
        return;
    }

    // a sign over serial port has arrived 
    if (PIE1bits.RC1IE && PIR1bits.RCIF) {
        // copy the received character into variable command
        char data = Read1USART();
        // check what to do - depending on our current state        
        if (!state_text_mode) {
            // handle the received command 
            switch (data) {
                case 't':
                    sendStatus('A');
                    // switch on LED
                    LATBbits.LATB5 = 0;
                    // set state text mode
                    state_text_mode = 1;
                    // reset column and clear lcd
                    text_mode_column = 0;
                    GLCD_Clear();
                    break;
                case 'a':
                    sendStatus('A');
                    // start AD conversion
                    ADCON0bits.GO = 1;
                    break;
                    // ignore line feeds
                case '\n': case '\r':
                    break;
                default:
                    // a different character has been received: send X as error response
                    sendStatus('X');
            }
        }            // we are currently in text mode
        else {
            if (data != 0) {
                // if not 0: append text on lcd
                append_sign_to_LCD(data);
            }
            else {
                // exit text mode
                state_text_mode = 0;
                GLCD_Clear();
                LATBbits.LATB5 = 1;
            }
        }
    }
}

void sendStatus(char status) {
    // Wait until transmission register is free again    
    while (Busy1USART());
    // Send the sign as status response
    Write1USART(status);
    // Wait until transmission register is free again
    while (Busy1USART());
    // Send newline
    Write1USART('\r');
}

void write_to_LCD(char *data) {
    GLCD_TextOut(0, 0, data);
}

void append_sign_to_LCD(char data) {
    GLCD_Char2Out(1, text_mode_column, data);
    text_mode_column += 6;
}

void adc_to_LCD(unsigned int val) {
    GLCD_Clear();
    GLCD_Value2Out_00(1, 0, (short) val, 4); // display result
    GLCD_Text2Out(1, 4, " mV"); // in mV    
}

void writeValToUSART(unsigned int val) {
    char c = 0;
    do {
        c = val % 10;
        val /= 10;
        c += '0';
        while (Busy1USART());
        Write1USART(c);
    } while (val != 0);
    while (Busy1USART());
    Write1USART('\r');

}