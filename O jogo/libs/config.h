#include <stdint.h>

#ifndef LIBS_CONFIG_H_
#define LIBS_CONFIG_H_

#define ABERTA  1
#define FECHADA 0

void config_init();
uint8_t get_s1();
uint8_t get_s2();

#endif /* LIBS_CONFIG_H_ */
