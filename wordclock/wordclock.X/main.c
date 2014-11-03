/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__PIC24E__)
    	#include <p24Exxxx.h>
    #elif defined (__PIC24F__)||defined (__PIC24FK__)
	#include <p24Fxxxx.h>
    #elif defined(__PIC24H__)
	#include <p24Hxxxx.h>
    #endif
#endif

#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp   */

#include <libpic30.h>


/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/* i.e. uint16_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

void shiftH() {
    LATBbits.LATB1 = 1;
    __delay_ms(1);
    LATAbits.LATA2 = 1;
    __delay_ms(1);
    LATAbits.LATA2 = 0;
    LATBbits.LATB1 = 0;
    __delay_ms(1);
}

void shiftL() {
    LATBbits.LATB1 = 0;
    __delay_ms(1);
    LATAbits.LATA2 = 1;
    __delay_ms(1);
    LATAbits.LATA2 = 0;
    __delay_ms(1);
}

void shift(int value) {
    if (value) {
        shiftH();
    } else {
        shiftL();
    }
}

void enableOutput() {
    LATBbits.LATB2 = 1;
    __delay_ms(1);
    LATBbits.LATB2 = 0;
}

int hour = 0;
int minute = 0;
int seconds = 0;

int16_t main(void)
{

    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize IO ports and peripherals */
    InitApp();

    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB2 = 0;
    TRISAbits.TRISA2 = 0;

    LATBbits.LATB1=0;
    LATBbits.LATB2=0;
    LATAbits.LATA2=0;

   IPC0bits.T1IP = 5;	 //set interrupt priority

    PR1 = 32767;
    T1CONbits.TON = 1;
    T1CONbits.TCS = 1;
    T1CONbits.TCKPS = 0;

    IFS0bits.T1IF = 0;	 //reset interrupt flag
    IEC0bits.T1IE = 1;	 //turn on the timer1 interrupt
    while(1);
}

void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{
    seconds++;
    if (seconds > 255) {
        seconds = 0;
    }
    shift(seconds & 0b10000000);
    shift(seconds & 0b1000000);
    shift(seconds & 0b100000);
    shift(seconds & 0b10000);
    shift(seconds & 0b1000);
    shift(seconds & 0b100);
    shift(seconds & 0b10);
    shift(seconds & 0b1);
    enableOutput();
    IFS0bits.T1IF = 0;
}
