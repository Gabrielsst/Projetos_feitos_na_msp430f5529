#include <msp430.h>
#include <stdint.h>
#include "adc.h"

void config_ADC()
{
    ADC12CTL0 = 0;                                          // Setando reset
    ADC12CTL0 |= ADC12ON;                                   // Ativando ADC12

    ADC12CTL1 |= ADC12CSTARTADD_0
              | ADC12SHS_1                                  // Trigger TA0.1
              | ADC12SSEL_3                                 // Fonte de clock é SMCLK
              | ADC12CONSEQ_3;
    ADC12CTL2 |= ADC12RES_2;

    ADC12MCTL0  = ADC12SREF_0 | ADC12INCH_1;                // Todos setados para receber
    ADC12MCTL1  = ADC12SREF_0 | ADC12INCH_1;                // do canal 1 (P6.1)
    ADC12MCTL2  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL3  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL4  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL5  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL6  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL7  = ADC12SREF_0 | ADC12INCH_1 | ADC12EOS;

    ADC12CTL0 |= ADC12ENC;                                  // Conversão habilitada

    P6SEL |= BIT1;                                          // Usando o pino de ADC12

    ADC12IE |= ADC12IE7;                                    // Interrupt do ADC12 quando chega no
}                                                           // final da sequência

void config_TA0()
{
    TA0CTL |= TASSEL__SMCLK | MC__UP | TACLR;
    TA0CCR0 = 72;                                           // ~70us
    TA0CCTL1 |= OUTMOD_6;                                   // Toggle/Set
    TA0CCR1 = TA0CCR0 >> 1;

    P1DIR |= BIT2;                                          // Saída
    P1SEL |= BIT2;                                          // Habilitado como saída PWM
}                                                           // Timer A0
