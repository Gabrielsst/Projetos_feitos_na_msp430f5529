#include <msp430.h>
#include <stdint.h>
#include "config.h"

void config_init()
{
    // Configurando Switch 1
    P2DIR &= ~BIT1;                                     // Entrada
    P2REN |=  BIT1;                                     // Resistor habilitado
    P2OUT |=  BIT1;                                     // Resistor de pull-up

    // Configurando Switch 2
    P1DIR &= ~BIT1;                                     // Entrada
    P1REN |=  BIT1;                                     // Resistor habilitado
    P1OUT |=  BIT1;                                     // Resistor de pull-up

    // debounce botões (Timer A1)
    TA1CTL |= TASSEL__SMCLK | MC__STOP | TACLR;
    TA1CCR0 = 1048;                                     // 1ms

    // Contador do jogo (Timer A2)
    TA2CTL |= TASSEL__ACLK | MC__STOP | TAIE | TACLR ;

    // Timer auxiliar (Timer B0)
    TB0CTL |= TBSSEL__ACLK | MC__STOP | TBCLR ;
}

// Função que verifica Switch 1
uint8_t get_s1()
{
    static uint8_t ps1 = ABERTA;
    if(!(P2IN & BIT1))
    {
        if(ps1 == ABERTA)                               // verifica estado passado
        {
            // Faz debounce
            TA1CTL |= MC__UP | TACLR;
            while(!(TA1CCTL0 & CCIFG));
            TA1CTL &= ~MC__UP;
            TA1CCTL0 &= ~CCIFG;                         // Muda estado passado do botão 1
            ps1 = FECHADA;
            return 1;                                    // Retorna 1, se aberta
        }
    }
    else
    {
        if(ps1 == FECHADA)                              // Verifica estado passado
        {
            // Faz debounce
            TA1CTL |= MC__UP | TACLR;
            while(!(TA1CCTL0 & CCIFG));
            TA1CTL &= ~MC__UP;
            TA1CCTL0 &= ~CCIFG;
            ps1 = ABERTA;                               // Muda estado passado do botão 1
            return 0;                                    // Retorna 0, se fechada
        }
    }
    return 0;
}

// Função que verifica Switch 2
uint8_t get_s2()
{
    static uint8_t ps2 = ABERTA;
    if(!(P1IN & BIT1))
    {
        if(ps2 == ABERTA)                               // verifica estado passado
        {
            // Faz debounce
            TA1CTL |= MC__UP | TACLR;
            while(!(TA1CCTL0 & CCIFG));
            TA1CTL &= ~MC__UP;
            TA1CCTL0 &= ~CCIFG;
            ps2 = FECHADA;                              // Muda estado passado do botão 2
            return 1;                                   // Retorna 1, se aberta
        }
    }
    else
    {
        if(ps2 == FECHADA)                              // verifica estado passado
        {
            // Faz debounde
            TA1CTL |= MC__UP | TACLR;
            while(!(TA1CCTL0 & CCIFG));
            TA1CTL &= ~MC__UP;
            TA1CCTL0 &= ~CCIFG;
            ps2 = ABERTA;                               // Muda estado passado do botão 2
            return 0;                                   // Retorna 0, se fechada
        }
    }
    return 0;
}
