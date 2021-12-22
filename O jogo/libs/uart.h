#include <stdint.h>

#ifndef LIBS_UART_H_
#define LIBS_UART_H_

void uart_Config();
void uart_Print(uint8_t * str);
void uart_PrintByte(uint8_t byte);
void uart_Print_dec16(uint16_t num_dec);

#endif /* LIBS_UART_H_ */
