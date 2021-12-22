// Nome: Gabriel Santos Teixeira
// Matrícula: 190087587

#include <msp430.h> 
#include <stdint.h>

#define ABERTA  1
#define FECHADA 0
#define TRUE        1
#define FALSE       0
#define RW          BIT1
#define EN          BIT2
#define BL          BIT3

void lcd_SendNibble(uint8_t nibble, uint8_t isChar);
void lcd_SendByte(uint8_t byte, uint8_t isChar);
void lcd_PrintByte(uint8_t * str);
void Config_lcd();
void lcdWhichAddr(void);
void lcd_Print(uint8_t byte);
void lcd_dec16(uint16_t num_dec);
void Print_dec_to_hex(uint16_t dec);
void Print_hex(uint16_t hex);
void lcd_cmd(uint8_t cmd);
void lcd_cursor(uint8_t pos);
void lcd_volts_bar(uint16_t volts);
void Print_conv_lcd(uint16_t volts, uint8_t val_cnl);
void Move_cursor(uint16_t move, uint8_t channel);
void config_ADC();
void config_TA0();
uint8_t get_sw();
void config_init();
void modo_1();
void modo_2();
void modo_3();
void modo_4();
void i2cConfig_UCB0();
uint8_t i2cSend_UCB0(uint8_t addr, uint8_t * data, uint8_t nBytes);
uint8_t i2cSendByte_UCB0(uint8_t addr, uint8_t byte);

uint32_t eixo_x = 0, eixo_y = 0, media_x = 0, media_y = 0;
uint8_t flag = 0, addr = 0;

int main(void)
  {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    uint8_t button = 1;

    config_init();
    config_TA0();
    config_ADC();
    i2cConfig_UCB0();
    Config_lcd();

    while(1)
    {
        if(get_sw()) button++;
        if(button == 5) button = 1;
        if(flag)
        {
            P1OUT ^= BIT0;
            switch (button) {
                case 1:
                    modo_1();
                    break;
                case 2:
                    modo_2();
                    break;
                case 3:
                    modo_3();
                    break;
                default:
                    modo_4();
                    break;
            }
            flag = 0;
        }
    }
}

void modo_4()
{
    Print_conv_lcd(media_x, 0);
    Print_conv_lcd(media_y, 1);
}

void modo_3()
{
    static uint8_t aux_1 = 16, idx;
        if(idx == 0)
        {
            lcd_cursor(0x40);
            while(aux_1--)
            {
                lcd_Print(0xFE);
            }
            idx = 1;
        }
        else idx = 1;

        lcd_Print(0xFE);
        lcd_cursor(0x00);

    static uint8_t aux = 16, i;
    if(i == 0)
    {
        lcd_cursor(0x40);
        while(aux--)
        {
            lcd_Print(0xFE);
        }
        i = 1;
    }

    lcd_Print(0xFE);
    lcd_cursor(0x00);
    lcd_PrintByte("A1:");

    lcd_cursor(0x04);
    lcd_dec16(eixo_y);
    lcd_PrintByte("V");

    lcd_cursor(0x0C);
    Print_dec_to_hex(media_y);

    lcd_volts_bar(media_y);
}

void modo_2()
{
    static uint8_t aux = 16, i;
    if(i == 0)
    {
        lcd_cursor(0x40);
        while(aux--)
        {
            lcd_Print(0xFE);
        }
        i = 1;
    }

    lcd_Print(0xFE);
    lcd_cursor(0x00);
    lcd_PrintByte("A0:");

    lcd_cursor(0x04);
    lcd_dec16(eixo_x);
    lcd_PrintByte("V");

    lcd_cursor(0x0C);
    Print_dec_to_hex(media_x);

    lcd_volts_bar(media_x);
}

void modo_1()
{
    static uint8_t aux = 32, i;
    if(i == 0)
    {
        lcd_cursor(0x00);
        while(aux--)
        {
            if(aux == 16) lcd_cursor(0x40);
            lcd_Print(0xFE);
        }
        i = 1;
    }

    lcd_cursor(0x43);
    lcd_Print(0xFE);
    lcd_Print(0xFE);
    lcd_cursor(0x00);
    lcd_PrintByte("A0:");

    lcd_cursor(0x04);
    lcd_dec16(eixo_x);
    lcd_PrintByte("V");

    lcd_cursor(0x0C);
    Print_dec_to_hex(media_x);

    lcd_cursor(0x40);
    lcd_PrintByte("A1:");

    lcd_cursor(0x44);
    lcd_dec16(eixo_y);
    lcd_PrintByte("V");

    lcd_cursor(0x4C);
    Print_dec_to_hex(media_y);
}

void config_init()
{
    P6DIR &= ~BIT2;
    P6REN |=  BIT2;
    P6OUT |=  BIT2;

    P1OUT &= ~BIT0;
    P1DIR |=  BIT0;

    TA1CTL |= TASSEL__SMCLK | MC__UP | TACLR;
    TA1CCR0 = 5242;                             // 5ms
}

uint8_t get_sw()
{
    static uint8_t ps = ABERTA;
    if(!(P6IN & BIT2))
    {
        if(ps == ABERTA)
        {
            TA1CTL |= TACLR;
            TA1CCTL0 &= ~TAIFG;
            while(!(TA1CCTL0 & TAIFG));
            TA1CCTL0 &= ~TAIFG;
            ps = FECHADA;
            return 1;
        }
    }
    else
    {
        if(ps == FECHADA)
        {
            TA1CTL |= TACLR;
            TA1CCTL0 &= ~TAIFG;
            while(!(TA1CCTL0 & TAIFG));
            TA1CCTL0 &= ~TAIFG;
            ps = ABERTA;
            return 0;
        }
    }
    return 0;
}

void config_ADC()
{
    ADC12CTL0 = 0;
    ADC12CTL0 |= ADC12ON;

    ADC12CTL1 |= ADC12CSTARTADD_0
              | ADC12SHS_1
              | ADC12SSEL_3
              | ADC12CONSEQ_3;

    ADC12CTL2 |= ADC12RES_2;

    ADC12MCTL0  = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL1  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL2  = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL3  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL4  = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL5  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL6  = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL7  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL8  = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL9  = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL10 = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL11 = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL12 = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL13 = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL14 = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL15 = ADC12SREF_0 | ADC12INCH_1 | ADC12EOS;

    ADC12CTL0 |= ADC12ENC;

    P6SEL |= (BIT0 | BIT1);

    ADC12IE |= ADC12IE15;

    __enable_interrupt();
}

void config_TA0()
{
    TA0CTL |= TASSEL__ACLK | MC__UP | TACLR;
    TA0CCR0 = 655;
    TA0CCTL1 |= OUTMOD_6;
    TA0CCR1 = TA0CCR0 >> 1;

    P1DIR |= BIT2;
    P1SEL |= BIT2;
}

#pragma vector = ADC12_VECTOR;
__interrupt void ISR_ADC12()
{
    eixo_x =  ADC12MEM0 + ADC12MEM2  + ADC12MEM4  + ADC12MEM6;
    eixo_x += ADC12MEM8 + ADC12MEM10 + ADC12MEM12 + ADC12MEM14;
    media_x = eixo_x >> 3;
    eixo_x = (media_x * 3300) >> 12;

    eixo_y =  ADC12MEM1 + ADC12MEM3  + ADC12MEM5  + ADC12MEM7;
    eixo_y += ADC12MEM9 + ADC12MEM11 + ADC12MEM13 + ADC12MEM15;
    media_y = eixo_y >> 3;
    eixo_y = (media_y * 3300) >> 12;

    flag = 1;
}

void lcd_SendNibble(uint8_t nibble, uint8_t isChar)
{
    nibble = nibble << 4;

    i2cSendByte_UCB0(addr, nibble | BL | 0  | 0 | isChar);
    i2cSendByte_UCB0(addr, nibble | BL | EN | 0 | isChar);
    i2cSendByte_UCB0(addr, nibble | BL | 0  | 0 | isChar);
}

void lcd_SendByte(uint8_t byte, uint8_t isChar)
{
    lcd_SendNibble(byte >> 4  , isChar);
    lcd_SendNibble(byte & 0x0F, isChar);
}

void lcd_cmd(uint8_t cmd)
{
    lcd_SendByte(cmd, 0);
}

void lcd_cursor(uint8_t pos)
{
    lcd_cmd(0x80 | (pos&0x7F));
}

void lcd_Print(uint8_t byte)
{
    lcd_SendNibble(byte >> 4  , 1);
    lcd_SendNibble(byte & 0x0F, 1);
}

void lcd_PrintByte(uint8_t * str)
{
    while(*str)
    {
        lcd_SendByte(*str++, TRUE);
    }
}

void Config_lcd()
{
    lcdWhichAddr();

    lcd_SendNibble(0x03, FALSE);
    lcd_SendNibble(0x03, FALSE);
    lcd_SendNibble(0x03, FALSE);

    lcd_SendNibble(0x02, FALSE);

    lcd_SendByte(0x06, FALSE);
    lcd_SendByte(0x0C, FALSE);
    lcd_SendByte(0x14, FALSE);
    lcd_SendByte(0x28, FALSE);
    lcd_SendByte(0x01, FALSE);
}

void lcdWhichAddr(void)
{
    if(i2cSendByte_UCB0(0x27, 8))      addr = 0x27;
    else if(i2cSendByte_UCB0(0x3F, 8)) addr = 0x37;
}

void lcd_dec16(uint16_t num_dec)
{
    uint16_t aux_dec = 0;

    aux_dec = num_dec/1000;
    lcd_Print(0x30+aux_dec);    // unidade

    lcd_Print(0x2C);            // vírgula

    num_dec -= 1000*aux_dec;
    aux_dec = num_dec/100;
    lcd_Print(0x30+aux_dec);    // décimo

    num_dec -= 100*aux_dec;
    aux_dec = num_dec/10;
    lcd_Print(0x30+aux_dec);    // centésimo


    num_dec -= 10*aux_dec;
    lcd_Print(0x30+num_dec);    // milésimo
}

void Print_dec_to_hex(uint16_t dec)
{
    uint16_t aux_hex = 0;

    aux_hex = dec >> 12;
    Print_hex(aux_hex);

    dec -= 4096*aux_hex;
    aux_hex = dec >> 8;
    Print_hex(aux_hex);

    dec -= 256*aux_hex;
    aux_hex = dec >> 4;
    Print_hex(aux_hex);

    dec -= 16*aux_hex;
    Print_hex(dec);
}

void Print_hex(uint16_t hex)
{
    switch (hex)
    {
            case 10:
                lcd_Print(0x41);
                break;
            case 11:
                lcd_Print(0x42);
                break;
            case 12:
                lcd_Print(0x43);
                break;
            case 13:
                lcd_Print(0x44);
                break;
            case 14:
                lcd_Print(0x45);
                break;
            case 15:
                lcd_Print(0x46);
                break;
            default:
                lcd_Print(0x30+hex);
                break;
    }
}

void lcd_volts_bar(uint16_t volts)
{
    static uint8_t aux_volts;

    if(volts > 4096) volts = 4096;
    volts = (volts >> 8) + 1;
    if(volts == 17) volts -= 1;

    if(volts > aux_volts)
    {
        aux_volts = volts;
        lcd_cursor(0x40);

        while(volts--)
        {
            lcd_Print(0xFF);
        }
    }
    else if(volts < aux_volts)
    {
        aux_volts = volts;
        volts = 16 - volts;
        lcd_cursor(0x4F);
        lcd_SendByte(0x04, FALSE);

        while(volts--)
        {
            lcd_Print(0xFE);
        }
        lcd_SendByte(0x06, FALSE);
    }
    else
    {
        aux_volts = volts;
        lcd_cursor(0x40);

        while(volts--)
        {
            lcd_Print(0xFF);
        }
    }
}

void Print_conv_lcd(uint16_t volts, uint8_t val_cnl)
{
    static uint8_t ax = 32, i;
        if(i == 0)
        {
            lcd_cursor(0x00);
            while(ax--)
            {
                if(ax == 16) lcd_cursor(0x40);
                lcd_Print(0xFE);
            }
            i = 1;
        }

    static uint8_t aux_volts1, aux_volts2, aux_index, aux;
    static uint16_t aux_value;

        if(volts > 4096) volts = 4096;
        volts = (volts >> 8);

        switch (aux_index)
        {
            case 1:
                Move_cursor(volts, val_cnl);
                lcd_Print(0x3D);
                aux_volts2 = volts;
                aux_index = 2;
                break;
            case 2:
                if((volts > aux_volts1) && (volts > aux_volts2))
                {
                    Move_cursor(volts, val_cnl);
                    aux_value = 0x3E;
                    lcd_Print(aux_value);
                }
                else if((volts < aux_volts1) && (volts < aux_volts2))
                {
                    Move_cursor(volts, val_cnl);
                    aux_value = 0x3C;
                    lcd_Print(aux_value);
                }
                else
                {
                    Move_cursor(volts, val_cnl);
                    aux_value = 0x23;
                    lcd_Print(aux_value);
                }
                aux_volts2 = volts;
                aux_volts1 = volts;
                aux_index = 0;
                break;
            default:
                if((volts == aux_volts1) && (volts == aux_volts2))
                {
                    aux = 16;
                    Move_cursor(0x00, val_cnl);
                    while(aux--)
                    {
                    if(volts == aux_volts2) lcd_Print(0xFE);
                    }

                    Move_cursor(volts, val_cnl);
                    lcd_Print(aux_value);
                }
                else
                {
                    aux = 16;
                    Move_cursor(0x00, val_cnl);
                    while(aux--)
                    {
                        lcd_Print(0xFE);
                    }

                    Move_cursor(aux, val_cnl);
                    lcd_Print(0xFE);
                    Move_cursor(volts, val_cnl);
                    lcd_Print(0x2D);
                    aux_volts1 = volts;
                    aux_index = 1;
                }
                break;
        }
}

void Move_cursor(uint16_t move, uint8_t channel)
{
    if(channel) channel = 0x40;
    else if(!channel) channel = 0x00;

    switch (move)
    {
            case 10:
                lcd_cursor(0x0A+channel);
                break;
            case 11:
                lcd_cursor(0x0B+channel);
                break;
            case 12:
                lcd_cursor(0x0C+channel);
                break;
            case 13:
                lcd_cursor(0x0D+channel);
                break;
            case 14:
                lcd_cursor(0x0E + channel);
                break;
            case 15:
                lcd_cursor(0x0F+channel);
                break;
            default:
                lcd_cursor(move + channel);
                break;
    }
}

// Configurando o Mestre
void i2cConfig_UCB0()
{
    UCB0CTL1  = UCSWRST;
    UCB0CTL0 |= UCMST | UCMODE_3 | UCSYNC;
    UCB0BRW   = 100;                           // frequência = 10kHZ
    UCB0CTL1 |= UCSSEL__SMCLK;

    P3SEL |=  (BIT0|BIT1);
    P3DIR &= ~(BIT0|BIT1);
    P3REN |=  (BIT0|BIT1);
    P3OUT |=  (BIT0|BIT1);

    UCB0CTL1 &= ~UCSWRST;
}

// Utilizando o Mestre
uint8_t i2cSend_UCB0(uint8_t addr, uint8_t * data, uint8_t nBytes)
{
    UCB0IFG = 0;                              // Boa prática

    UCB0I2CSA = addr;
    UCB0CTL1 |= UCTXSTT | UCTR;

    while(!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = *data++;
    nBytes--;

    while(UCB0CTL1 & UCTXSTT);

    if(UCB0IFG & UCNACKIFG)
    {
        UCB0CTL1 |= UCTXSTP;
        while(UCB0CTL1 & UCTXSTP);
        return 0;
    }

    while(nBytes--)
    {
        while(!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = *data++;
    }

    while(!(UCB0IFG & UCTXIFG));

    UCB0CTL1 |= UCTXSTP;
    while(UCB0CTL1 & UCTXSTP);
    return 1;
}

uint8_t i2cSendByte_UCB0(uint8_t addr, uint8_t byte)
{
    return i2cSend_UCB0(addr, &byte, 1);
}


