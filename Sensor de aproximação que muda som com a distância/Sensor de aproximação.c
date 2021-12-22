// Nome: Gabriel Santos Teixeira
// Matr�cula: 190087587

#include <msp430.h> 
#include <stdint.h>

void gpio_config(void);                 // Fun��o para configurar gpio
void TAx_config(void);                  // Fun��o para configurar os timers
uint32_t calc_dist(void);               // Fun��o que calcula dist�ncia do eco
void leds(uint32_t arg_dist);           // Fun��o que modifica a sa�da dos leds
uint32_t calc_freq(uint32_t dist_freq); // Fun��o calcula frequ�ncia, de acordo
void ta2_prog(uint32_t freq_sensor);    // com a dist�ncia, que ta2_prog vai
                                        // disparar o som.
volatile uint32_t eco = 0;              // Vari�vel global para levar valor da
                                        // interrup��o para a main.
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	volatile uint32_t dist = 0; // Vari�vel que recebe a dist�ncia
	volatile uint32_t freq = 0; // Vari�vel que recebe a frequ�ncia

	gpio_config();              // Fazendo as configura��es da gpio
	TAx_config();               // e dos timers.

	__enable_interrupt();       // Habilitando a interrup��o global.

	while(1){
	    dist = calc_dist();     // Retorna a dist�ncia
	    leds(dist);             // Modifica os leds
	    freq = calc_freq(dist); // Retorna a frequ�ncia
	    ta2_prog(freq);         // Modifica a frequ�ncia que vai ser emitida
	}
}

void gpio_config(void){
    // Configurando sa�da do canal 4 do timer TA0.4
    P1DIR |= BIT5;              // P1.5 como sa�da
    P1SEL |= BIT5;              // Selecionado como sa�da do timer TA0.4

    // Configurando P2.0 para captura do timer TA1.1
    P2DIR &= ~BIT0;             // P2.0 como entrada
    P2REN |=  BIT0;             // Resistor habilitado
    P2OUT |=  BIT0;             // Resistor em pull-up
    P2SEL |=  BIT0;             // Selecionado como entrada do timer TA1.1

    // Configurando sa�da do canal 2 do timer TA2.2
    P2DIR |= BIT5;              // P2.5 como sa�da
    P2SEL |= BIT5;              // Selecionado como sa�da do timer TA2.2

    // Configurar led vermelho
    P1OUT &= ~BIT0;
    P1DIR |=  BIT0;

    // Configurar led verde
    P4OUT &= ~BIT7;
    P4DIR |=  BIT7;
}

void TAx_config(void){
    // Configurar TA0.4
    TA0CTL |= TASSEL__SMCLK | MC__UP | TACLR;
    TA0CCR0 = 20971;            // Topo de contagem em 1/50Hz
    TA0CCR4 = 20;               // Pulso do trigger de 20us

    // Configurar TA2.2
    TA2CTL |= TASSEL__SMCLK | MC__UP | TACLR;

    // Configurar TA1.1 para modo de captura
    TA1CTL |= TASSEL__SMCLK | MC__UP | TACLR;
    TA1CCR0 = 26213;
    TA1CCTL1 = CCIS_0 | SCS | CAP | CM_3 | CCIE;  // Habilitado modo de captura do canal 1;
                                                  // Habilitado interrup��o local do canal 1.
    // Configurar modo da PWM para TA0.4
    TA0CCTL4 = OUTMOD_7;        // Modo reset/set

    // configurar modo da PWM para TA2.2
    TA2CCTL2 = OUTMOD_7;        // Modo reset/set
}

uint32_t calc_dist(void){
    static uint32_t dist_fint;  // Vari�vel da fun��o para fazer os c�lculos internos
    dist_fint = (17000L*eco)/1048576; // d = (100*V_som/2)*eco/freq_smclk

    return dist_fint;           // Retorna a dist�ncia do pulso capturado
}

void leds(uint32_t led_dist){
    if(led_dist < 10){          // Se dist�ncia menor que 10, os dois leds acendem
        P1OUT |=  BIT0;
        P4OUT |=  BIT7;
    }
    else if((10 == led_dist) || (led_dist < 30)){ // Se dist�ncia entre 10 e 30,
        P1OUT |=  BIT0;                           // led vermelho acende e
        P4OUT &= ~BIT7;                           // led verde apaga.
    }
    else if((30 == led_dist) || (led_dist < 51)){ // Se dist�ncia entre 30 e 50,
        P1OUT &= ~BIT0;                           // led vermelho apaga e
        P4OUT |=  BIT7;                           // led verde acende.
    }
    else if(led_dist > 50){     // Se dist�ncia maior que 50, os dois leds apagam
        P1OUT &= ~BIT0;
        P4OUT &= ~BIT7;
    }
}

uint32_t calc_freq(uint32_t freq_dist){
    static uint32_t freq_int;   // Vari�vel da fun��o para fazer os c�lculos internos
    if((freq_dist < 50) || (freq_dist != 50)){  // Calcula freq para dist�ncias < 50
        freq_int = 1048576L/((50 - freq_dist)*100);
    }                           // F�rmula: freq = freq_smclk/ ((50 - dist)*100)
    else{
        freq_int = 0;           // Se dist�ncia igual ou maior que 50, freq retorna 0
    }
    return freq_int;            // Retorna frequ�ncia como passos de contagem
}

void ta2_prog(uint32_t freq_sensor){
    if(freq_sensor != 0){       // Se freq n�o for zero
        TA2CCR0 = freq_sensor - 1;  // Per�odo da PWM, do som que ser� emitido
        TA2CCR2 = TA2CCR0 >> 1;     // Duty_cycle da PWM
    }
    else{                       // Se freq for zero, som n�o ser� emitido
        TA2CCR0 = 1048576L/1000;
        TA2CCR2 = 0;
    }
}

#pragma vector = TIMER1_A1_VECTOR;  // Interrup��o quando eco � capturado
__interrupt void TA1_CCRN_ISR(){
    static uint32_t t1;             // Vari�veis do interrupt para fazer os
    static uint32_t t2;             // c�lculos internos.

    if(TA1CCTL1 & CCI) t1 = TA1CCR1; // Verifica borda de subida
    else if(!(TA1CCTL1 & CCI)){      // Verifica borda de descida
        t2 = TA1CCR1;
        if(t2 < t1) eco = (t2 - t1) + 65536; // Valores capturados em per�odos diferentes
        else eco = t2 - t1;                  // Valores capturados no mesmo per�odo
    }
    TA1CCTL1 &= ~CCIFG;             // Abaixar a flag do canal 1 do TA1, pois a interrup��o
}                                   // � agrupada.
