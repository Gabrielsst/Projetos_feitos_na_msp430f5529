#include <msp430.h> 
#include <stdint.h>
#include "libs/uart.h"
#include "libs/adc.h"
#include "libs/config.h"

static uint16_t soma = 0;
static uint8_t flag = 0, start = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configurando uart
    uart_Config();
    // Configurações de pino e timers
    config_init();
    // Configurações do ADC12
    config_ADC();
    // Configuração do timer do ADC12
    config_TA0();

    // Habilita interrupt global
    __enable_interrupt();

    // Espera 2s antes de rodar o código, para clock do sistema estabilizar
    TB0CTL |= MC__CONTINUOUS | TBCLR;       // Inicia o timer
    while(!(TB0CTL & TBIFG));
    TB0CTL &= ~MC__CONTINUOUS;              // "Para" o timer
    TB0CTL &= ~TBIFG;

    uint8_t press_button = 0, aux = 0, line[24];
    uint16_t random = 0, points;
    // limpa vetor line
    while(aux < 25)
    {
        line[aux] = 0x20;
        aux++;
    }
    aux = 0;
    // Entra na rotina principal
    while(1)
    {
        // Quando inicializa o jogo
        if(start == 0)
        {
            while(flag == 0);               // Espera a conversão retorna um valor
            flag = 0;
            uart_Print("\033[2J\r");        // Limpa a tela
            line[random] = 0x20;            // Limpa o vetor do jogo anterior
            random = soma;                  // random recebe valor aleatório do ADC12
            points = 0;                     // zera os pontos
            start = 1;                      // condição para só entrar novamente ao reiniciar
            uart_Print("FastPress: Press any button\n\r");
            uart_Print("|     S1     |     S2     |\n\r");
            // espera botão ser pressionado
            while((P2IN & BIT1) && (P1IN & BIT1));
            // ativar timer do jogo
            TA2CTL |= MC__CONTINUOUS | TACLR;
            // faz debounce
            TA1CTL |= MC__UP | TACLR;       // Inicia o timer
            while(!(TA1CCTL0 & CCIFG));
            TA1CTL &= ~MC__UP;              // "Para" o timer
            TA1CCTL0 &= ~CCIFG;
            // espera botão ser solto
            while(!(P2IN & BIT1) || !(P1IN & BIT1));
            // faz debounce
            TA1CTL |= MC__UP | TACLR;
            while(!(TA1CCTL0 & CCIFG));
            TA1CTL &= ~MC__UP;
            TA1CCTL0 &= ~CCIFG;
            // coloca asterisco no vetor
            line[random] = 0x2A;
            // imprime próxima linha
            uart_PrintByte(0x7C);                       // Imprime "|" inicial
            while(aux < 24)
            {
                if(aux == 12) uart_PrintByte(0x7C);     // Imprime "|" que separa S1 de S2
                uart_PrintByte(line[aux]);              // Imprime o vetor line
                aux++;
            }
            aux = 0;
            uart_PrintByte(0x7C);                       // Imprime "|" final
            while(aux < 3)                              // Ajuste para impressão de pontuação
            {
                uart_PrintByte(0x20);
                aux++;
            }
            aux = 0;
            uart_Print_dec16(points);                   // Imprime pontuação
            uart_Print("\n\r");                         // Inicia nova linha no terminal
        }

        if((get_s1()) && (start == 1)) press_button = 1;    // Verifica Switch 1
        if((get_s2()) && (start == 1)) press_button = 2;    // Verifica Switch 2

        // Quando pressiona o switch 1
        if(press_button == 1)
        {
            press_button = 0;                           // Para não imprimir "infinitamente"
            while(flag == 0);                           // Espera a conversão retorna um valor
            flag = 0;
            aux = 0;
            // limpa vetor
            line[random] = 0x20;
            // se acertar, ganha ponto
            if(random < 12)
            {
                points++;
            }
            // Novo valor aleatório
            random = soma;
            // Coloca asterísco
            line[random] = 0x2A;
            // imprime próxima linha
            uart_PrintByte(0x7C);                       // Imprime "|" inicial
            while(aux < 24)
            {
                if(aux == 12) uart_PrintByte(0x7C);     // Imprime "|" que separa S1 de S2
                uart_PrintByte(line[aux]);              // Imprime vetor line
                aux++;
            }
            aux = 0;
            uart_PrintByte(0x7C);                       // Imprime "|" final
            while(aux < 3)                              // Ajuste para impressão de pontuação
            {
                uart_PrintByte(0x20);
                aux++;
            }
            aux = 0;
            uart_Print_dec16(points);                   // Imprime pontuação
            uart_Print("\n\r");                         // Inicia nova linha no terminal
        }

        // Quando pressiona o switch 2
        if(press_button == 2)
        {
            press_button = 0;                           // Para não imprimir "infinitamente"
            while(flag == 0);                           // Espera a conversão retorna um valor
            flag = 0;
            aux = 0;
            // limpa vetor
            line[random] = 0x20;
            // se acertar, ganha ponto
            if(random > 11)
            {
                points++;
            }
            // Novo valor aleatório
            random = soma;
            // Coloca asterísco
            line[random] = 0x2A;
            // imprime próxima linha
            uart_PrintByte(0x7C);                       // Imprime "|" inicial
            while(aux < 24)
            {
                if(aux == 12) uart_PrintByte(0x7C);     // Imprime "|" que separa S1 de S2
                uart_PrintByte(line[aux]);              // Imprime vetor line
                aux++;
            }
            aux = 0;
            uart_PrintByte(0x7C);                       // Imprime "|" final
            while(aux < 3)                              // Ajuste para impressão de pontuação
            {
                uart_PrintByte(0x20);
                aux++;
            }
            aux = 0;
            uart_Print_dec16(points);                   // Imprime pontuação
            uart_Print("\n\r");                         // Inicia nova linha no terminal
            }

        // Quando termina o jogo
        if(start == 2)
        {
            uart_Print("Time's up!");
            TB0CTL |= MC__CONTINUOUS | TACLR;           // Inicia o timer
            while(!(TB0CTL & TAIFG));
            TB0CTL &= ~MC__CONTINUOUS;                  // "Para" o timer
            TB0CTL &= ~TBIFG;
            start = 0;
        }
    }
}

// Interrupção do ADC12
#pragma vector = ADC12_VECTOR;
__interrupt void ISR_ADC12()
{
    soma  = (ADC12MEM0&0x03)+(ADC12MEM1&0x03)+(ADC12MEM2&0x03);     // Soma dos valores
    soma += (ADC12MEM3&0x03)+(ADC12MEM4&0x03)+(ADC12MEM5&0x03);     // filtrados
    soma += (ADC12MEM6&0x03)+(ADC12MEM7&0x03);

    if(soma > 23) soma = 23;                            // É raro, porém as vezes a conversão
                                                        // retorna 24, condição para evitar problemas
    flag = 1;                                           // Indica que uma nova conversão foi feita
}

// Interrupção do timer A2
#pragma vector = TIMER2_A1_VECTOR
__interrupt void TA2_CCRN_ISR()
{
    static uint8_t qty_in;                              // Conta quantas vezes ocorreu a interrupção

    TA2CTL &= ~TAIFG;
    qty_in++;

    if(qty_in == 5)                                     // 2s * 5 = 10s
    {
        TA2CTL &= ~MC__CONTINUOUS;                      // "Para" o timer do jogo
        start = 2;                                      // Leva o código para o final do jogo
        qty_in = 0;
    }
}
