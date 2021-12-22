#include <msp430.h>
#include <stdint.h>
#include "uart.h"

// Função que configura UART
void uart_Config()
{
    UCA1CTL1 |= UCSWRST;                                // Ativa reset

    UCA1CTL0 |= UCMODE_0;                               // Modo UART

    UCA1CTL1 |= UCSSEL__SMCLK;                          // Clock = SMCLK

    UCA1BRW = 18;                                       // 1048576/57600 = 18,204444
    UCA1MCTL |= UCBRF_2;                                // 0,204444 = 1,635555 ~ 2

    P4SEL |= (BIT4|BIT5);                               // Habilita saída do UART

    UCA1CTL1 &= ~UCSWRST;                               // Desativa reset
}

// Função que imprime os números decimais
void uart_Print_dec16(uint16_t num_dec)
{
    uint16_t aux_dec = 0;
    uint8_t aux = 0;

    if(num_dec > 999)
    {
        aux = 1;
        aux_dec = num_dec/1000;
        uart_PrintByte(0x30+aux_dec);       // milhar
    }

    num_dec -= 1000*aux_dec;
    if((num_dec > 99) || (aux == 1))
    {
        aux = 1;
        aux_dec = num_dec/100;
        uart_PrintByte(0x30+aux_dec);       // centena
    }

    num_dec -= 100*aux_dec;
    if((num_dec > 9) || (aux == 1))
    {
        aux = 1;
        aux_dec = num_dec/10;
        uart_PrintByte(0x30+aux_dec);       // dezena
    }

    num_dec -= 10*aux_dec;
    uart_PrintByte(0x30+num_dec);           // unidade
}

// Função que imprime uma string inteira
void uart_Print(uint8_t * str)
{
    while(*str)                             // Espera imprimir tds os valores da string
    {
        while(!(UCA1IFG & UCTXIFG));        // Espera TXIFG = 1
        UCA1TXBUF = *str++;                 // Trasmite string no buffer de transmissão
    }
}

// Função que imprime um byte
void uart_PrintByte(uint8_t byte)
{
    while(!(UCA1IFG & UCTXIFG));            // Espera TXIFG = 1
    UCA1TXBUF = byte;                       // Trasmite apenas um byte
}
